#include <pch.h>
#include "Console.h"

#include "Serial.h"
#include <serial/serial.h>


constexpr uint8_t startByte = 0x33;
constexpr uint8_t stopByte = 0x99;
	
void SerialPort::onIssue(){
	b_portOpen = false;
	Logger::warn("Serial Port {} Closed", portName);
	portClosedCallback();
}

void SerialPort::update(){
	try{
		if(b_portOpen) read();
		else findPort();
	}
	catch(serial::IOException e){
		if(e.getErrorNumber() != 6) {
			Logger::critical("Error while reading from serial port {} :", portName);
			Logger::critical("Error #{} : {}",  e.getErrorNumber(), e.what());
		}
		onIssue();
	}
	catch(serial::SerialException e){
		Logger::info("Error while reading from serial port {} :", portName);
		Logger::info("{}", e.what());
		b_portOpen = false;
		onIssue();
	}
	catch(serial::PortNotOpenedException e){
		Logger::info("Error not opened Exception while reading from serial port {} :", portName);
		Logger::info("{}", e.what());
		onIssue();
	}
}


void SerialPort::findPort(){
	for(auto& port : serial::list_ports()){
		if(port.port.find(portIdentificationString) != std::string::npos){
			serialPort = std::make_shared<serial::Serial>(port.port);
			if(serialPort->isOpen()){
				serialPort->flush();
				b_portOpen = true;
				portName = port.port;
				Logger::info("Opened serial port {}", portName);
				return;
			}else serialPort.reset();
		}
	}
}


void SerialPort::send(uint8_t* message, size_t messageLength){
	
	try{
	
		//[0]   Start Byte
		//[1]   Length of the message content (L + 2)
		//[2]   Message Buffer Start
		//[2+L] Checksum (xor of all bytes after start end before stop, starting with message length)
		//[3+L] Stop Byte
		
		static uint8_t frame[512];
		frame[0] = startByte;
		frame[1] = messageLength;
		memcpy(frame + 2, message, messageLength);
		uint8_t checksum = messageLength;
		for(int i = 0; i < messageLength; i++) checksum ^= message[i];
		frame[2 + messageLength] = checksum;
		frame[3 + messageLength] = stopByte;
		
		size_t frameLength = messageLength + 4;
		serialPort->write(frame, frameLength);
		
	}
	catch(serial::IOException e){
		Logger::critical("Error while writing to serial port {} :", portName);
		Logger::critical("Error #{} : {}",  e.getErrorNumber(), e.what());
		onIssue();
	}
	catch(serial::SerialException e){
		Logger::info("Error while writing to serial port {} :", portName);
		Logger::info("{}", e.what());
		b_portOpen = false;
		onIssue();
	}
	catch(serial::PortNotOpenedException e){
		Logger::info("Error while writing to serial port {} :", portName);
		Logger::info("{}", e.what());
		onIssue();
	}
}
	
void SerialPort::read(){
	while(size_t byteCount = serialPort->available()){
		serialPort->read(incomingBytes, byteCount);
		for(int i = 0; i < byteCount; i++) readMessage(incomingBytes[i]);
	}
}

void SerialPort::readMessage(uint8_t newByte){
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
	return false;
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
