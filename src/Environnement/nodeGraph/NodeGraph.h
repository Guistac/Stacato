#pragma once

#include "ioNode.h"
#include "ioData.h"
#include "ioLink.h"

class ioNode;
class ioData;
class ioLink;

class NodeGraph{
public:

	void addIoNode(std::shared_ptr<ioNode>);
	void removeIoNode(std::shared_ptr<ioNode>);

	bool isConnectionValid(std::shared_ptr<ioData>, std::shared_ptr<ioData>);
	std::shared_ptr<ioLink> connect(std::shared_ptr<ioData>, std::shared_ptr<ioData>);
	void disconnect(std::shared_ptr<ioLink>);

	std::shared_ptr<ioNode> getIoNode(int);
	std::shared_ptr<ioData> getIoData(int);
	std::shared_ptr<ioLink> getIoLink(int);

	std::vector<std::shared_ptr<ioNode>>& getIoNodes() { return ioNodeList; }
	std::vector<std::shared_ptr<ioData>>& getIoData() { return ioDataList; }
	std::vector<std::shared_ptr<ioLink>>& getIoLinks() { return ioLinkList; }

private:

	friend class ioNode;
	friend class ioData;
	friend class ioLink;

	std::vector<std::shared_ptr<ioNode>> ioNodeList;
	std::vector<std::shared_ptr<ioData>> ioDataList;
	std::vector<std::shared_ptr<ioLink>> ioLinkList;

	int uniqueID = 1; //counter to add new nodes, pins and links
};

