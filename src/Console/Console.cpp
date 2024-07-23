#include <pch.h>
#include "Console.h"

#include "ConsoleHandler.h"
#include "ConsoleMapping.h"
#include "ConsoleIODevice.h"
#include "Serial.h"

//Operating System Abstraction Layer from libSOEM
//Uselful for precise cross-platform thread sleep
#include <osal.h>


std::shared_ptr<Console> Console::initialize(std::shared_ptr<SerialPort> port){
	auto console = std::make_shared<Console>();
	console->serialPort = port;
	console->connectionState = ConnectionState::NOT_CONNECTED;
	console->serialPort->setMessageReceiveCallback([console](uint8_t* message, size_t size){
		console->readMessage(message, size);
	});
	console->serialPort->setPortCloseCallback([console](){
		console->onDisconnection();
	});
	console->inputHandler = std::thread([console](){
		console->updateInputs();
	});
	return console;
}

void Console::terminate(std::shared_ptr<Console> console){
	if(console->inputHandler.joinable()) console->inputHandler.join();
	if(console->outputHandler.joinable()) console->outputHandler.join();
}







void Console::onDisconnection(){
	mapping->onDisconnection();
	Logger::warn("Console {} Disconnected", consoleName);
	b_inputHandlerRunning = false;
	b_outputHandlerRunning = false;
	connectionState = ConnectionState::DISCONNECTED;
}

void Console::onConnection(){
	Logger::info("Console {} Connected !", consoleName);
	std::string threadName = "Console " + getName() + " Input Handler";
	//pthread_setname_np(threadName.c_str());
	connectionState = ConnectionState::CONNECTED;
	ConsoleHandler::applyMapping(shared_from_this());
	outputHandler = std::thread([this](){ updateOutputs(); });
}

void Console::setMapping(std::shared_ptr<ConsoleMapping> mapping_){
	mapping_->apply(shared_from_this());
	mapping = mapping_;
}


void Console::receiveDeviceInput(uint8_t* message, size_t size){
	
	IODevice::Type deviceType = IODevice::getTypeFromCode(message[0]);
	uint8_t deviceIndex = message[1];
	
	if(deviceIndex >= ioDevices.size()) return;
	auto& device = ioDevices[deviceIndex];
	if(device->getType() != deviceType) return;
	
	if(size < 3) return;
	size_t deviceInputDataSize = size - 2;
	uint8_t* deviceInputData = &message[2];
	
	device->updateInput(deviceInputData, deviceInputDataSize);
	
	
	/*
	uint8_t deviceTypeCode = message[0];
	switch(IODevice::getTypeFromCode(deviceTypeCode)){
		case IODevice::Type::PUSHBUTTON:
			Logger::warn("Push Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case IODevice::Type::SWITCH:			break;
		case IODevice::Type::POTENTIOMETER:		break;
		case IODevice::Type::ENCODER:			break;
		case IODevice::Type::JOYSTICK_1AXIS:	break;
		case IODevice::Type::JOYSTICK_2AXIS:{
			float x = (int8_t)message[2] / 127.f;
			float y = (int8_t)message[3] / 127.f;
			Logger::warn("Joystick {} x:{:.2f} y:{:.2f}", message[1], x, y);
			}break;
		case IODevice::Type::JOYSTICK_3AXIS:	break;
		case IODevice::Type::LED:				break;
		case IODevice::Type::LED_PWM:			break;
		case IODevice::Type::LED_RGB:			break;
		case IODevice::Type::LED_BUTTON:
			Logger::warn("LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case IODevice::Type::LED_PWM_BUTTON:
			Logger::warn("PWM LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case IODevice::Type::LED_RGB_BUTTON:
			Logger::warn("RGB LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case IODevice::Type::UNKNOWN:
			Logger::warn("Unknown device input received...");
			break;
	}
	*/
}

void Console::receiveConnectionConfirmation(uint8_t* message, size_t size){
	if(connectionState != ConnectionState::CONNECTION_REQUESTED) return;
	if(size != 4) return;
    uint8_t msb = message[1];
    uint8_t lsb = message[2];
    uint16_t t = msb << 8 | lsb;
	if(t != timeoutDelay_milliseconds) return;
	if(message[3] != heartbeatInterval_milliseconds) return;
	connectionState = ConnectionState::CONNECTION_CONFIRMATION_RECEIVED;
	Logger::debug("Received Connection Confirmation");
}

void Console::receiveIdentificationReply(uint8_t* message, size_t size){
	if(connectionState != ConnectionState::IDENTIFICATION_REQUESTED) return;
	if(size < 2) return;
	if(message[size - 1] != 0) return;
	const char* name = (char*)&message[1];
	consoleName = name;
	connectionState = ConnectionState::IDENTIFICATION_RECEIVED;
	Logger::debug("Received Identification Reply, console name is {}", consoleName);
}

void Console::receiveDeviceEnumerationReply(uint8_t* message, size_t size){
	if(connectionState != ConnectionState::DEVICE_ENUMERATION_REQUESTED) return;
	if(size < 2) return;
	for(int i = 1; i < size; i++) if(IODevice::getTypeFromCode(message[i]) == IODevice::Type::UNKNOWN) return;
	int deviceCount = size - 1;
	Logger::debug("Received Device Enumeration Reply");
	Logger::debug("{} has {} IODevices:", consoleName, deviceCount);
	ioDevices.reserve(deviceCount);
	for(int i = 1; i < size; i++){
		IODevice::Type deviceType = IODevice::getTypeFromCode(message[i]);
		ioDevices.push_back(IODevice::make(deviceType));
		Logger::debug("[{}] {}", i, IODevice::getTypeString(deviceType));
	}
	connectionState = ConnectionState::DEVICE_ENUMERATION_RECEIVED;
}

void Console::receiveHeartbeat(uint8_t* message, size_t size){
	if(size != 1) return;
    double now_millis = Timing::getProgramTime_milliseconds();
    lastHeartbeatReceiveTime = now_millis;
}




void Console::readMessage(uint8_t* message, size_t messageLength){
	uint8_t header = message[0];
	switch(header){
		case CONNECTION_CONFIRMATION: 	receiveConnectionConfirmation(message, messageLength); break;
		case IDENTIFICATION_REPLY:		receiveIdentificationReply(message, messageLength); break;
		case DEVICE_ENUMERATION_REPLY:	receiveDeviceEnumerationReply(message, messageLength); break;
		case HEARTBEAT: 				receiveHeartbeat(message, messageLength); break;
		default:						receiveDeviceInput(message, messageLength);
	}
}






void Console::sendConnectionRequest(){
	uint8_t message[4];
	message[0] = CONNECTION_REQUEST;
    message[1] = (timeoutDelay_milliseconds >> 8) & 0xFF;
    message[2] = timeoutDelay_milliseconds & 0xFF;
	message[3] = heartbeatInterval_milliseconds;
	serialPort->send(message, 4);
}

void Console::sendIdentificationRequest(){
	uint8_t message[1];
	message[0] = IDENTIFICATION_REQUEST;
	serialPort->send(message, 1);
}

void Console::sendDeviceEnumerationRequest(){
	uint8_t message[1];
	message[0] = DEVICE_ENUMERATION_REQUEST;
	serialPort->send(message, 1);
}

void Console::sendConnectionConfirmation(){
	uint8_t message[1];
	message[0] = CONNECTION_CONFIRMATION;
	serialPort->send(message, 1);
}

void Console::sendHeartbeat(){
	uint8_t message[1];
	message[0] = HEARTBEAT;
	serialPort->send(message, 1);
}



void Console::connect(){
	switch(connectionState){
		case ConnectionState::NOT_CONNECTED:
			sendConnectionRequest();
            connectionRequestTime = Timing::getProgramTime_milliseconds();
			connectionState = ConnectionState::CONNECTION_REQUESTED;
			Logger::trace("Connection Requested");
			break;
		case ConnectionState::CONNECTION_CONFIRMATION_RECEIVED:
			sendIdentificationRequest();
			connectionState = ConnectionState::IDENTIFICATION_REQUESTED;
			Logger::trace("Identification Requested");
			break;
		case ConnectionState::IDENTIFICATION_RECEIVED:
			sendDeviceEnumerationRequest();
			connectionState = ConnectionState::DEVICE_ENUMERATION_REQUESTED;
			Logger::trace("Enumeration Requested");
			break;
		case ConnectionState::DEVICE_ENUMERATION_RECEIVED:
			sendConnectionConfirmation();
            lastHeartbeatReceiveTime = Timing::getProgramTime_milliseconds();
            lastHeartbeatSendTime = Timing::getProgramTime_milliseconds();
			onConnection();
			break;
		default: break;
	}
}


void Console::handleTimeout(){
    auto now = Timing::getProgramTime_milliseconds();
	if(isConnected()){
		if(now - lastHeartbeatSendTime > heartbeatInterval_milliseconds){
			lastHeartbeatSendTime = now;
			sendHeartbeat();
		}
		if(now - lastHeartbeatReceiveTime > timeoutDelay_milliseconds){
			Logger::info("Console Connection Timed out");
			onDisconnection();
		}
	}
	else if(isConnecting()){
		if(now - connectionRequestTime > connectionTimeoutDelay_milliseconds){
			Logger::info("Connection Request Timed out");
			onDisconnection();
		}
	}
}



void Console::updateInputs(){
	std::string threadName = "Console " + getName() + " Connection Handler";
	//pthread_setname_np(threadName.c_str());
	b_inputHandlerRunning = true;
	while(b_inputHandlerRunning){
		if(!isConnected()) connect();
		serialPort->read();
		handleTimeout();
        osal_usleep(5000);
	}
}

void Console::updateOutputs(){
	std::string threadName = "Console " + getName() + " Output Handler";
	//pthread_setname_np(threadName.c_str());
	b_outputHandlerRunning = true;
	while(b_outputHandlerRunning){
		for(int i = 0; i < ioDevices.size(); i++){
			uint8_t* deviceData;
			size_t deviceDataSize = 0;
			if(ioDevices[i]->updateOutput(&deviceData, &deviceDataSize)){
				outputMessage[0] = IODevice::getCodeFromType(ioDevices[i]->getType());
				outputMessage[1] = i;
				memcpy(&outputMessage[2], deviceData, deviceDataSize);
				size_t messageSize = deviceDataSize + 2;
				serialPort->send(outputMessage, messageSize);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}
