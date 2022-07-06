#pragma once

namespace serial{ class Serial; }

class SerialPort{
public:

	SerialPort(std::string port) : portIdentificationString(port){}
	
	typedef std::function<void(uint8_t*,size_t)> MessageReceiveCallback;
	typedef std::function<void(void)> PortCloseCallback;
	void setMessageReceiveCallback(MessageReceiveCallback callback){ messageReceivedCallback = callback; }
	void setPortCloseCallback(PortCloseCallback callback) { portClosedCallback = callback; }
	
	void update();
	void send(uint8_t* buffer, size_t size);
	
	bool isOpen(){ return b_portOpen; }
	
private:
	
	void onIssue();
	void findPort();
	void read();
	void readMessage(uint8_t byte);
	
	std::string portIdentificationString;
	std::string portName;
	std::shared_ptr<serial::Serial> serialPort;
	bool b_portOpen = false;
	MessageReceiveCallback messageReceivedCallback = [](uint8_t*, size_t){};
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
	IncomingMessageState incomingMessageState = IncomingMessageState::EXPECTING_START_BYTE;
	int currentIncomingMessageLength = 0;
	int expectedIncomingMessageLength = 0;
};
