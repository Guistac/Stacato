#pragma once

//we include ioData.h so classes can be derived by including the ioNode.h file
#include "ioData.h"
#include "ioLink.h"

#define DEFINE_PROCESSOR_NODE(nodeName, className, category)	public:																							\
																virtual const char* getNodeName() { return nodeName; }											\
																static const char* getNodeNameStatic() { return nodeName; }										\
																virtual const char* getNodeCategory() { return category; }										\
																static const char* getNodeCategoryStatic() { return category; }									\
																className(){ setName(nodeName); }																\
																virtual NodeType getType() { return NodeType::PROCESSOR; }										\
																virtual DeviceType getDeviceType() { return DeviceType::NONE; }									\
																virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

#define DEFINE_CONTAINER_NODE(nodeName, className, category)	public:																							\
																virtual const char * getNodeName() { return nodeName; }											\
																static const char * getNodeNameStatic() { return nodeName; }									\
																virtual const char* getNodeCategory() { return category; }										\
																static const char* getNodeCategoryStatic() { return category; }									\
																className(){ setName(nodeName); }																\
																virtual NodeType getType() { return NodeType::CONTAINER; }										\
																virtual DeviceType getDeviceType() { return DeviceType::NONE; }									\
																virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

#define DEFINE_DEVICE_NODE(nodeName, className, deviceType, category)	public:																							\
																		virtual const char * getNodeName() { return nodeName; }											\
																		static const char * getNodeNameStatic() { return nodeName; }									\
																		virtual const char* getNodeCategory() { return category; }										\
																		static const char* getNodeCategoryStatic() { return category; }									\
																		className(){ setName(nodeName); }																\
																		virtual NodeType getType() { return NodeType::IODEVICE; }										\
																		virtual DeviceType getDeviceType() { return deviceType; }										\
																		virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

class NodeGraph;

enum NodeType {
	IODEVICE,
	PROCESSOR,
	AXIS,
	CONTAINER
};

enum DeviceType {
	CLOCK,
	ETHERCATSLAVE,
	NETWORKDEVICE,
	USBDEVICE,
	NONE
};

class ioNode {
public:

	virtual NodeType getType() = 0;
	virtual DeviceType getDeviceType() = 0;
	virtual const char* getNodeName() = 0;
	virtual const char* getNodeCategory() = 0;
	virtual std::shared_ptr<ioNode> getNewNodeInstance() = 0;

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }

	const char* getTypeString() {
		switch (getType()) {
			case IODEVICE: return "IODEVICE";
			case PROCESSOR: return "PROCESSOR";
			case AXIS: return "AXIS";
			case CONTAINER: return "CONTAINER";
			default: return "";
		}
	}

	const char* getDeviceTypeString() {
		switch (getDeviceType()) {
			case CLOCK: return "CLOCK";
			case ETHERCATSLAVE: return "ETHERCATSLAVE";
			case NETWORKDEVICE: return "NETWORKDEVICE";
			case USBDEVICE: return "USBDEVICE";
			case NONE: return "NONE";
			default: return "";
		}
	}

	void addIoData(std::shared_ptr<ioData> d);
	void removeIoData(std::shared_ptr<ioData> d);
	std::vector<std::shared_ptr<ioData>>& getNodeInputData() { return nodeInputData; }
	std::vector<std::shared_ptr<ioData>>& getNodeOutputData() { return nodeOutputData; }
	bool hasInputs() { return !nodeInputData.empty(); }
	bool hasOutputs() { return !nodeOutputData.empty(); }
	virtual void assignIoData() = 0;

	int getUniqueID() { return uniqueID; }
	bool isInNodeGraph() { return b_isInNodeGraph; }

	virtual void process(bool inputsValid) = 0;
	bool wasProcessed() { return b_wasProcessed; }
	bool areAllLinkedInputNodesProcessed();

	virtual void nodeGui(bool alwaysShowValue);
	virtual void propertiesGui(); //defined in nodegui.cpp
	virtual void nodeSpecificGui() {}

	//for device Nodes
	bool isOnline() { return b_isOnline; }
	void setOnline(bool b) { b_isOnline = b; }

	void getNodeGraphPosition(float& x, float& y);

private:

	friend class NodeGraph;
	friend class NodePin;
	friend class NodeLink;

	char name[128];

	NodeGraph* parentNodeGraph = nullptr;
	bool b_isInNodeGraph = false;
	int uniqueID = -1;

	std::vector<std::shared_ptr<ioData>> nodeInputData;
	std::vector<std::shared_ptr<ioData>> nodeOutputData;

	bool b_wasProcessed = false;
	bool b_circularDependencyFlag = false;

	bool b_isOnline = false;
};