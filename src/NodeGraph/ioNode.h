#pragma once

//we include ioData.h so classes can be derived by including the ioNode.h file
#include "ioData.h"
#include "ioLink.h"

#define DEFINE_PROCESSOR_NODE(nodeTypeName, className)	public:																	\
														virtual const char * getNodeTypeName() { return nodeTypeName; }			\
														static const char * getNodeTypeNameStatic() { return nodeTypeName; }	\
														className(){															\
															setName(nodeTypeName);												\
														}																		\
														virtual NodeType getType() { return NodeType::PROCESSOR; }				\
														virtual DeviceType getDeviceType() { return DeviceType::NONE; }			\

#define DEFINE_GROUP_NODE(nodeTypeName, className)		public:																	\
														virtual const char * getNodeTypeName() { return nodeTypeName; }			\
														static const char * getNodeTypeNameStatic() { return nodeTypeName; }	\
														className(){															\
															setName(nodeTypeName);												\
														}																		\
														virtual NodeType getType() { return NodeType::NODEGROUPER; }			\
														virtual DeviceType getDeviceType() { return DeviceType::NONE; }			\

#define DEFINE_DEVICE_NODE(nodeTypeName, className, deviceType)	public:																	\
																virtual const char * getNodeTypeName() { return nodeTypeName; }			\
																static const char * getNodeTypeNameStatic() { return nodeTypeName; }	\
																className(){															\
																	setName(nodeTypeName);												\
																}																		\
																virtual NodeType getType() { return NodeType::IODEVICE; }				\
																virtual DeviceType getDeviceType() { return deviceType; }				\

class NodeGraph;

enum NodeType {
	IODEVICE,
	PROCESSOR,
	NODEGROUPER
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

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	int getUniqueID() { return uniqueID; }

	virtual const char* getNodeTypeName() = 0;

	void addIoData(std::shared_ptr<ioData> d);
	void removeIoData(std::shared_ptr<ioData> d);

	std::vector<std::shared_ptr<ioData>>& getNodeInputData() { return nodeInputData; }
	std::vector<std::shared_ptr<ioData>>& getNodeOutputData() { return nodeOutputData; }
	bool hasInputs() { return !nodeInputData.empty(); }
	bool hasOutputs() { return !nodeOutputData.empty(); }

	bool isInNodeGraph() { return b_isInNodeGraph; }

	virtual void nodeGui(bool alwaysShowValue);
	virtual void propertiesGui();
	virtual void nodeSpecificGui() {}

	virtual void process(bool inputsValid) = 0;
	bool wasProcessed() { return b_wasProcessed; }

	//check if all nodes linked to the inputs of this node were processed
	bool areAllLinkedInputNodesProcessed() {
		for (auto inputData : nodeInputData) {
			for (auto inputDataLink : inputData->getLinks()) {
				if (!inputDataLink->getInputData()->getNode()->wasProcessed()) return false;
			}
		}
		return true;
	}

	virtual void assignIoData() = 0;

	//for device Nodes
	bool isOnline() { return b_isOnline; }
	void setOnline(bool b) { b_isOnline = b; }

private:

	friend class NodeGraph;
	friend class NodePin;
	friend class NodeLink;

	NodeGraph* parentNodeGraph = nullptr;
	int uniqueID = -1;

	char name[128];
	std::vector<std::shared_ptr<ioData>> nodeInputData;
	std::vector<std::shared_ptr<ioData>> nodeOutputData;

	bool b_isInNodeGraph = false;

	bool b_wasProcessed = false;
	bool b_circularDependencyFlag = false;

	bool b_isOnline = false;
};