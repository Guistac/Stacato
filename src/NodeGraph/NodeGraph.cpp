#include <pch.h>
#include "NodeGraph.h"

#include "Node.h"
#include "NodePin.h"
#include "NodeLink.h"

void NodeGraph::addNode(std::shared_ptr<Node> newNode) {
	newNode->uniqueID = uniqueID;
	uniqueID++;
	nodes.push_back(newNode);
	newNode->assignIoData(); //this tries to generate an ID and adds all data to the nodelist if a parent was specified, we don't want this so we add the parent afterwards
	for (std::shared_ptr<NodePin> data : newNode->nodeInputData) {
		data->uniqueID = uniqueID;
		uniqueID++;
		data->parentNode = newNode;
		pins.push_back(data);
	}
	for (std::shared_ptr<NodePin> data : newNode->nodeOutputData) {
		data->uniqueID = uniqueID;
		uniqueID++;
		data->parentNode = newNode;
		pins.push_back(data);
	}
	newNode->parentNodeGraph = this;
	newNode->b_isInNodeGraph = true;
}

void NodeGraph::removeNode(std::shared_ptr<Node> removedNode) {
	for (auto data : removedNode->nodeInputData) {
		for (std::shared_ptr<NodeLink> link : data->NodeLinks) disconnect(link);
	}
	for (auto data : removedNode->nodeOutputData) {
		for (std::shared_ptr<NodeLink> link : data->NodeLinks) disconnect(link);
	}
	for (int i = (int)nodes.size() - 1; i >= 0; i--) {
		if (nodes[i] == removedNode) {
			nodes.erase(nodes.begin() + i);
			break;
		}
	}
	removedNode->b_isInNodeGraph = false;
}


bool NodeGraph::isConnectionValid(std::shared_ptr<NodePin> data1, std::shared_ptr<NodePin> data2) {

	//only allow connection between an input and an output
	if (data1->isInput() && data2->isInput()) return false;
	else if (data1->isOutput() && data2->isOutput()) return false;

	//don't allow multiple links on an input pin
	if (data1->isInput() && !data1->NodeLinks.empty() && !data1->acceptsMultipleInputs()) return false;
	else if (data2->isInput() && !data2->NodeLinks.empty() && !data2->acceptsMultipleInputs()) return false;

	//check if the link already exists, don't allow duplicates
	for (std::shared_ptr<NodeLink> link : data1->NodeLinks) {
		if (link->outputData == data2 || link->inputData == data2) return false;
	}

	//don't allow connecting two NodePin pins on the same node
	if (data1->parentNode == data2->parentNode) return false;

	//if all checks pass, check if the data types are compatible to decide validity
	return data1->isDataTypeCompatible(data2);
}

std::shared_ptr<NodeLink> NodeGraph::connect(std::shared_ptr<NodePin> data1, std::shared_ptr<NodePin> data2) {
	if (!isConnectionValid(data1, data2)) return nullptr;
	std::shared_ptr<NodeLink> newIoLink = std::make_shared<NodeLink>();
	newIoLink->uniqueID = uniqueID;
	uniqueID++;
	newIoLink->inputData = data1->isOutput() ? data1 : data2;
	newIoLink->outputData = data2->isInput() ? data2 : data1;
	data1->NodeLinks.push_back(newIoLink);
	data2->NodeLinks.push_back(newIoLink);
	links.push_back(newIoLink);
	return newIoLink;
}

void NodeGraph::disconnect(std::shared_ptr<NodeLink> removedIoLink) {
	std::vector<std::shared_ptr<NodeLink>>& inputDataLinks = removedIoLink->inputData->NodeLinks;
	std::vector<std::shared_ptr<NodeLink>>& outputDataLinks = removedIoLink->outputData->NodeLinks;

	std::shared_ptr<Node> updatedNode = removedIoLink->getOutputData()->getNode();

	for (int i = (int)inputDataLinks.size() - 1; i >= 0; i--) {
		if (inputDataLinks[i] == removedIoLink) {
			inputDataLinks.erase(inputDataLinks.begin() + i);
			break;
		}
	}
	for (int i = (int)outputDataLinks.size() - 1; i >= 0; i--) {
		if (outputDataLinks[i] == removedIoLink) {
			outputDataLinks.erase(outputDataLinks.begin() + i);
			break;
		}
	}
	for (int i = (int)links.size() - 1; i >= 0; i--) {
		if (links[i] == removedIoLink) {
			links.erase(links.begin() + i);
			break;
		}
	}

	//TODO: gui should not evaluate node graph ?
	//evaluate(updatedNode);
}

std::shared_ptr<Node> NodeGraph::getNode(int Id) {
	for (std::shared_ptr<Node> node : nodes) {
		if (Id == node->uniqueID) return node;
	}
	return nullptr;
}

std::shared_ptr<NodePin> NodeGraph::getPin(int Id) {
	for (std::shared_ptr<NodePin> pin : pins) {
		if (Id == pin->uniqueID) return pin;
	}
	return nullptr;
}

std::shared_ptr<NodeLink> NodeGraph::getLink(int Id) {
	for (std::shared_ptr<NodeLink> link : links) {
		if (Id == link->uniqueID) return link;
	}
	return nullptr;
}