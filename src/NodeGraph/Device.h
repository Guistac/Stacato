#pragma once

#include "Node.h"

#define DEFINE_DEVICE_NODE(className, displayName, saveName, deviceType, category)\
	DEFINE_NODE(className, displayName, saveName, Node::Type::IODEVICE, category)\
	virtual Device::Type getDeviceType() { return deviceType; }\
	virtual bool isDetected();\
	virtual bool isConnected();\
	virtual void onConnection();\
	virtual void onDisconnection();\
	virtual void readInputs();\
	virtual void prepareOutputs();\

class Device : public Node {
public:

	enum class Type {
		ETHERCAT_DEVICE,
		NETWORK_DEVICE,
		USB_DEVICE
	};

	virtual Type getDeviceType() = 0;

	//we define process() here so device subclasses don't have to define it.
	//for device nodes, processing takes place inside the device.
	//instead we only prepare input when data is available
	//and prepare outputs when new data is send to the device
	virtual void process() {}
	virtual void readInputs() = 0;
	virtual void prepareOutputs() = 0;

	virtual bool isDetected() = 0;
	virtual bool isConnected() = 0;
	virtual void onConnection() = 0;
	virtual void onDisconnection() = 0;
};

struct DeviceType {
	Device::Type type;
	const char* saveName;
	const char* displayName;
};

DeviceType* getDeviceType(const char* saveName);
DeviceType* getDeviceType(Device::Type t);
