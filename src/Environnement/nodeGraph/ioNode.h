#pragma once

//we include ioData.h so classes can be derived by including the ioNode.h file
#include "ioData.h"

class NodeGraph;
class ioLink;

class ioNode{
public:

	enum NodeType {
		EtherCatSlave,
		Axis,
		Processor,
		NetworkDevice
	};

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	int getUniqueID() { return uniqueID; }

	void addIoData(std::shared_ptr<ioData> d);
	void removeIoData(std::shared_ptr<ioData> d);

	std::vector<std::shared_ptr<ioData>>& getNodeInputData() { return nodeInputData; }
	std::vector<std::shared_ptr<ioData>>& getNodeOutputData() { return nodeOutputData; }

	bool isInNodeGraph() { return b_isInNodeGraph; }

	NodeType getType() { return type; }

	virtual void gui();

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

protected:

	NodeType type;
};

