#pragma once

#include "Environnement/DeviceNode.h"
#include "NodePin.h"
#include "NodeLink.h"

namespace tinyxml2 { class XMLElement; }

namespace Environnement::NodeGraph{

	void addNode(std::shared_ptr<Node>);
	void removeNode(std::shared_ptr<Node>);

	bool isConnectionValid(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	std::shared_ptr<NodeLink> connect(std::shared_ptr<NodePin>, std::shared_ptr<NodePin>);
	void disconnect(std::shared_ptr<NodeLink>);

	std::shared_ptr<Node> getNode(int);
	std::shared_ptr<NodePin> getPin(int);
	std::shared_ptr<NodeLink> getLink(int);

	std::vector<std::shared_ptr<Node>>& getNodes();
	std::vector<std::shared_ptr<NodePin>>& getPins();
	std::vector<std::shared_ptr<NodeLink>>& getLinks();
	std::vector<std::shared_ptr<Node>>& getSelectedNodes();
	std::vector<std::shared_ptr<NodeLink>>& getSelectedLinks();

	void evaluate();
	void evaluate(Device::Type deviceType);
	void evaluate(std::shared_ptr<Node> node);
	void evaluate(std::vector<std::shared_ptr<Node>> nodes);

	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);

	void editorGui();
	void centerView();
	void showFlow();

	int getNewUniqueID();
	void startCountingUniqueIDsFrom(int largestUniqueID);
	
	bool& getShowOutputValues();
	bool& getWasJustLoaded();

	void reset();
	
}

