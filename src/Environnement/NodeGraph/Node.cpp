#include <pch.h>

#include "Node.h"

#include "NodeGraph.h"
#include "NodePin.h"

void Node::addNodePin(std::shared_ptr<NodePin> NodePin) {

	//don't add the nodepin if the node already has it
	for (auto input : nodeInputPins) if (input == NodePin) return;
	for (auto output : nodeOutputPins) if (output == NodePin) return;

	if (NodePin->isInput()) nodeInputPins.push_back(NodePin);
	else if (NodePin->isOutput()) nodeOutputPins.push_back(NodePin);

	//if a pins gets added after if the node is already on the editor, this handles everything
	if (b_isInNodeGraph) {
		NodePin->uniqueID = Environnement::NodeGraph::getNewUniqueID();
		Environnement::NodeGraph::getPins().push_back(NodePin);
		NodePin->parentNode = shared_from_this();
	}
}

void Node::removeIoData(std::shared_ptr<NodePin> removedIoData) {
	if (removedIoData->isInput()) {
		for (int i = (int)nodeInputPins.size() - 1; i >= 0; i--) {
			if (nodeInputPins[i] == removedIoData) {
				nodeInputPins.erase(nodeInputPins.begin() + i);
				break;
			}
		}
	}
	else if (removedIoData->isOutput()) {
		for (int i = (int)nodeOutputPins.size() - 1; i >= 0; i--) {
			if (nodeOutputPins[i] == removedIoData) {
				nodeOutputPins.erase(nodeOutputPins.begin() + i);
				break;
			}
		}
	}
	removedIoData->disconnectAllLinks();
	if (b_isInNodeGraph) {
		std::vector<std::shared_ptr<NodePin>>& NodePinList = Environnement::NodeGraph::getPins();
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
	for (auto inputData : nodeInputPins) {
		for (auto inputDataLink : inputData->getLinks()) {
			auto connectedNode = inputDataLink->getInputData()->getNode();
			if (!connectedNode->wasProcessed()) return false;
		}
	}
	return true;
}

bool Node::areAllLinkedBidirectionalOutputNodesProcessed(){
	for(auto outputData : nodeOutputPins){
		if(!outputData->isBidirectional()) continue;
		for(auto outputDataLink : outputData->getLinks()){
			auto connectedNode = outputDataLink->getOutputData()->getNode();
			if(!connectedNode->wasProcessed()) return false;
		}
	}
	return true;
}
