#include <pch.h>

#include "OscDevice.h"

#include "Networking/Network.h"

#include <tinyxml2.h>
#include "OscSocket.h"

//Operating System Abstraction Layer from libSOEM
//Uselful for precise cross-platform thread sleep
#include <osal.h>

void OscDevice::assignIoData(){}

void OscDevice::connect(){
	oscSocket = std::make_shared<OscSocket>(4096);
	oscSocket->open(listeningPort, std::vector<int>({remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]}), remotePort);
	
	if(!oscSocket->isOpen()) {
		b_enabled = false;
		oscSocket = nullptr;
		return Logger::error("{} : Could not start Osc Socket", getName());
	}
	
	onConnection();
}

void OscDevice::disconnect(){
	onDisconnection();
}

bool OscDevice::isConnected(){
	return b_enabled;
}

bool OscDevice::isDetected(){
	return Network::isInitialized();
}

void OscDevice::onConnection(){
	Logger::info("{} : Started Osc Socket, Remote Address: {}.{}.{}.{}:{}, Listening on Port: {}", getName(), remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], remotePort, listeningPort);
	for(auto& outgoingMessage : outgoingMessages) outgoingMessage->startSendingRuntime(oscSocket);
	b_enabled = true;
}

void OscDevice::onDisconnection(){
	b_enabled = false;
	for(auto& outgoingMessage : outgoingMessages) outgoingMessage->stopSendingRuntime();
	oscSocket = nullptr;
	Logger::info("{} : Closed Osc Socket", getName());
}

void OscDevice::readInputs(){
	//incoming messages should be read by and asynchronous asio routine
}

void OscDevice::prepareOutputs(){
	//outgoing messages should be sent by their respective timing thread
}





void OscDevice::addOutgoingMessage(){
	std::shared_ptr<OSC::Message> message = std::make_shared<OSC::Message>();
	message->type = OSC::MessageType::OUTGOING_MESSAGE;
	outgoingMessages.push_back(message);
	selectMessage(message);
	updatePins();
}

void OscDevice::addIncomingMessage(){
	std::shared_ptr<OSC::Message> message = std::make_shared<OSC::Message>();
	message->type = OSC::MessageType::INCOMING_MESSAGE;
	incomingMessages.push_back(message);
	selectMessage(message);
	updatePins();
}

void OscDevice::deleteMessage(std::shared_ptr<OSC::Message> msg){
	int messageIndex = getMessageIndex(msg);
	if(messageIndex == -1) return;
	switch(msg->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			outgoingMessages.erase(outgoingMessages.begin() + messageIndex);
			if(isMessageSelected(msg)) selectedOutgoingMessage = nullptr;
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			incomingMessages.erase(incomingMessages.begin() + messageIndex);
			if(isMessageSelected(msg)) selectedIncomingMessage = nullptr;
			break;
	}
	updatePins();
}

void OscDevice::moveMessageUp(std::shared_ptr<OSC::Message> msg){
	int messageIndex = getMessageIndex(msg);
	switch(msg->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			if(messageIndex == -1 || messageIndex == 0) return;
			outgoingMessages.erase(outgoingMessages.begin() + messageIndex);
			outgoingMessages.insert(outgoingMessages.begin() + messageIndex - 1, msg);
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			if(messageIndex == -1 || messageIndex == 0) return;
			incomingMessages.erase(incomingMessages.begin() + messageIndex);
			incomingMessages.insert(incomingMessages.begin() + messageIndex - 1, msg);
			break;
	}
	updatePins();
}

void OscDevice::moveMessageDown(std::shared_ptr<OSC::Message> msg){
	int messageIndex = getMessageIndex(msg);
	switch(msg->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			if(messageIndex == -1 || messageIndex == outgoingMessages.size() - 1) return;
			outgoingMessages.erase(outgoingMessages.begin() + messageIndex);
			outgoingMessages.insert(outgoingMessages.begin() + messageIndex + 1, msg);
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			if(messageIndex == -1 || messageIndex == incomingMessages.size() - 1) return;
			incomingMessages.erase(incomingMessages.begin() + messageIndex);
			incomingMessages.insert(incomingMessages.begin() + messageIndex + 1, msg);
			break;
	}
	updatePins();
}

int OscDevice::getMessageIndex(std::shared_ptr<OSC::Message> msg){
	switch(msg->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			for(int i = 0; i < outgoingMessages.size(); i++) if(outgoingMessages[i] == msg) return i;
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			for(int i = 0; i < incomingMessages.size(); i++) if(incomingMessages[i] == msg) return i;
			break;
	}
	return -1;
}

void OscDevice::selectMessage(std::shared_ptr<OSC::Message> msg){
	switch(msg->type){
		case OSC::MessageType::INCOMING_MESSAGE: selectedIncomingMessage = msg; break;
		case OSC::MessageType::OUTGOING_MESSAGE: selectedOutgoingMessage = msg; break;
	}
}

bool OscDevice::isMessageSelected(std::shared_ptr<OSC::Message> msg){
	switch(msg->type){
		case OSC::MessageType::INCOMING_MESSAGE: return selectedIncomingMessage == msg;
		case OSC::MessageType::OUTGOING_MESSAGE: return selectedOutgoingMessage == msg;
	}
}

void OscDevice::addArgument(std::shared_ptr<OSC::Message> msg){
	std::shared_ptr<OSC::Argument> argument = std::make_shared<OSC::Argument>(msg);
	msg->arguments.push_back(argument);
	argument->setIndex(getMessageIndex(msg), msg->arguments.size());
	addIoData(argument->pin);
	updatePins();
}

void OscDevice::removeArgument(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg){
	int argumentIndex = msg->getArgumentIndex(arg);
	if(argumentIndex == -1) return;
	removeIoData(arg->pin);
	msg->arguments.erase(msg->arguments.begin() + argumentIndex);
	updatePins();
}

void OscDevice::moveArgumentUp(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg){
	int argumentIndex = msg->getArgumentIndex(arg);
	if(argumentIndex == -1 || argumentIndex == 0) return;
	msg->arguments.erase(msg->arguments.begin() + argumentIndex);
	msg->arguments.insert(msg->arguments.begin() + argumentIndex - 1, arg);
	updatePins();
}

void OscDevice::moveArgumentDown(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg){
	int argumentIndex = msg->getArgumentIndex(arg);
	if(argumentIndex == -1 || argumentIndex == msg->arguments.size() - 1) return;
	msg->arguments.erase(msg->arguments.begin() + argumentIndex);
	msg->arguments.insert(msg->arguments.begin() + argumentIndex + 1, arg);
	updatePins();
}

void OscDevice::updatePins(){
	for(int i = 0; i < outgoingMessages.size(); i++){
		for(int j = 0; j < outgoingMessages[i]->arguments.size(); j++){
			auto& arg = outgoingMessages[i]->arguments[j];
			arg->setIndex(i, j);
		}
	}
	for(int i = 0; i < incomingMessages.size(); i++){
		for(int j = 0; j < incomingMessages[i]->arguments.size(); j++){
			auto& arg = incomingMessages[i]->arguments[j];
			arg->setIndex(i, j);
		}
	}
	
	//erase all input pins from the nodes pin vectors
	std::vector<std::shared_ptr<NodePin>>& inputPins = getNodeInputData();
	std::vector<std::shared_ptr<NodePin>>& outputPins = getNodeOutputData();
	inputPins.clear();
	outputPins.clear();
	//and add them back in the correct order
	for(auto& outgoingMessage : outgoingMessages){
		for(auto& argument : outgoingMessage->arguments) inputPins.push_back(argument->pin);
	}
	for(auto& incomingMessage : incomingMessages){
		for(auto& argument : incomingMessage->arguments) outputPins.push_back(argument->pin);
	}
}









namespace OSC{

std::vector<ArgumentType> argumentTypes = {
	{ArgumentType::Type::FLOAT_DATA, "Float", "Float"},
	{ArgumentType::Type::DOUBLE_DATA, "Double", "Double"},
	{ArgumentType::Type::INTEGER_DATA, "Integer", "Integer"},
	{ArgumentType::Type::BOOLEAN_DATA, "Boolean", "Boolean"}
};

std::vector<ArgumentType>& getArgumentTypes(){
	return argumentTypes;
}

ArgumentType* getArgumentType(ArgumentType::Type t){
	for(auto& argtype : argumentTypes){
		if(t == argtype.type) return &argtype;
	}
	return nullptr;
}

ArgumentType* getArgumentType(const char* saveName){
	for(auto& argtype : argumentTypes){
		if(strcmp(argtype.saveName, saveName) == 0) return &argtype;
	}
	return nullptr;
}

Argument::Argument(std::shared_ptr<Message> msg) : parentMessage(msg){
	switch(parentMessage->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			pin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "dummyName");
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			pin = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "dummyName");
			break;
	}
	setType(type);
}

void Argument::setType(ArgumentType::Type t){
	type = t;
	switch(type){
		case ArgumentType::Type::FLOAT_DATA:
		case ArgumentType::Type::DOUBLE_DATA:
			pin->assignData(realValue);
			break;
		case ArgumentType::Type::INTEGER_DATA:
			pin->assignData(integerValue);
			break;
		case ArgumentType::Type::BOOLEAN_DATA:
			pin->assignData(booleanValue);
			break;
	}
}

void Argument::setIndex(int mesIndex, int argIndex){
	static char previousDefaultDisplayString[128];
	
	switch(parentMessage->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			sprintf((char*)pin->getSaveString(), "OutgoingMessage%iArgument%i", messageIndex, argumentIndex);
			
			//only update the display name if it is not the default one
			sprintf(previousDefaultDisplayString, "Outgoing Message %i Argument %i", messageIndex, argumentIndex);
			if(strcmp(pin->getDisplayString(), previousDefaultDisplayString) == 0 || strcmp(pin->getDisplayString(), "dummyName") == 0){
			   sprintf((char*)pin->getDisplayString(), "Outgoing Message %i Argument %i", mesIndex, argIndex);
			}
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			sprintf((char*)pin->getSaveString(), "IncomingMessage%iArgument%i", messageIndex, argumentIndex);
	
			//only update the display name if it is not the default one
			sprintf(previousDefaultDisplayString, "Incoming Message %i Argument %i", messageIndex, argumentIndex);
			if(strcmp(pin->getDisplayString(), previousDefaultDisplayString) == 0 || strcmp(pin->getDisplayString(), "dummyName") == 0){
				sprintf((char*)pin->getDisplayString(), "Incoming Message %i Argument %i", mesIndex, argIndex);
			}
			break;
	}
	messageIndex = mesIndex;
	argumentIndex = argIndex;
}

void Message::startSendingRuntime(std::shared_ptr<OscSocket> socket){
	
	oscSocket = socket;
	
	sendingRuntime = std::thread([this](){
		
		long long interval_nanoseconds = 1000000000.0 / outputFrequency_Hertz;
		b_isSending = true;
		
		long long cycleTime = Timing::getSystemTime_nanoseconds();
		long long previousTime = cycleTime - interval_nanoseconds;
		long long cycleDeltaTime;
		long long cycleDeltaTimeError;
		long long cycleDeltaTimeErrorSmoothed = 0.0;
		long long sleepTime = 0;
		int integralTerm = 0;
		
		long long printTime = 0;
		int cycleCount = 0;
		
		while(b_isSending){
			
			previousTime = cycleTime;
			cycleTime = Timing::getSystemTime_nanoseconds();
			cycleDeltaTime = cycleTime - previousTime;
			long long deltaTimeError = interval_nanoseconds - cycleDeltaTime;
			float deltaTimeErrorPercentage = 100.0 * ((float)deltaTimeError / (float)interval_nanoseconds);

			//timestamp based method
			
			int64_t timeError_nanoseconds = cycleTime % interval_nanoseconds;
			if (timeError_nanoseconds > interval_nanoseconds / 2) timeError_nanoseconds -= interval_nanoseconds;
			
			if (timeError_nanoseconds > 0) { integralTerm++; }
			else if (timeError_nanoseconds < 0) { integralTerm--; }
			
			sleepTime = interval_nanoseconds - (timeError_nanoseconds / 2) - (integralTerm * 5000);
			if(sleepTime < 0) sleepTime = 0;
			
			//interval based method
			/*
			int64_t timeError_nanoseconds = interval_nanoseconds - cycleDeltaTime;
			
			if (timeError_nanoseconds > 0) { integralTerm++; }
			else if (timeError_nanoseconds < 0) { integralTerm--; }
			
			sleepTime = interval_nanoseconds + (timeError_nanoseconds / 4) + (integralTerm * 5000);
			if(sleepTime < 0) sleepTime = 0;
			*/
			
			osal_usleep(sleepTime / 1000);
			
			//construct message
			std::shared_ptr<OscMessage> message = std::make_shared<OscMessage>(path);
			for(auto& argument : arguments){
				std::shared_ptr<NodePin> pin = argument->pin;
				if(pin->isConnected()) pin->updateValueFromConnectedPinValue();
				switch(argument->type){
					case OSC::ArgumentType::Type::FLOAT_DATA:
						message->addFloat(pin->get<double>());
						break;
					case OSC::ArgumentType::Type::DOUBLE_DATA:
						message->addDouble(argument->pin->get<double>());
						break;
					case OSC::ArgumentType::Type::INTEGER_DATA:
						message->addInt32(argument->pin->get<int>());
						break;
					case OSC::ArgumentType::Type::BOOLEAN_DATA:
						message->addBool(argument->pin->get<bool>());
						break;
				}
			}
			oscSocket->send(message);
			
			/*
			cycleCount++;
			float errorPercentage = 100.0f * (float)timeError_nanoseconds / (float)interval_nanoseconds;
			//Logger::info("error: {}%  {}  sleep: {}  integral: {}", deltaTimeErrorPercentage, deltaTimeError, sleepTime, integralTerm);
			if(cycleTime > printTime + 1000000000){
				printTime = cycleTime;
				Logger::warn("Cycles Per Second: {}", cycleCount);
				cycleCount = 0;
			}
			*/
		}
		
	});
}

void Message::stopSendingRuntime(){
	b_isSending = false;
	if(sendingRuntime.joinable()) sendingRuntime.join();
	oscSocket = nullptr;
}

int Message::getArgumentIndex(std::shared_ptr<Argument> arg){
	for(int i = 0; i < arguments.size(); i++) if(arguments[i] == arg) return i;
	return -1;
}


};




bool OscDevice::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* remoteIPXML = xml->InsertNewChildElement("RemoteIP");
	remoteIPXML->SetAttribute("ipv4-0", remoteIP[0]);
	remoteIPXML->SetAttribute("ipv4-1", remoteIP[1]);
	remoteIPXML->SetAttribute("ipv4-2", remoteIP[2]);
	remoteIPXML->SetAttribute("ipv4-3", remoteIP[3]);
	
	XMLElement* portsXML = xml->InsertNewChildElement("Ports");
	portsXML->SetAttribute("send", remotePort);
	portsXML->SetAttribute("receive", listeningPort);
	
	XMLElement* outgoingMessagesXML = xml->InsertNewChildElement("OutgoingMessages");
	for(auto& message : outgoingMessages){
		XMLElement* messageXML = outgoingMessagesXML->InsertNewChildElement("OSCMessage");
		messageXML->SetAttribute("Path", message->path);
		messageXML->SetAttribute("SendingFrequency", message->outputFrequency_Hertz);
		messageXML->SetAttribute("IncludeTimestamp", message->b_includeTimestamp);
		for(auto& argument : message->arguments){
			XMLElement* argumentXML = messageXML->InsertNewChildElement("OSCArgument");
			argumentXML->SetAttribute("DataType", OSC::getArgumentType(argument->type)->saveName);
			argumentXML->SetAttribute("PinSaveName", argument->pin->getSaveString());
		}
	}
	
	XMLElement* incomingMessagesXML = xml->InsertNewChildElement("IncomingMessages");
	for(auto& message : incomingMessages){
		XMLElement* messageXML = incomingMessagesXML->InsertNewChildElement("OSCMessage");
		messageXML->SetAttribute("Path", message->path);
		for(auto& argument : message->arguments){
			XMLElement* argumentXML = messageXML->InsertNewChildElement("OSCArgument");
			argumentXML->SetAttribute("DataType", OSC::getArgumentType(argument->type)->saveName);
			argumentXML->SetAttribute("PinSaveName", argument->pin->getSaveString());
		}
	}
	
	return true;
}


bool OscDevice::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;

	XMLElement* remoteIPXML = xml->FirstChildElement("RemoteIP");
	if(remoteIPXML == nullptr) return Logger::warn("Could not find remote ip attribute");
	int ipv4[4];
	if(remoteIPXML->QueryAttribute("ipv4-0", &ipv4[0]) != XML_SUCCESS) return Logger::warn("could not find ipv4#0 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-1", &ipv4[1]) != XML_SUCCESS) return Logger::warn("could not find ipv4#1 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-2", &ipv4[2]) != XML_SUCCESS) return Logger::warn("could not find ipv4#2 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-3", &ipv4[3]) != XML_SUCCESS) return Logger::warn("could not find ipv4#3 attribute");
	for(int i = 0; i < 4; i++) remoteIP[i] = ipv4[i];
	
	XMLElement* portsXML = xml->FirstChildElement("Ports");
	if(portsXML == nullptr) return Logger::warn("Could not find port attribute");
	int sendPort, receivePort;
	if(portsXML->QueryAttribute("send", &sendPort) != XML_SUCCESS) return Logger::warn("Could not find send port attribute");
	if(portsXML->QueryAttribute("receive", &receivePort) != XML_SUCCESS) return Logger::warn("Could not find receive port attribute");
	remotePort = sendPort;
	listeningPort = receivePort;
	
	XMLElement* outgoingMessagesXML = xml->FirstChildElement("OutgoingMessages");
	if(outgoingMessagesXML == nullptr) return Logger::warn("Could not find Outgoing Messages Attribute");
	
	XMLElement* outMessageXML = outgoingMessagesXML->FirstChildElement("OSCMessage");
	while(outMessageXML != nullptr){
		
		std::shared_ptr<OSC::Message> message = std::make_shared<OSC::Message>();
		message->type = OSC::MessageType::OUTGOING_MESSAGE;
		
		const char* path;
		if(outMessageXML->QueryStringAttribute("Path", &path) != XML_SUCCESS) return Logger::warn("Could not find message address attribute");
		strcpy(message->path, path);
		if(outMessageXML->QueryDoubleAttribute("SendingFrequency", &message->outputFrequency_Hertz) != XML_SUCCESS) return Logger::warn("Could not find sending frequency attribute");
		if(outMessageXML->QueryBoolAttribute("IncludeTimestamp", &message->b_includeTimestamp) != XML_SUCCESS) return Logger::warn("Could not find include timestamp attribute");
		
		XMLElement* argumentXML = outMessageXML->FirstChildElement("OSCArgument");
		
		while(argumentXML != nullptr){
			
			std::shared_ptr<OSC::Argument> argument = std::make_shared<OSC::Argument>(message);
			const char* dataTypeString;
			if(argumentXML->QueryStringAttribute("DataType", &dataTypeString) != XML_SUCCESS) return Logger::warn("could not find osc argument data type attribute");
			OSC::ArgumentType* argumentType = OSC::getArgumentType(dataTypeString);
			if(argumentType == nullptr) return Logger::warn("Could not identify osc argument type");
			argument->setType(argumentType->type);
			const char* pinSaveNameString;
			if(argumentXML->QueryStringAttribute("PinSaveName", &pinSaveNameString) != XML_SUCCESS) return Logger::warn("could not find osc argument pin save name attribute");
			strcpy((char*)argument->pin->getSaveString(), pinSaveNameString);
			
			argument->messageIndex = outgoingMessages.size();
			argument->argumentIndex = message->arguments.size();
			message->arguments.push_back(argument);
			addIoData(argument->pin);
			
			argumentXML = argumentXML->NextSiblingElement("OSCArgument");
		}
		
		outgoingMessages.push_back(message);
		
		outMessageXML = outMessageXML->NextSiblingElement("OSCMessage");
	}
	
	XMLElement* incomingMessagesXML = xml->FirstChildElement("IncomingMessages");
	if(incomingMessagesXML == nullptr) return Logger::warn("Could not find Incoming Messages Attribute");
	
	XMLElement* inMessageXML = incomingMessagesXML->FirstChildElement("OSCMessage");
	while(inMessageXML != nullptr){
		
		std::shared_ptr<OSC::Message> message = std::make_shared<OSC::Message>();
		message->type = OSC::MessageType::INCOMING_MESSAGE;
		
		const char* address;
		if(inMessageXML->QueryStringAttribute("Path", &address) != XML_SUCCESS) return Logger::warn("Could not find message address attribute");
		strcpy(message->path, address);
		
		XMLElement* argumentXML = inMessageXML->FirstChildElement("OSCArgument");
		
		while(argumentXML != nullptr){
			
			std::shared_ptr<OSC::Argument> argument = std::make_shared<OSC::Argument>(message);
			const char* dataTypeString;
			if(argumentXML->QueryStringAttribute("DataType", &dataTypeString) != XML_SUCCESS) return Logger::warn("could not find osc argument data type attribute");
			OSC::ArgumentType* argumentType = OSC::getArgumentType(dataTypeString);
			if(argumentType == nullptr) return Logger::warn("Could not identify osc argument type");
			argument->setType(argumentType->type);
			const char* pinSaveNameString;
			if(argumentXML->QueryStringAttribute("PinSaveName", &dataTypeString) != XML_SUCCESS) return Logger::warn("could not find osc argument pin save name attribute");
			strcpy((char*)argument->pin->getSaveString(), pinSaveNameString);
			
			argument->messageIndex = incomingMessages.size();
			argument->argumentIndex = message->arguments.size();
			message->arguments.push_back(argument);
			addIoData(argument->pin);
			
			argumentXML = argumentXML->NextSiblingElement("OSCArgument");
		}
		
		incomingMessages.push_back(message);
		
		inMessageXML = inMessageXML->NextSiblingElement("OSCMessage");
	}
	
	return true;
}
