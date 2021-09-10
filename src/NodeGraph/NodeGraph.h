#pragma once

#include "ioNode.h"
#include "ioData.h"
#include "ioLink.h"

class ioNode;
class ioData;
class ioLink;

namespace tinyxml2 { class XMLElement; }

class NodeGraph {
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

	void evaluate();
	void evaluate(DeviceType deviceType);
	void evaluate(std::shared_ptr<ioNode> node);
	void evaluate(std::vector<std::shared_ptr<ioNode>> nodes);

	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);

	void nodeEditorGui();
	void centerView();
	void showFlow();

	bool b_showOutputValues = false;

private:

	friend class ioNode;
	friend class ioData;
	friend class ioLink;

	std::vector<std::shared_ptr<ioNode>> ioNodeList;
	std::vector<std::shared_ptr<ioData>> ioDataList;
	std::vector<std::shared_ptr<ioLink>> ioLinkList;

	//counter to add new nodes, pins and links
	//all items are odd numbers except for split node counterparts which are the an even number above the main node ID
	int uniqueID = 1;

	bool b_justLoaded = false;

	std::mutex mutex;
};

