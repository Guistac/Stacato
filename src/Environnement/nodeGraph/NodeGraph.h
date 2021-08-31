#pragma once

#include "ioNode.h"
#include "ioData.h"
#include "ioLink.h"

class ioNode;
class ioData;
class ioLink;

class NodeGraph{
public:

	void addIoNode(ioNode*);
	void removeIoNode(ioNode*);

	bool isConnectionValid(ioData*, ioData*);
	ioLink* connect(ioData*, ioData*);
	void disconnect(ioLink*);

	ioNode* getIoNode(int);
	ioData* getIoData(int);
	ioLink* getIoLink(int);

	std::vector<ioNode*>& getNodes() { return ioNodeList; }
	std::vector<ioData*>& getPins() { return ioDataList; }
	std::vector<ioLink*>& getLinks() { return ioLinkList; }

private:

	friend class ioNode;
	friend class ioData;
	friend class ioLink;

	std::vector<ioNode*> ioNodeList;
	std::vector<ioData*> ioDataList;
	std::vector<ioLink*> ioLinkList;

	int uniqueID = 1; //counter to add new nodes, pins and links
};

