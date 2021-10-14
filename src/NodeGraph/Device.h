#pragma once

#include "Node.h"

#define DEFINE_DEVICE_NODE(nodeName, className, deviceType, category)	public:																							\
																		virtual const char * getNodeName() { return nodeName; }											\
																		virtual const char* getNodeCategory() { return category; }										\
																		className(){ setName(nodeName); }																\
																		virtual Node::Type getType() { return Node::Type::IODEVICE; }									\
																		virtual Device::Type getDeviceType() { return deviceType; }										\
																		virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }	\
																		virtual bool isOnline();																		\
																		virtual bool isReady();																			\
																		virtual bool isEnabled();																		\
																		virtual bool hasError();																		\
																		virtual const char* getErrorString();															\
																		virtual void enable();																			\
																		virtual void disable();																			\
																		virtual void clearError();																		\

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
	virtual void onConnection() = 0;
	virtual void onDisconnection() = 0;

	virtual bool isDetected() = 0;
	virtual bool isOnline() = 0;
	virtual bool isReady() = 0;
	virtual bool isEnabled() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;

};

struct DeviceType {
	Device::Type type;
	const char* saveName;
	const char* displayName;
};

DeviceType* getDeviceType(const char* saveName);
DeviceType* getDeviceType(Device::Type t);
