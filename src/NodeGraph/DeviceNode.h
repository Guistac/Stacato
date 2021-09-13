#pragma once

#include "ioNode.h"

#define DEFINE_DEVICE_NODE(nodeName, className, deviceType, category)	public:																							\
																		virtual const char * getNodeName() { return nodeName; }											\
																		virtual const char* getNodeCategory() { return category; }										\
																		className(){ setName(nodeName); }																\
																		virtual NodeType getType() { return NodeType::IODEVICE; }										\
																		virtual DeviceType getDeviceType() { return deviceType; }										\
																		virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\
																		virtual bool isOnline();																		\
																		virtual bool isReady();																			\
																		virtual bool isEnabled();																		\
																		virtual bool hasError();																		\
																		virtual const char* getErrorString();															\
																		virtual void enable();																			\
																		virtual void disable();																			\
																		virtual void clearError();																		\

enum DeviceType {
	ETHERCATSLAVE,
	NETWORKDEVICE,
	USBDEVICE
};

class DeviceNode : public ioNode {
public:

	virtual DeviceType getDeviceType() = 0;

	const char* getDeviceTypeString() {
		switch (getDeviceType()) {
		case ETHERCATSLAVE: return "ETHERCATSLAVE";
		case NETWORKDEVICE: return "NETWORKDEVICE";
		case USBDEVICE: return "USBDEVICE";
		default: return "";
		}
	}

	virtual bool isOnline() = 0;
	virtual bool isReady() = 0;
	virtual bool isEnabled() = 0;

	virtual bool hasError() = 0;
	virtual const char* getErrorString() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void clearError() = 0;

};