#pragma once

#include "Node.h"

#define DEFINE_DEVICE_NODE(className, category)\
	DEFINE_NODE(className, Node::Type::IODEVICE, category)\
	virtual bool isDetected() override;\
	virtual bool isConnected() override;\
	virtual void onConnection() override;\
	virtual void onDisconnection() override;\
	virtual void readInputs() override;\
	virtual void writeOutputs() override;\

class Device : public Node {
public:

	//for device nodes, processing takes place inside the device.
	//instead we only prepare input when data is available
	//and prepare outputs when new data is send to the device
	virtual void readInputs() = 0;
	virtual void writeOutputs() = 0;

	virtual bool isDetected() = 0;
	virtual bool isConnected() = 0;
	virtual void onConnection() = 0;
	virtual void onDisconnection() = 0;
};
