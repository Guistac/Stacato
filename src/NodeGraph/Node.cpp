#include <pch.h>

#include "Node.h"

#include "NodeGraph.h"
#include "NodePin.h"

void Node::addIoData(std::shared_ptr<NodePin> NodePin) {

	//don't add the nodepin if the node already has it
	for (auto input : nodeInputData) if (input == NodePin) return;
	for (auto output : nodeOutputData) if (output == NodePin) return;

	if (NodePin->isInput()) nodeInputData.push_back(NodePin);
	else if (NodePin->isOutput()) nodeOutputData.push_back(NodePin);

	//if a pins gets added after if the node is already on the editor, this handles everything
	if (parentNodeGraph) {
		NodePin->uniqueID = parentNodeGraph->uniqueID;
		parentNodeGraph->uniqueID++;
		parentNodeGraph->pins.push_back(NodePin);
		NodePin->parentNode = shared_from_this();
	}
}

void Node::removeIoData(std::shared_ptr<NodePin> removedIoData) {
	if (removedIoData->isInput()) {
		for (int i = (int)nodeInputData.size() - 1; i >= 0; i--) {
			if (nodeInputData[i] == removedIoData) {
				nodeInputData.erase(nodeInputData.begin() + i);
				break;
			}
		}
	}
	else if (removedIoData->isOutput()) {
		for (int i = (int)nodeOutputData.size() - 1; i >= 0; i--) {
			if (nodeOutputData[i] == removedIoData) {
				nodeOutputData.erase(nodeOutputData.begin() + i);
				break;
			}
		}
	}
	removedIoData->disconnectAllLinks();
	if (parentNodeGraph) {
		std::vector<std::shared_ptr<NodePin>>& NodePinList = parentNodeGraph->pins;
		for (int i = (int)NodePinList.size() - 1; i >= 0; i--) {
			if (NodePinList[i] == removedIoData) {
				NodePinList.erase(NodePinList.begin() + i);
				break;
			}
		}
	}
}

//check if all nodes linked to the inputs of this node were processed
bool Node::areAllLinkedInputNodesProcessed() {
	for (auto inputData : nodeInputData) {
		for (auto inputDataLink : inputData->getLinks()) {
			if (!inputDataLink->getInputData()->getNode()->wasProcessed()) return false;
		}
	}
	return true;
}


static std::vector<NodeType> nodeTypes = {
	{Node::Type::IODEVICE,	"IODevice",		"IODevice"},
	{Node::Type::PROCESSOR, "Processor",	"Processor"},
	{Node::Type::CLOCK,		"Clock",		"Clock"},
	{Node::Type::MACHINE,	"Machine",		"Machine"},
	{Node::Type::CONTAINER, "Container",	"Container"}
};
NodeType* getNodeType(Node::Type t) {
	for (NodeType& nodeType : nodeTypes) {
		if (t == nodeType.type) return &nodeType;
	}
	return nullptr;
}
NodeType* getNodeType(const char* saveName) {
	for (NodeType& nodeType : nodeTypes) {
		if (strcmp(nodeType.saveName, saveName) == 0) return &nodeType;
	}
	return nullptr;
}