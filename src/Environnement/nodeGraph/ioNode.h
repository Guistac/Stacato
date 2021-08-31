#pragma once

//we include ioData.h so classes can be derived by including the ioNode.h file
#include "ioData.h"

class NodeGraph;
class ioLink;

class ioNode{
public:

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	int getUniqueID() { return uniqueID; }

	void addIoData(ioData* d);
	void removeIoData(ioData* d);

	std::vector<ioData*>& getNodeInputData() { return nodeInputData; }
	std::vector<ioData*>& getNodeOutputData() { return nodeOutputData; }

private:

	friend class NodeGraph;
	friend class NodePin;
	friend class NodeLink;

	NodeGraph* parentNodeGraph = nullptr;
	int uniqueID = -1;

	char name[128];
	std::vector<ioData*> nodeInputData;
	std::vector<ioData*> nodeOutputData;

};

