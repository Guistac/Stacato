#include <pch.h>

#include "OscDevice.h"

#include "Networking/Network.h"

#include <tinyxml2.h>
#include "OscSocket.h"

//SOEM Operating System Abstraction Layer
//Uselful for precise thread sleep
#include <osal.h>

void OscDevice::assignIoData(){}

void OscDevice::connect(){
	oscSocket = std::make_shared<OscSocket>(4096);
	oscSocket->open(listeningPort, std::vector<int>({remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]}), remotePort);
	
	if(!oscSocket->isOpen()) return Logger::error("{} : Could not start Osc Socket", getName());
	Logger::info("{} : Started Osc Socket, Remote Address: {}.{}.{}.{}:{}  Listening Port: {}", getName(), remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], remotePort, listeningPort);
	
	b_enabled = true;
	runtime = std::thread([this](){
		Logger::critical("START OSC THREAD");

		float frequency = 120.0;
		long long interval_nanoseconds = 1000000000.0 / frequency;
		
		long long cycleTime = Timing::getSystemTime_nanoseconds();
		long long previousTime = cycleTime - interval_nanoseconds;
		long long cycleDeltaTime;
		long long cycleDeltaTimeError;
		long long cycleDeltaTimeErrorSmoothed = 0.0;
		long long sleepTime = 0;
		int integralTerm = 0;
		
		long long printTime = 0;
		int cycleCount = 0;
		
		while(b_enabled){
			
			previousTime = cycleTime;
			cycleTime = Timing::getSystemTime_nanoseconds();
			cycleDeltaTime = cycleTime - previousTime;
			long long deltaTimeError = interval_nanoseconds - cycleDeltaTime;
			float deltaTimeErrorPercentage = 100.0 * ((float)deltaTimeError / (float)interval_nanoseconds);
			
#define METHOD_1
#ifdef METHOD_1
			
			//timestamp based method
			
			int64_t timeError_nanoseconds = cycleTime % interval_nanoseconds;
			if (timeError_nanoseconds > interval_nanoseconds / 2) timeError_nanoseconds -= interval_nanoseconds;
			
			if (timeError_nanoseconds > 0) { integralTerm++; }
			else if (timeError_nanoseconds < 0) { integralTerm--; }
			
			sleepTime = interval_nanoseconds - (timeError_nanoseconds / 2) - (integralTerm * 5000);
			if(sleepTime < 0) sleepTime = 0;
			
#else
			
			//interval based method
			
			int64_t timeError_nanoseconds = interval_nanoseconds - cycleDeltaTime;
			
			if (timeError_nanoseconds > 0) { integralTerm++; }
			else if (timeError_nanoseconds < 0) { integralTerm--; }
			
			sleepTime = interval_nanoseconds + (timeError_nanoseconds / 4) + (integralTerm * 5000);
			if(sleepTime < 0) sleepTime = 0;
			
#endif
			
			osal_usleep(sleepTime / 1000);
			
			std::shared_ptr<OscMessage> message = std::make_shared<OscMessage>("/Stacato/TestMessage");
			message->addFloat(Timing::getProgramTime_seconds());
			oscSocket->send(message);
			
			cycleCount++;
			
			float errorPercentage = 100.0f * (float)timeError_nanoseconds / (float)interval_nanoseconds;
			//Logger::info("error: {}%  {}  sleep: {}  integral: {}", deltaTimeErrorPercentage, deltaTimeError, sleepTime, integralTerm);
			
			if(cycleTime > printTime + 1000000000){
				printTime = cycleTime;
				Logger::warn("Cycles Per Second: {}", cycleCount);
				cycleCount = 0;
			}
		}
		Logger::critical("EXIT OSC THREAD");
	});
	
}

void OscDevice::disconnect(){
	b_enabled = false;
	runtime.join();
	oscSocket = nullptr;
	Logger::info("{} : Closed Osc Socket", getName());
}

bool OscDevice::isConnected(){}

bool OscDevice::isDetected(){}

void OscDevice::onConnection(){}

void OscDevice::onDisconnection(){}

void OscDevice::readInputs(){}

void OscDevice::prepareOutputs(){}



void OscDevice::addOutgoingMessage(){
	std::shared_ptr<OSC::Message> message = std::make_shared<OSC::Message>();
	message->type = OSC::MessageType::OUTGOING_MESSAGE;
	outgoingMessages.push_back(message);
	selectMessage(message);
}

void OscDevice::addIncomingMessage(){
	std::shared_ptr<OSC::Message> message = std::make_shared<OSC::Message>();
	message->type = OSC::MessageType::INCOMING_MESSAGE;
	incomingMessages.push_back(message);
	selectMessage(message);
}

void OscDevice::deleteMessage(std::shared_ptr<OSC::Message> msg){
	int messageIndex = getMessageIndex(msg);
	if(messageIndex == -1) return;
	selectedMessage = nullptr;
	switch(msg->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			outgoingMessages.erase(outgoingMessages.begin() + messageIndex);
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			incomingMessages.erase(incomingMessages.begin() + messageIndex);
			break;
	}
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
	selectedMessage = msg;
}

bool OscDevice::isMessageSelected(std::shared_ptr<OSC::Message> msg){
	return selectedMessage == msg;
}

void OscDevice::addArgument(std::shared_ptr<OSC::Message> msg){
	std::shared_ptr<OSC::Argument> argument = std::make_shared<OSC::Argument>(msg);
	msg->arguments.push_back(argument);
	addIoData(argument->pin);
	updatePins();
}

void OscDevice::removeArgument(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg){
	int argumentIndex = msg->getArgumentIndex(arg);
	if(argumentIndex == -1) return;
	removeIoData(arg->pin);
	msg->arguments.erase(msg->arguments.begin() + argumentIndex);
}

void OscDevice::moveArgumentUp(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg){
	int argumentIndex = msg->getArgumentIndex(arg);
	if(argumentIndex == -1 || argumentIndex == 0) return;
	msg->arguments.erase(msg->arguments.begin() + argumentIndex);
	msg->arguments.insert(msg->arguments.begin() + argumentIndex - 1, arg);
}

void OscDevice::moveArgumentDown(std::shared_ptr<OSC::Message> msg, std::shared_ptr<OSC::Argument> arg){
	int argumentIndex = msg->getArgumentIndex(arg);
	if(argumentIndex == -1 || argumentIndex == msg->arguments.size() - 1) return;
	msg->arguments.erase(msg->arguments.begin() + argumentIndex);
	msg->arguments.insert(msg->arguments.begin() + argumentIndex + 1, arg);
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
		for(auto& argument : incomingMessage->arguments) inputPins.push_back(argument->pin);
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
			pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "dummyName");
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "dummyName");
			break;
	}
}

void Argument::setType(ArgumentType::Type t){
	type = t;
}

int Message::getArgumentIndex(std::shared_ptr<Argument> arg){
	for(int i = 0; i < arguments.size(); i++) if(arguments[i] == arg) return i;
	return -1;
}

void Argument::setIndex(int messageIndex, int argumentIndex){
	switch(parentMessage->type){
		case OSC::MessageType::OUTGOING_MESSAGE:
			sprintf((char*)pin->getSaveName(), "OutgoingMessage%iArgument%i", messageIndex, argumentIndex);
			sprintf((char*)pin->getDisplayName(), "Outgoing Message %i Argument %i", messageIndex, argumentIndex);
			break;
		case OSC::MessageType::INCOMING_MESSAGE:
			sprintf((char*)pin->getSaveName(), "IncomingMessage%iArgument%i", messageIndex, argumentIndex);
			sprintf((char*)pin->getDisplayName(), "Incoming Message %i Argument %i", messageIndex, argumentIndex);
			break;
	}
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
	
	return true;
}


bool OscDevice::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;

	XMLElement* remoteIPXML = xml->FirstChildElement("RemoteIP");
	if(remoteIPXML == nullptr) return Logger::warn("Could not find remote ip attribute");
	if(remoteIPXML->QueryAttribute("ipv4-0", &remoteIP[0]) != XML_SUCCESS) return Logger::warn("could not find ipv4#0 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-1", &remoteIP[1]) != XML_SUCCESS) return Logger::warn("could not find ipv4#1 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-2", &remoteIP[2]) != XML_SUCCESS) return Logger::warn("could not find ipv4#2 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-3", &remoteIP[3]) != XML_SUCCESS) return Logger::warn("could not find ipv4#3 attribute");
	
	XMLElement* portsXML = xml->FirstChildElement("Ports");
	if(portsXML == nullptr) return Logger::warn("Could not find port attribute");
	if(portsXML->QueryAttribute("send", &remotePort) != XML_SUCCESS) return Logger::warn("Could not find send port attribute");
	if(portsXML->QueryAttribute("receive", &listeningPort) != XML_SUCCESS) return Logger::warn("Could not find receive port attribute");
	
	return true;
}
