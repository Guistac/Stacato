#include <pch.h>

#include "NodeLink.h"
#include "NodePin.h"
#include "NodeGraph.h"

void NodeLink::onConstruction(){
	Component::onConstruction();
}

void NodeLink::onCopyFrom(std::shared_ptr<PrototypeBase> source) {}

bool NodeLink::onSerialization() {
	//we don't call this since we don't need a saved name for links
	//Component::onSerialization();
	
	std::string start = getInputPin()->getNode()->getName() + " : " + getInputPin()->getName();
	std::string end = getOutputPin()->getNode()->getName() + " : " + getOutputPin()->getName();

	serializeAttribute("Start", start);
	serializeAttribute("End", start);
	
	serializeAttribute("UniqueID", getUniqueID());
	serializeAttribute("StartPinID", getInputPin()->getUniqueID());
	serializeAttribute("EndPinID", getOutputPin()->getUniqueID());
	
	return true;
}

bool NodeLink::onDeserialization() {
	Component::onDeserialization();
}

void NodeLink::disconnect(){
	std::vector<std::shared_ptr<NodeLink>>& inputDataLinks = inputPin->nodeLinks;
	std::vector<std::shared_ptr<NodeLink>>& outputDataLinks = outputPin->nodeLinks;

	auto thisLink = std::static_pointer_cast<NodeLink>(shared_from_this());
	
	for (int i = 0; i < inputDataLinks.size(); i++) {
		if (inputDataLinks[i] == thisLink) {
			inputDataLinks.erase(inputDataLinks.begin() + i);
			break;
		}
	}
	for (int i = 0; i < outputDataLinks.size(); i++) {
		if (outputDataLinks[i] == thisLink) {
			outputDataLinks.erase(outputDataLinks.begin() + i);
			break;
		}
	}
	
	nodeGraph->removeLink(thisLink);
		
	inputPin->parentNode->onPinDisconnection(inputPin);
	outputPin->parentNode->onPinDisconnection(outputPin);
	
	nodeGraph = nullptr;
	inputPin = nullptr;
	outputPin = nullptr;
	//ideally the link is referenced nowhere else and should free itself at the end of this function
};
