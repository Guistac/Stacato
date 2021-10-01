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

	void addNode(std::shared_ptr<Node>);
	void removeNode(std::shared_ptr<Node>);

	bool isConnectionValid(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	std::shared_ptr<NodeLink> connect(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	void disconnect(std::shared_ptr<NodeLink>);

	std::shared_ptr<Node> getNode(int);
	std::shared_ptr<NodePin> getPin(int);
	std::shared_ptr<NodeLink> getLink(int);

	std::vector<std::shared_ptr<Node>>& getNodes() { return nodes; }
	std::vector<std::shared_ptr<NodePin>>& getPins() { return pins; }
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return links; }

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

	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<std::shared_ptr<NodeLink>> links;

	//counter to add new nodes, pins and links
	//all items are odd numbers except for split node counterparts which are the an even number above the main node ID
	int uniqueID = 1;

	bool b_justLoaded = false;

	std::mutex mutex;
};

