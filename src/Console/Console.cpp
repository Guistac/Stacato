#include <pch.h>
#include "Console.h"

#include <serial/serial.h>
#include "Serial.h"

namespace Console{

std::thread consoleHandler;
void updateConsole();
bool b_handlerRunning = false;
bool b_connected = false;
std::mutex mutex;

std::shared_ptr<serial::Serial> consoleSerialPort = nullptr;

void initialize(){
	consoleHandler = std::thread([](){
		b_handlerRunning = true;
		pthread_setname_np("Console Handler Thread");
		while(b_handlerRunning) updateConsole();
		b_handlerRunning = false;
	});
}

void terminate(){
	b_handlerRunning = false;
	consoleHandler.join();
}



void handleIncomingMessage(std::vector<uint8_t>& message){
	uint8_t header = message.front();
	switch(header){
		case 0x10: //PUSHBUTTON
			Logger::warn("Push Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case 0x1A: //LED_BUTTON
			Logger::warn("LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case 0x1B: //LED_PWM_BUTTON
			Logger::warn("PWM LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case 0x1C: //LED_RGB_BUTTON
			Logger::warn("RGB LED Button {} {}", message[1], message[2] == 1 ? "Pressed" : "Released");
			break;
		case 0x11: //SWITCH
			break;
		case 0x12: //POTENTIOMETER
			break;
		case 0x13: //ENCODER
			break;
		case 0x14: //JOYSTICK_1AXIS
			break;
		case 0x15:{ //JOYSTICK_2AXIS
			float x = (int8_t)message[2] / 127.f;
			float y = (int8_t)message[3] / 127.f;
			Logger::warn("Joystick {} x:{:.2f} y:{:.2f}", message[1], x, y);
			}break;
		case 0x16: //JOYSTICK_3AXIS
			break;
		case 0x17: //LED
			break;
		case 0x18: //LED_PWM
			break;
		case 0x19: //LED_RGB
			break;
		default: return "unknown message";
	}
	
	
	//printMessage(message);
}


void connect(){
	for(auto& port : serial::list_ports()){
		if(port.port.find("Stacato") != std::string::npos){
			consoleSerialPort = std::make_shared<serial::Serial>(port.port);
			if(consoleSerialPort->isOpen()){
				consoleSerialPort->flush();
				b_connected = true;
				Logger::info("Console Connected.");
				return;
			}else consoleSerialPort.reset();
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(250));
}

void communicate(){
	static std::vector<uint8_t> message;
	static uint8_t incomingBytes[512];
	while(size_t byteCount = consoleSerialPort->available()){
		consoleSerialPort->read(incomingBytes, byteCount);
		for(int i = 0; i < byteCount; i++){
			if(readMessage(incomingBytes[i], message)) handleIncomingMessage(message);
		}
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void updateConsole(){
	const std::lock_guard<std::mutex> lock(mutex);
	if(b_connected) {
		try{
			communicate();
		}catch(serial::IOException e){
			if(e.getErrorNumber() != 6) Logger::error("IOException: {} {}", e.getErrorNumber(), e.what());
			Logger::warn("Console Disconnected.");
			b_connected = false;
		}catch(serial::SerialException e){
			Logger::info("Serial Exception: {}", e.what());
			b_connected = false;
		}catch(serial::PortNotOpenedException e){
			Logger::critical("Port Not Opened Exception: {}", e.what());
			b_connected = false;
		}
	}
	else connect();
}



bool isConnected();
std::string& getModelName();



};
