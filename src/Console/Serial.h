#pragma once

constexpr uint8_t startByte = 0x33;
constexpr uint8_t stopByte = 0x99;

//[0]   Start Byte
//[1]   Length of the message content (L + 2)
//[1+L] Checksum (xor of all bytes between Device Number and buffer end, starting from 0x0)
//[2+L] Stop Byte
//total size = 3 + L

void sendMessage(uint8_t* buffer, size_t size){
	
	static uint8_t outputBuffer[512];
	outputBuffer[0] = startByte;
	outputBuffer[1] = size;
	memcpy(outputBuffer + 2, buffer, size);
	uint8_t checksum = 0x0;
	for(int i = 0; i < size; i++) checksum ^= buffer[i];
	outputBuffer[1 + size] = checksum;
	outputBuffer[2 + size] = stopByte;
	
};

bool readMessage(uint8_t newByte, std::vector<uint8_t>& output) {
	
	enum class MessageState{
		EXPECTING_START,
		EXPECTING_LENGTH,
		READING_CONTENT,
		EXPECTING_CHECKSUM,
		EXPECTING_STOP
	};
	
	static uint8_t currentMessage[512];
	static MessageState state = MessageState::EXPECTING_START;
	static int currentLength = 0;
	static int expectedLength = 0;
	
	switch(state){
		case MessageState::EXPECTING_START:
			if(newByte == startByte) state = MessageState::EXPECTING_LENGTH;
			break;
		case MessageState::EXPECTING_LENGTH:
			if(newByte == 0x0) state = MessageState::EXPECTING_START;
			else {
				expectedLength = newByte;
				currentLength = 0;
				state = MessageState::READING_CONTENT;
			}
			break;
		case MessageState::READING_CONTENT:
			currentMessage[currentLength] = newByte;
			currentLength++;
			if(currentLength == expectedLength) state = MessageState::EXPECTING_CHECKSUM;
			break;
		case MessageState::EXPECTING_CHECKSUM:{
				uint8_t checksum = 0x0;
				for(uint8_t i = 0; i < expectedLength; i++) checksum ^= currentMessage[i];
				if(checksum == newByte) state = MessageState::EXPECTING_STOP;
				else state = MessageState::EXPECTING_START;
			}break;
		case MessageState::EXPECTING_STOP:
			state = MessageState::EXPECTING_START;
			if(newByte == stopByte) {
				output = std::vector<uint8_t>(currentMessage, currentMessage + currentLength);
				return true;
			}
			break;
	}
	return false;
};



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
