#pragma once

#include "NodeGraph/Device.h"

#define DEFINE_NETWORK_DEVICE(className, displayName, saveName)\
	public:\
	\
	/*Node Specific*/\
	virtual const char * getSaveName() { return saveName; }\
	virtual const char* getNodeCategory() { return "none"; }\
	className(){ setName(displayName); }\
	virtual Node::Type getType() { return Node::Type::IODEVICE; }\
	virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }\
	virtual void assignIoData();\
	virtual void nodeSpecificGui();\
	\
	/*Device Specific*/\
	virtual Device::Type getDeviceType() { return Type::NETWORK_DEVICE; }\
	virtual void readInputs();\
	virtual void prepareOutputs();\
	virtual bool isDetected();\
	virtual bool isConnected();\
	virtual void onConnection();\
	virtual void onDisconnection();\
	\
	/*Network Device Specific*/\
	virtual void connect();\
	virtual void disconnect();\

class NetworkDevice : public Device{
public:
	
	virtual bool isDetected(){ return isConnected(); }
	
	virtual void connect() = 0;
	virtual void disconnect() = 0;
};
