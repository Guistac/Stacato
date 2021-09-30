#pragma once

#include "Node.h"
#include "NodePin.h"
#include "NodeLink.h"
#include "DeviceNode.h"

class Node;
class NodePin;
class NodeLink;

namespace tinyxml2 { class XMLElement; }

class NodeGraph {
public:

	void addIoNode(std::shared_ptr<Node>);
	void removeIoNode(std::shared_ptr<Node>);

	bool isConnectionValid(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	std::shared_ptr<NodeLink> connect(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	void disconnect(std::shared_ptr<NodeLink>);

	std::shared_ptr<Node> getIoNode(int);
	std::shared_ptr<NodePin> getIoData(int);
	std::shared_ptr<NodeLink> getIoLink(int);

	std::vector<std::shared_ptr<Node>>& getIoNodes() { return NodeList; }
	std::vector<std::shared_ptr<NodePin>>& getIoData() { return NodePinList; }
	std::vector<std::shared_ptr<NodeLink>>& getIoLinks() { return NodeLinkList; }

	void evaluate();
	void evaluate(DeviceType deviceType);
	void evaluate(std::shared_ptr<Node> node);
	void evaluate(std::vector<std::shared_ptr<Node>> nodes);

	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);

	void nodeEditorGui();
	void centerView();
	void showFlow();

	bool b_showOutputValues = false;

private:

	friend class Node;
	friend class NodePin;
	friend class NodeLink;

	std::vector<std::shared_ptr<Node>> NodeList;
	std::vector<std::shared_ptr<NodePin>> NodePinList;
	std::vector<std::shared_ptr<NodeLink>> NodeLinkList;

	//counter to add new nodes, pins and links
	//all items are odd numbers except for split node counterparts which are the an even number above the main node ID
	int uniqueID = 1;

	bool b_justLoaded = false;

	std::mutex mutex;
};

