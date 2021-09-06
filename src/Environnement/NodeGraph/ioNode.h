#pragma once

//we include ioData.h so classes can be derived by including the ioNode.h file
#include "ioData.h"

class NodeGraph;
class ioLink;

class ioNode{
public:

	enum NodeType {
		IODEVICE,
		PROCESSOR
	};

	virtual NodeType getType() = 0;
	
	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	int getUniqueID() { return uniqueID; }

	void addIoData(std::shared_ptr<ioData> d);
	void removeIoData(std::shared_ptr<ioData> d);

	std::vector<std::shared_ptr<ioData>>& getNodeInputData() { return nodeInputData; }
	std::vector<std::shared_ptr<ioData>>& getNodeOutputData() { return nodeOutputData; }

	bool isInNodeGraph() { return b_isInNodeGraph; }

	virtual void nodeGui();
	virtual void propertiesGui();
	virtual void nodeSpecificGui() = 0;

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
};

class DeviceNode : public ioNode {
public:

	enum DeviceType {
		ETHERCATSLAVE,
		NETWORKDEVICE,
		USBDEVICE,
		NONE
	};

	virtual NodeType getType() { return NodeType::IODEVICE; }
	virtual DeviceType getDeviceType() = 0;

	bool isOnline() { return b_isOnline; }
	void setOnline(bool b) { b_isOnline = b; }

private:

	bool b_isOnline = false;

};

class ProcessorNode : public ioNode {
public:

	virtual NodeType getType() { return NodeType::PROCESSOR; }

	virtual void process() = 0;

};
