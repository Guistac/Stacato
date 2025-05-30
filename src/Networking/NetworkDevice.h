#pragma once

#include "Environnement/NodeGraph/DeviceNode.h"

#define DEFINE_NETWORK_DEVICE(className, displayName, saveName)\
	public:\
	\
	/*Node Specific*/\
	DEFINE_NODE(className, displayName, saveName, Node::Type::IODEVICE, "none")\
	/*Device Specific*/\
	virtual Device::Type getDeviceType() override { return Type::NETWORK_DEVICE; }\
	virtual void readInputs() override;\
	virtual void writeOutputs() override;\
	virtual bool isDetected() override;\
	virtual bool isConnected() override;\
	virtual void onConnection() override;\
	virtual void onDisconnection() override;\
	\
	/*Network Device Specific*/\
	virtual void connect() override;\
	virtual void disconnect() override;\

class NetworkDevice : public Device{
public:
	
	virtual bool isDetected(){ return isConnected(); }
	
	virtual void connect() = 0;
	virtual void disconnect() = 0;
	
	
};
