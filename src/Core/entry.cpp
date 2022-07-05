#include <pch.h>

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Nodes/NodeFactory.h"
#include "Project/Project.h"
#include "Environnement/Environnement.h"

#include <serial/serial.h>

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
    //initialize application
	ApplicationWindow::init();
	
	std::vector<serial::PortInfo> ports = serial::list_ports();
	std::string portName;
	for(auto& port : ports){
		if(port.port.find("Stacato") != std::string::npos){
			portName = port.port;
			break;
		}
	}
	
	
	auto getMessage = [](uint8_t newByte) -> std::vector<uint8_t> {
		static std::vector<uint8_t> currentMessage;
		static bool b_messageStarted = false;
		static int expectedLength = 0;
		static int currentLength;
		
		static uint8_t startByte = 0x33;
		static uint8_t stopByte = 0x99;
		
		if(!b_messageStarted){
			if(newByte == startByte) {
				b_messageStarted = true;
				expectedLength = 0;
				currentMessage.clear();
				//currentMessage.push_back(newByte);
			}
		}else{
			if(expectedLength == 0){
				if(newByte == 0) b_messageStarted = false;
				else{
					expectedLength = newByte;
					currentLength = 0;
					//currentMessage.push_back(newByte);
				}
			}
			else if(currentLength < expectedLength){
				currentLength++;
				currentMessage.push_back(newByte);
			}else if(currentLength == expectedLength){
				uint8_t checksum = 0x0;
				for(uint8_t byte : currentMessage) checksum ^= byte;
				if(checksum != newByte) b_messageStarted = false;
				currentLength++;
				//currentMessage.push_back(newByte);
			}else if(currentLength == expectedLength + 1){
				if(newByte == stopByte){
					b_messageStarted = false;
					
					//currentMessage.push_back(newByte);
					auto output = currentMessage;
					currentMessage.clear();
					return output;
				}else b_messageStarted = false;
			}else b_messageStarted = false;
		
		}
		return {};
	};
	
	auto printMessage = [](std::vector<uint8_t>& message){
		static char messageString[256];
		messageString[0] = 0;
		int length = 0;
		length += sprintf(messageString + length,"[");
		for(size_t i = 0; i < message.size(); i++){
			length += sprintf(messageString + length, "%X", message[i]);
			if(i < message.size() - 1) length += sprintf(messageString + length, ",");
		}
		length += sprintf(messageString + length,"]");
		Logger::warn("{}", messageString);
	};
	
	serial::Serial consoleSerial(portName);
	if(consoleSerial.isOpen()){
		consoleSerial.flush();
		while(true){
			if(consoleSerial.available()){
				uint8_t newByte;
				consoleSerial.read(&newByte, 1);
				auto message = getMessage(newByte);
				if(!message.empty()){
					printMessage(message);
				}
			}
		}
	}
	
	
	
	//initialize node factory modules
	NodeFactory::load();
	
	//load environnement and plots, configure ethercat network interfaces
	Project::loadStartup();
	
	//load network interfaces, initialize networking, open ethercat network interface
	Environnement::initialize();
	
	//application gui runtime, function returns when application is quit
	ApplicationWindow::open();

	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();

	//terminate application
	ApplicationWindow::terminate();
}
