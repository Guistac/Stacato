#pragma once

namespace serial{ class Serial; }

class SerialPort{
public:
	
	SerialPort(std::shared_ptr<serial::Serial> port, std::string& name);
	
	typedef std::function<void(uint8_t*,int)> MessageReceiveCallback;
	typedef std::function<void(void)> PortCloseCallback;
	void setMessageReceiveCallback(MessageReceiveCallback callback){ messageReceivedCallback = callback; }
	void setPortCloseCallback(PortCloseCallback callback) { portClosedCallback = callback; }
	
	void read();
	void send(uint8_t* buffer, int size);
	void sendMultiple(std::vector<std::vector<uint8_t>>& messages);

	void close();

	
private:
	
	void readByte(uint8_t byte);
	
	std::string portName;
	std::shared_ptr<serial::Serial> serialPort;
	bool b_portOpen = false;
	MessageReceiveCallback messageReceivedCallback = [](uint8_t*, int){};
	PortCloseCallback portClosedCallback = [](){};
	
	enum class IncomingMessageState{
		EXPECTING_START_BYTE,
		EXPECTING_LENGTH,
		READING_CONTENT,
		EXPECTING_CHECKSUM,
		EXPECTING_STOP_BYTE
	};
	uint8_t incomingBytes[512];
	uint8_t currentMessage[512];
	uint8_t outgoingFrame[512];
	IncomingMessageState incomingMessageState = IncomingMessageState::EXPECTING_START_BYTE;
	int currentIncomingMessageLength = 0;
	int expectedIncomingMessageLength = 0;
};

std::shared_ptr<SerialPort> findSerialPort(std::string& portMatchingString);
