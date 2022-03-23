#include <pch.h>
#include "NodeGraph.h"

#include "Node.h"
#include "NodePin.h"
#include "NodeLink.h"

namespace Environnement::NodeGraph{

	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<std::shared_ptr<NodeLink>> links;
	std::vector<std::shared_ptr<Node>> selectedNodes;
	std::vector<std::shared_ptr<NodeLink>> selectedLinks;

	std::vector<std::shared_ptr<Node>>& getNodes() { return nodes; }
	std::vector<std::shared_ptr<NodePin>>& getPins() { return pins; }
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return links; }
	std::vector<std::shared_ptr<Node>>& getSelectedNodes(){ return selectedNodes; }
	std::vector<std::shared_ptr<NodeLink>>& getSelectedLinks(){ return selectedLinks; }

	//counter to add new nodes, pins and links
	//all items are odd numbers except for split node counterparts which are the an even number above the main node ID
	int uniqueID = 1;
	int getNewUniqueID(){
		uniqueID++;
		return uniqueID;
	}


	bool b_showOutputValues = false;
	bool& getShouldShowOutputValues(){ return b_showOutputValues; }

	bool b_justLoaded = false;
	bool& getWasJustLoaded(){ return b_justLoaded; }

	void addNode(std::shared_ptr<Node> newNode) {
		newNode->uniqueID = uniqueID;
		uniqueID++;
		nodes.push_back(newNode);
		for (std::shared_ptr<NodePin> data : newNode->nodeInputPins) {
			data->uniqueID = uniqueID;
			uniqueID++;
			data->parentNode = newNode;
			pins.push_back(data);
		}
		for (std::shared_ptr<NodePin> data : newNode->nodeOutputPins) {
			data->uniqueID = uniqueID;
			uniqueID++;
			data->parentNode = newNode;
			pins.push_back(data);
		}
		newNode->b_isInNodeGraph = true;
	}

	void removeNode(std::shared_ptr<Node> removedNode) {
		for (auto data : removedNode->nodeInputPins) {
			for (std::shared_ptr<NodeLink> link : data->nodeLinks) disconnect(link);
		}
		for (auto data : removedNode->nodeOutputPins) {
			for (std::shared_ptr<NodeLink> link : data->nodeLinks) disconnect(link);
		}
		for (int i = (int)nodes.size() - 1; i >= 0; i--) {
			if (nodes[i] == removedNode) {
				nodes.erase(nodes.begin() + i);
				break;
			}
		}
		for (int i = (int)selectedNodes.size() - 1; i >= 0; i--) {
			if (selectedNodes[i] == removedNode) {
				selectedNodes.erase(selectedNodes.begin() + i);
			}
		}
		removedNode->b_isInNodeGraph = false;
	}


	bool isConnectionValid(std::shared_ptr<NodePin> data1, std::shared_ptr<NodePin> data2) {

		//only allow connection between an input and an output
		if (data1->isInput() && data2->isInput()) return false;
		else if (data1->isOutput() && data2->isOutput()) return false;

		//don't allow multiple links on an input pin
		if (data1->isInput() && !data1->nodeLinks.empty() && !data1->acceptsMultipleInputs()) return false;
		else if (data2->isInput() && !data2->nodeLinks.empty() && !data2->acceptsMultipleInputs()) return false;

		//check if the link already exists, don't allow duplicates
		for (std::shared_ptr<NodeLink> link : data1->nodeLinks) {
			if (link->outputData == data2 || link->inputData == data2) return false;
		}

		//if all checks pass, check if the data types are compatible to decide validity
		return data1->isDataTypeCompatible(data2);
	}

	std::shared_ptr<NodeLink> connect(std::shared_ptr<NodePin> data1, std::shared_ptr<NodePin> data2) {
		if (!isConnectionValid(data1, data2)) return nullptr;
		std::shared_ptr<NodeLink> newIoLink = std::make_shared<NodeLink>();
		newIoLink->uniqueID = uniqueID;
		uniqueID++;
		newIoLink->inputData = data1->isOutput() ? data1 : data2;
		newIoLink->outputData = data2->isInput() ? data2 : data1;
		data1->nodeLinks.push_back(newIoLink);
		data2->nodeLinks.push_back(newIoLink);
		links.push_back(newIoLink);
		return newIoLink;
	}

	void disconnect(std::shared_ptr<NodeLink> removedIoLink) {
		std::vector<std::shared_ptr<NodeLink>>& inputDataLinks = removedIoLink->inputData->nodeLinks;
		std::vector<std::shared_ptr<NodeLink>>& outputDataLinks = removedIoLink->outputData->nodeLinks;

		for (int i = 0; i < inputDataLinks.size(); i++) {
			if (inputDataLinks[i] == removedIoLink) {
				inputDataLinks.erase(inputDataLinks.begin() + i);
				break;
			}
		}
		for (int i = 0; i < outputDataLinks.size(); i++) {
			if (outputDataLinks[i] == removedIoLink) {
				outputDataLinks.erase(outputDataLinks.begin() + i);
				break;
			}
		}
		for (int i = 0; i < links.size(); i++) {
			if (links[i] == removedIoLink) {
				links.erase(links.begin() + i);
				break;
			}
		}
	}

	std::shared_ptr<Node> getNode(int Id) {
		for (std::shared_ptr<Node> node : nodes) {
			if (Id == node->uniqueID) return node;
		}
		return nullptr;
	}

	std::shared_ptr<NodePin> getPin(int Id) {
		for (std::shared_ptr<NodePin> pin : pins) {
			if (Id == pin->uniqueID) return pin;
		}
		return nullptr;
	}

	std::shared_ptr<NodeLink> getLink(int Id) {
		for (std::shared_ptr<NodeLink> link : links) {
			if (Id == link->uniqueID) return link;
		}
		return nullptr;
	}

}
