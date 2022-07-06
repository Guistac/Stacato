#include <pch.h>
#include "Console.h"

#include <serial/serial.h>
#include "Serial.h"

namespace Console{

std::thread consoleHandler;
bool b_handlerRunning = false;
void update();

SerialPort consoleSerialPort("Stacato");

std::mutex mutex;

ConnectionState connectionState = ConnectionState::NOT_CONNECTED;
bool isConnected(){ return connectionState == ConnectionState::CONNECTED; }

std::string consoleName;
std::string& getName(){ return consoleName; }

int timeoutDelay_milliseconds = 200;
int heartbeatInterval_milliseconds = 50;
int connectionTimeoutDelay_milliseconds = 200;
std::chrono::time_point<std::chrono::system_clock> lastHeartbeatReceiveTime;
std::chrono::time_point<std::chrono::system_clock> lastHeartbeatSendTime;
std::chrono::time_point<std::chrono::system_clock> connectionRequestTime;

void readMessage(uint8_t*, size_t);
void onDisconnection();

void initialize(){
	if(b_handlerRunning) return;
	consoleSerialPort.setMessageReceiveCallback(readMessage);
	consoleSerialPort.setPortCloseCallback(onDisconnection);
	consoleHandler = std::thread([](){
		b_handlerRunning = true;
		pthread_setname_np("Console Handler Thread");
		while(b_handlerRunning) update();
		b_handlerRunning = false;
	});
}

void terminate(){
	if(!b_handlerRunning) return;
	b_handlerRunning = false;
	consoleHandler.join();
}

void onDisconnection(){
	if(!isConnected()) return;
	Logger::warn("Console {} Disconnected", consoleName);
	connectionState = ConnectionState::NOT_CONNECTED;
}

void onConnection(){
	Logger::info("Console {} Connected !", consoleName);
}



uint8_t getDeviceTypeCode(DeviceType deviceType){
	switch(deviceType){
		case DeviceType::PUSHBUTTON:        return 0x10;
		case DeviceType::SWITCH:            return 0x11;
		case DeviceType::POTENTIOMETER:     return 0x12;
		case DeviceType::ENCODER:           return 0x13;
		case DeviceType::JOYSTICK_1AXIS:    return 0x14;
		case DeviceType::JOYSTICK_2AXIS:    return 0x15;
		case DeviceType::JOYSTICK_3AXIS:    return 0x16;
		case DeviceType::LED:               return 0x17;
		case DeviceType::LED_PWM:           return 0x18;
		case DeviceType::LED_RGB:           return 0x19;
		case DeviceType::LED_BUTTON:        return 0x1A;
		case DeviceType::LED_PWM_BUTTON:    return 0x1B;
		case DeviceType::LED_RGB_BUTTON:    return 0x1C;
		default: 							return 0xFF;
	}
}

DeviceType getDeviceType(uint8_t deviceTypeCode){
	switch(deviceTypeCode){
		case 0x10: 	return DeviceType::PUSHBUTTON;
		case 0x11: 	return DeviceType::SWITCH;
		case 0x12: 	return DeviceType::POTENTIOMETER;
		case 0x13: 	return DeviceType::ENCODER;
		case 0x14: 	return DeviceType::JOYSTICK_1AXIS;
		case 0x15: 	return DeviceType::JOYSTICK_2AXIS;
		case 0x16: 	return DeviceType::JOYSTICK_3AXIS;
		case 0x17: 	return DeviceType::LED;
		case 0x18: 	return DeviceType::LED_PWM;
		case 0x19: 	return DeviceType::LED_RGB;
		case 0x1A: 	return DeviceType::LED_BUTTON;
		case 0x1B: 	return DeviceType::LED_PWM_BUTTON;
		case 0x1C: 	return DeviceType::LED_RGB_BUTTON;
		default: 	return DeviceType::UNKNOWN;
	}
}

const char* getDeviceTypeString(DeviceType deviceType){
	switch(deviceType){
		case DeviceType::PUSHBUTTON:        return "PushButton";
		case DeviceType::SWITCH:            return "Switch";
		case DeviceType::POTENTIOMETER:     return "Potentiometer";
		case DeviceType::ENCODER:           return "Encoder";
		case DeviceType::JOYSTICK_1AXIS:    return "Joystick 1 Axis";
		case DeviceType::JOYSTICK_2AXIS:    return "Joystick 2 Axis";
		case DeviceType::JOYSTICK_3AXIS:    return "Joystick 3 Axis";
		case DeviceType::LED:               return "LED";
		case DeviceType::LED_PWM:           return "LED (PWM)";
		case DeviceType::LED_RGB:           return "LED (RGB)";
		case DeviceType::LED_BUTTON:        return "Button with LED";
		case DeviceType::LED_PWM_BUTTON:    return "Button with LED (PWM)";
		case DeviceType::LED_RGB_BUTTON:    return "Button with LED (RGB)";
		default: 							return "Unknown Device";
	}
}


void receiveDeviceInput(uint8_t* message, size_t size){
	uint8_t deviceTypeCode = message[0];
	DeviceType deviceType = getDeviceType(deviceTypeCode);
	switch(deviceType){
		case DeviceType::PUSHBUTTON:
			Logger::warn("Push Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case DeviceType::SWITCH:			break;
		case DeviceType::POTENTIOMETER:		break;
		case DeviceType::ENCODER:			break;
		case DeviceType::JOYSTICK_1AXIS:	break;
		case DeviceType::JOYSTICK_2AXIS:{
			float x = (int8_t)message[2] / 127.f;
			float y = (int8_t)message[3] / 127.f;
			Logger::warn("Joystick {} x:{:.2f} y:{:.2f}", message[1], x, y);
			}break;
		case DeviceType::JOYSTICK_3AXIS:	break;
		case DeviceType::LED:				break;
		case DeviceType::LED_PWM:			break;
		case DeviceType::LED_RGB:			break;
		case DeviceType::LED_BUTTON:
			Logger::warn("LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case DeviceType::LED_PWM_BUTTON:
			Logger::warn("PWM LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case DeviceType::LED_RGB_BUTTON:
			Logger::warn("RGB LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case DeviceType::UNKNOWN:
			Logger::warn("Unknown device input received...");
			break;
	}
}

void receiveConnectionConfirmation(uint8_t* message, size_t size){
	if(connectionState != ConnectionState::CONNECTION_REQUESTED) return;
	if(size != 3) return;
	if(message[1] != timeoutDelay_milliseconds) return;
	if(message[2] != heartbeatInterval_milliseconds) return;
	connectionState = ConnectionState::CONNECTION_CONFIRMATION_RECEIVED;
	Logger::debug("Received Connection Confirmation");
}

void receiveIdentificationReply(uint8_t* message, size_t size){
	if(connectionState != ConnectionState::IDENTIFICATION_REQUESTED) return;
	if(size < 2) return;
	if(message[size - 1] != 0) return;
	const char* name = (char*)&message[1];
	consoleName = name;
	connectionState = ConnectionState::IDENTIFICATION_RECEIVED;
	Logger::debug("Received Identification Reply, console name is {}", consoleName);
}

void receiveDeviceEnumerationReply(uint8_t* message, size_t size){
	if(connectionState != ConnectionState::DEVICE_ENUMERATION_REQUESTED) return;
	if(size < 2) return;
	for(int i = 1; i < size; i++) if(getDeviceType(message[i]) == DeviceType::UNKNOWN) return;
	
	Logger::debug("Received Device Enumeration Reply");
	Logger::debug("{} has {} Devices:", consoleName, size - 1);
	for(int i = 1; i < size; i++){
		DeviceType deviceType = getDeviceType(message[i]);
		Logger::debug("[{}] {}", i, getDeviceTypeString(deviceType));
	}
	
	connectionState = ConnectionState::DEVICE_ENUMERATION_RECEIVED;
}

void receiveHeartbeat(uint8_t* message, size_t size){
	if(size != 1) return;
	lastHeartbeatReceiveTime = std::chrono::system_clock::now();
}




void readMessage(uint8_t* message, size_t messageLength){
	uint8_t header = message[0];
	switch(header){
		case CONNECTION_CONFIRMATION: 	receiveConnectionConfirmation(message, messageLength); break;
		case IDENTIFICATION_REPLY:		receiveIdentificationReply(message, messageLength); break;
		case DEVICE_ENUMERATION_REPLY:	receiveDeviceEnumerationReply(message, messageLength); break;
		case HEARTBEAT: 				receiveHeartbeat(message, messageLength); break;
		default:						receiveDeviceInput(message, messageLength);
	}
}






void sendConnectionRequest(){
	uint8_t message[3];
	message[0] = CONNECTION_REQUEST;
	message[1] = timeoutDelay_milliseconds;
	message[2] = heartbeatInterval_milliseconds;
	consoleSerialPort.send(message, 3);
}

void sendIdentificationRequest(){
	uint8_t message[1];
	message[0] = IDENTIFICATION_REQUEST;
	consoleSerialPort.send(message, 1);
}

void sendDeviceEnumerationRequest(){
	uint8_t message[1];
	message[0] = DEVICE_ENUMERATION_REQUEST;
	consoleSerialPort.send(message, 1);
}

void sendConnectionConfirmation(){
	uint8_t message[1];
	message[0] = CONNECTION_CONFIRMATION;
	consoleSerialPort.send(message, 1);
}

void sendHeartbeat(){
	uint8_t message[1];
	message[0] = HEARTBEAT;
	consoleSerialPort.send(message, 1);
}



void connect(){
	switch(connectionState){
		case ConnectionState::NOT_CONNECTED:
			sendConnectionRequest();
			connectionRequestTime = std::chrono::system_clock::now();
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
			connectionState = ConnectionState::CONNECTED;
			lastHeartbeatReceiveTime = std::chrono::system_clock::now();
			lastHeartbeatSendTime = std::chrono::system_clock::now();
			onConnection();
			break;
		default: break;
	}
}


void handleTimeout(){
	using namespace std::chrono;
	auto now = system_clock::now();
	if(!isConnected()){
		if(now - connectionRequestTime > milliseconds(connectionTimeoutDelay_milliseconds)){
			connectionState = ConnectionState::NOT_CONNECTED;
			Logger::info("Connection Request Timed out");
		}
	}else{
		if(now - lastHeartbeatSendTime > milliseconds(heartbeatInterval_milliseconds)){
			lastHeartbeatSendTime = now;
			sendHeartbeat();
		}
		if(now - lastHeartbeatReceiveTime > milliseconds(timeoutDelay_milliseconds)){
			Logger::info("Console Connection Timed out");
			onDisconnection();
		}
	}
}

void update(){
	using namespace std::chrono;
	if(consoleSerialPort.isOpen()){
		std::this_thread::sleep_for(milliseconds(10));
		const std::lock_guard<std::mutex> lock(mutex);
		if(!isConnected()) connect();
		handleTimeout();
	}else std::this_thread::sleep_for(milliseconds(250));
	consoleSerialPort.update();
}



};
