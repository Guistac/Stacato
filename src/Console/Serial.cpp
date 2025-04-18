#include <pch.h>
#include "Console.h"

#include "Serial.h"
#include <serial/serial.h>
#include "config.h"


std::shared_ptr<SerialPort> findSerialPort(std::string& portMatchingString){
	auto serialports = serial::list_ports();

	for(auto& port : serialports){
		#if defined(STACATO_UNIX)
		if(port.description.find(portMatchingString) != std::string::npos){
		#else if defined(STACATO_MACOS)
		if(port.port.find(portMatchingString) != std::string::npos){
		#endif
			auto openedPort = std::make_shared<serial::Serial>(port.port);
			if(openedPort->isOpen()) {
				Logger::info("Opened serial port {} ({})", port.port, port.description);
				return std::make_shared<SerialPort>(openedPort, port.port);
			}
			else openedPort = nullptr;
		}
	}
	return nullptr;
}



//[0]   Start Byte
//[1]   Length of the message content (L + 2)
//[2]   Message Buffer Start
//[2+L] Checksum (xor of all bytes after start end before stop, starting with message length)
//[3+L] Stop Byte

SerialPort::SerialPort(std::shared_ptr<serial::Serial> port, std::string& name) : serialPort(port), portName(name){
	b_portOpen = true;
}

void SerialPort::close(){
	serialPort->close();
	b_portOpen = false;
	Logger::warn("Serial Port {} Closed", portName);
}

constexpr uint8_t startByte = 0x33;
constexpr uint8_t stopByte = 0x99;
	
void SerialPort::read(){
	if(!b_portOpen) return;
	try{
		while(int byteCount = serialPort->available()){
			serialPort->read(incomingBytes, byteCount);
			for(int i = 0; i < byteCount; i++) readByte(incomingBytes[i]);
		}
	}
	catch(serial::IOException e){
		if(e.getErrorNumber() != 6) {
			Logger::critical("Error while reading from serial port {} :", portName);
			Logger::critical("Error #{} : {}",  e.getErrorNumber(), e.what());
		}
		close();
	}
	catch(serial::SerialException e){
		Logger::info("Error while reading from serial port {} :", portName);
		Logger::info("{}", e.what());
		b_portOpen = false;
		close();
	}
	catch(serial::PortNotOpenedException e){
		Logger::info("Error not opened Exception while reading from serial port {} :", portName);
		Logger::info("{}", e.what());
		close();
	}
}

void SerialPort::send(uint8_t* message, int messageLength){
	if(!b_portOpen) return;

	static int outframecount = 0;

	outgoingFrame[0] = startByte;
	outgoingFrame[1] = messageLength;
	memcpy(outgoingFrame + 2, message, messageLength);
	uint8_t checksum = messageLength;
	for(int i = 0; i < messageLength; i++) checksum ^= message[i];
	outgoingFrame[2 + messageLength] = checksum;
	outgoingFrame[3 + messageLength] = stopByte;
	int frameLength = messageLength + 4;
	
	try{
		serialPort->write(outgoingFrame, frameLength);
	}
	catch(serial::IOException e){
		Logger::critical("Error while writing to serial port {} :", portName);
		Logger::critical("Error #{} : {}",  e.getErrorNumber(), e.what());
		close();
	}
	catch(serial::SerialException e){
		Logger::info("Error while writing to serial port {} :", portName);
		Logger::info("{}", e.what());
		b_portOpen = false;
		close();
	}
	catch(serial::PortNotOpenedException e){
		Logger::info("Error while writing to serial port {} :", portName);
		Logger::info("{}", e.what());
		close();	
	}
}

void SerialPort::sendMultiple(std::vector<std::vector<uint8_t>>& messages){
	if(!b_portOpen) return;

	uint8_t outBuffer[1024];
	int byteCount = 0;
	for(auto& msg : messages){
		uint8_t messageLength = msg.size();
		uint8_t* thisMessage = &outBuffer[byteCount];
		thisMessage[0] = startByte;
		thisMessage[1] = messageLength;
		memcpy(thisMessage + 2, &msg[0], messageLength);
		uint8_t checksum = messageLength;
		for(int i = 0; i < messageLength; i++) checksum ^= msg[i];
		thisMessage[2 + messageLength] = checksum;
		thisMessage[3 + messageLength] = stopByte;
		byteCount += messageLength + 4;
	}
	try{
		serialPort->write(outBuffer, byteCount);
	}
	catch(serial::IOException e){
		Logger::critical("Error while writing to serial port {} :", portName);
		Logger::critical("Error #{} : {}",  e.getErrorNumber(), e.what());
		close();
	}
	catch(serial::SerialException e){
		Logger::info("Error while writing to serial port {} :", portName);
		Logger::info("{}", e.what());
		b_portOpen = false;
		close();
	}
	catch(serial::PortNotOpenedException e){
		Logger::info("Error while writing to serial port {} :", portName);
		Logger::info("{}", e.what());
		close();	
	}

}


void SerialPort::readByte(uint8_t newByte){
	if(!b_portOpen) return;

	switch(incomingMessageState){
		case IncomingMessageState::EXPECTING_START_BYTE:
			if(newByte == startByte) incomingMessageState = IncomingMessageState::EXPECTING_LENGTH;
			break;
		case IncomingMessageState::EXPECTING_LENGTH:
			if(newByte == 0x0) incomingMessageState = IncomingMessageState::EXPECTING_START_BYTE;
			else {
				expectedIncomingMessageLength = newByte;
				currentIncomingMessageLength = 0;
				incomingMessageState = IncomingMessageState::READING_CONTENT;
			}
			break;
		case IncomingMessageState::READING_CONTENT:
			currentMessage[currentIncomingMessageLength] = newByte;
			currentIncomingMessageLength++;
			if(currentIncomingMessageLength == expectedIncomingMessageLength) incomingMessageState = IncomingMessageState::EXPECTING_CHECKSUM;
			break;
		case IncomingMessageState::EXPECTING_CHECKSUM:{
				uint8_t checksum = expectedIncomingMessageLength;
				for(uint8_t i = 0; i < expectedIncomingMessageLength; i++) checksum ^= currentMessage[i];
				if(checksum == newByte) incomingMessageState = IncomingMessageState::EXPECTING_STOP_BYTE;
				else incomingMessageState = IncomingMessageState::EXPECTING_START_BYTE;
			}break;
		case IncomingMessageState::EXPECTING_STOP_BYTE:
			incomingMessageState = IncomingMessageState::EXPECTING_START_BYTE;
			if(newByte == stopByte) messageReceivedCallback(currentMessage, currentIncomingMessageLength);
			break;
	}
	//return false;
 };


/*
 void printMessage(std::vector<uint8_t>& message){
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
 */
