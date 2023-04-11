#pragma once

#include "Networking/NetworkDevice.h"

class OscSocket;
class OscDevice;

namespace OSC{

	class Message;

	enum class MessageType{
		OUTGOING_MESSAGE,
		INCOMING_MESSAGE
	};

	struct ArgumentType{
		enum class Type{
			FLOAT_DATA,
			DOUBLE_DATA,
			INTEGER_DATA,
			BOOLEAN_DATA
		};
		Type type;
		char saveName[64];
		char displayName[64];
	};
 

	std::vector<ArgumentType>& getArgumentTypes();
	ArgumentType* getArgumentType(ArgumentType::Type t);
	ArgumentType* getArgumentType(const char* saveName);

	class Argument{
	public:
		Argument(std::shared_ptr<Message> msg);
		void setIndex(int messageIndex, int argumentIndex);
		void setType(ArgumentType::Type t);
		
		ArgumentType::Type type = ArgumentType::Type::FLOAT_DATA;
		std::shared_ptr<Message> parentMessage = nullptr;
		std::shared_ptr<NodePin> pin;
		
		std::shared_ptr<bool> booleanValue = std::make_shared<bool>(false);
		std::shared_ptr<int> integerValue = std::make_shared<int>(0);
		std::shared_ptr<double> realValue = std::make_shared<double>(0.0);
		
		//used to update argument name
		int messageIndex = -1;
		int argumentIndex = -1;
	};

	class Message : public std::enable_shared_from_this<Message>{
	public:
		MessageType type;
		char path[256] = "/Stacato/";
		bool b_includeTimestamp = false;
		double outputFrequency_Hertz = 1.0;
		
		std::vector<std::shared_ptr<Argument>> arguments;
		int getArgumentIndex(std::shared_ptr<Argument>);
		
		std::shared_ptr<OscSocket> oscSocket = nullptr;
		std::thread sendingRuntime;
		bool b_isSending = false;
		void startSendingRuntime(std::shared_ptr<OscSocket> socket);
		void stopSendingRuntime();
	};

}



class OscDevice : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(OscDevice)
	
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		NetworkDevice::onCopyFrom(source);
	};
	
	void networkGui();
	void outgoingMessagesGui();
	void incomingMessagesGui();
	void messageGui(std::shared_ptr<OSC::Message> msg);
	
	bool b_enabled = false;
	
	std::shared_ptr<OscSocket> oscSocket;
	
	uint8_t remoteIP[4] = {0, 0, 0, 0};
	uint16_t remotePort = 0;
	uint16_t listeningPort = 0;
	
	std::vector<std::shared_ptr<OSC::Message>> outgoingMessages;
	std::vector<std::shared_ptr<OSC::Message>> incomingMessages;
	
	void addOutgoingMessage();
	void addIncomingMessage();
	void deleteMessage(std::shared_ptr<OSC::Message> msg);
	void moveMessageUp(std::shared_ptr<OSC::Message> msg);
	void moveMessageDown(std::shared_ptr<OSC::Message> msg);
	int getMessageIndex(std::shared_ptr<OSC::Message> msg);
	
	void addArgument(std::shared_ptr<OSC::Message> msg);
	void removeArgument(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg);
	void moveArgumentUp(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument>);
	void moveArgumentDown(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument>);
	
	void updatePins();
	
	void selectMessage(std::shared_ptr<OSC::Message> msg);
	bool isMessageSelected(std::shared_ptr<OSC::Message> msg);
	
	std::shared_ptr<OSC::Message> selectedOutgoingMessage = nullptr;
	std::shared_ptr<OSC::Message> selectedIncomingMessage = nullptr;
	
	virtual void nodeSpecificGui();\
	virtual bool load(tinyxml2::XMLElement* xml);\
	virtual bool save(tinyxml2::XMLElement* xml);\
};


	
