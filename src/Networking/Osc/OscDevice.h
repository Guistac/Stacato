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

/*
	enum class ArgumentType{
		FLOAT_DATA,
		DOUBLE_DATA,
		INTEGER_DATA,
		BOOLEAN_DATA
	};

	const char* getSaveName(ArgumentType t);
	const char* getDisplayName(ArgumentType t);
	ArgumentType getType(const char* saveName);
*/

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
		
		ArgumentType::Type type = ArgumentType::Type::FLOAT_DATA;
		std::shared_ptr<Message> parentMessage = nullptr;
		std::shared_ptr<NodePin> pin;
		
		void setType(ArgumentType::Type t);
	};

	class Message : public std::enable_shared_from_this<Message>{
	public:
		char address[256] = "/Stacato/";
		std::vector<std::shared_ptr<Argument>> arguments;
		uint64_t timestamp = 0;
		
		double outputFrequency_Hertz = 1.0;
		std::thread runtime;
		
		MessageType type;
		bool b_includeTimestamp = false;
		
		int getArgumentIndex(std::shared_ptr<Argument>);
	};

}



class OscDevice : public NetworkDevice{
	DEFINE_NETWORK_DEVICE(OscDevice, "Osc Device", "OscDevice")
	
	void networkGui();
	void dataGui();
	
	bool b_enabled = false;
	
	std::shared_ptr<OscSocket> oscSocket;
	
	std::thread runtime;
	
	int remoteIP[4] = {0, 0, 0, 0};
	int remotePort = 0;
	int listeningPort = 0;
	
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
	
	std::shared_ptr<OSC::Message> selectedMessage = nullptr;
};


	
