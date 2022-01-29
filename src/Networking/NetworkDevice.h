#pragma once

#include "NodeGraph/Device.h"

#define DEFINE_NETWORK_DEVICE(className, displayName, saveName)\
	public:\
	\
	/*Node Specific*/\
	DEFINE_NODE(className, displayName, saveName, Node::Type::IODEVICE, "none")\
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
