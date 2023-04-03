#include <pch.h>

#include "Node.h"

#include "NodeGraph.h"
#include "NodePin.h"

void Node::addNodePin(std::shared_ptr<NodePin> pin) {

	//don't add the nodepin if the node already has it
	for (auto input : nodeInputPins) if (input == pin) return;
	for (auto output : nodeOutputPins) if (output == pin) return;

	if (pin->isInput()) nodeInputPins.push_back(pin);
	else if (pin->isOutput()) nodeOutputPins.push_back(pin);

	//if a pins gets added after if the node is already on the editor, this handles everything
	if (nodeGraph != nullptr) {
		pin->uniqueID = nodeGraph->getNewUniqueID();
		nodeGraph->getPins().push_back(pin);
		pin->parentNode = std::static_pointer_cast<Node>(shared_from_this());
	}
}

void Node::removeNodePin(std::shared_ptr<NodePin> pin) {
	if (pin->isInput()) {
		for (int i = (int)nodeInputPins.size() - 1; i >= 0; i--) {
			if (nodeInputPins[i] == pin) {
				nodeInputPins.erase(nodeInputPins.begin() + i);
				break;
			}
		}
	}
	else if (pin->isOutput()) {
		for (int i = (int)nodeOutputPins.size() - 1; i >= 0; i--) {
			if (nodeOutputPins[i] == pin) {
				nodeOutputPins.erase(nodeOutputPins.begin() + i);
				break;
			}
		}
	}
	pin->disconnectAllLinks();
	
	if (nodeGraph != nullptr) {
		auto& pins = nodeGraph->getPins();
		for (int i = (int)pins.size() - 1; i >= 0; i--) {
			if (pins[i] == pin) {
				pins.erase(pins.begin() + i);
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
