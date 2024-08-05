#pragma once

class SerialPort;
class ConsoleMapping;
class IODevice;

class Console : public std::enable_shared_from_this<Console>{
public:
	
    static std::shared_ptr<Console> initialize(std::shared_ptr<SerialPort> port);
    static void terminate(std::shared_ptr<Console> console);
        
    bool isConnected(){ return connectionState == ConnectionState::CONNECTED; }
    bool isConnecting(){
        switch(connectionState){
            case ConnectionState::NOT_CONNECTED:
            case ConnectionState::CONNECTION_REQUESTED:
            case ConnectionState::CONNECTION_CONFIRMATION_RECEIVED:
            case ConnectionState::IDENTIFICATION_REQUESTED:
            case ConnectionState::IDENTIFICATION_RECEIVED:
            case ConnectionState::DEVICE_ENUMERATION_REQUESTED:
            case ConnectionState::DEVICE_ENUMERATION_RECEIVED: return true;
            default: return false;
        }
    }
    bool wasDisconnected(){ return connectionState == ConnectionState::DISCONNECTED; }
    
    std::string& getName(){ return consoleName; }
	std::vector<std::shared_ptr<IODevice>>& getIODevices(){ return ioDevices; }
	
	void setMapping(std::shared_ptr<ConsoleMapping> mapping_);
	std::shared_ptr<ConsoleMapping> getMapping(){ return mapping; }
	
private:
	
	std::thread inputHandler;
	std::thread outputHandler;
	bool b_inputHandlerRunning = false;
	bool b_outputHandlerRunning = false;
	void connect();
	void updateInputs();
	void updateOutputs();
	void handleTimeout();

	enum MessageHeader{
		CONNECTION_REQUEST 					= 0x0, //[2] uint16_t (timeout in milliseconds)
		CONNECTION_CONFIRMATION 			= 0x1, //[2] uint16_t (timeout in milliseconds)
		HEARTBEAT 							= 0x2, //[0]
		IDENTIFICATION_REQUEST 				= 0x3, //[0]
		IDENTIFICATION_REPLY 				= 0x4, //[x] string (device name)
		DEVICE_ENUMERATION_REQUEST 			= 0x5, //[0]
		DEVICE_ENUMERATION_REPLY 			= 0x6, //[deviceCount] uint8_t (deviceTypeCode for each iodevice)
		ALL_INPUT_DEVICES_STATE_REQUEST 	= 0x7  //[0]
	};

	enum class ConnectionState{
		NOT_CONNECTED,
		CONNECTION_REQUESTED,
		CONNECTION_CONFIRMATION_RECEIVED,
		IDENTIFICATION_REQUESTED,
		IDENTIFICATION_RECEIVED,
		DEVICE_ENUMERATION_REQUESTED,
		DEVICE_ENUMERATION_RECEIVED,
		CONNECTED,
		DISCONNECTED
	};

	std::shared_ptr<SerialPort> serialPort;
	ConnectionState connectionState = ConnectionState::NOT_CONNECTED;
	std::string consoleName;
	std::vector<std::shared_ptr<IODevice>> ioDevices;
	std::shared_ptr<ConsoleMapping> mapping;

	uint16_t timeoutDelay_milliseconds = 512;
    uint8_t heartbeatInterval_milliseconds = 25;
	int connectionTimeoutDelay_milliseconds = 500;
	double lastHeartbeatReceiveTime;
	double lastHeartbeatSendTime;
	double connectionRequestTime;

	
	uint8_t outputMessage[128];

	void readMessage(uint8_t*, int);
	void receiveConnectionConfirmation(uint8_t* message, int size);
	void receiveIdentificationReply(uint8_t* message, int size);
	void receiveDeviceEnumerationReply(uint8_t* message, int size);
	void receiveHeartbeat(uint8_t* message, int size);
	void receiveDeviceInput(uint8_t* message, int size);
	
	void sendConnectionRequest();
	void sendIdentificationRequest();
	void sendDeviceEnumerationRequest();
	void sendConnectionConfirmation();
	void sendHeartbeat();
	
	void onDisconnection();
	void onConnection();
	
};
