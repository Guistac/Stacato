#include <pch.h>

#include "NodeGraph.h"
#include "NodePin.h"
#include "NodeLink.h"
#include "Environnement/NodeGraph/Node.h"

#include <tinyxml2.h>


bool NodePin::onSerialization() {
	Component::onSerialization();
	
	serializeAttribute("UniqueID", getUniqueID());
	serializeAttribute("SaveString", getSaveString());
	serializeAttribute("DataType", Enumerator::getSaveString(dataType));
	serializeAttribute("Visible", isVisible());
	
	serializeAttribute("AcceptsMultipleConnections", b_acceptsMultipleInputs);
	serializeAttribute("Disabled", b_disablePin);
	serializeAttribute("NoDataField", b_noDataField);
	serializeAttribute("ForceDataField", b_forceDataField);
	serializeAttribute("DisableDataField", b_disableDataField);
	
	return true;
}

bool NodePin::onDeserialization() {
	Component::onDeserialization();
	
	deserializeAttribute("UniqueID", uniqueID);
	deserializeAttribute("SaveString", saveString);
	
	std::string dataTypeString;
	deserializeAttribute("DataType", dataTypeString);
	if(Enumerator::isValidSaveName<NodePin::DataType>(dataTypeString.c_str())){
		dataType = Enumerator::getEnumeratorFromSaveString<NodePin::DataType>(dataTypeString.c_str());
	}
	
	deserializeAttribute("Visible", b_visible);
	
	deserializeAttribute("AcceptsMultipleConnections", b_acceptsMultipleInputs);
	deserializeAttribute("Disabled", b_disablePin);
	deserializeAttribute("NoDataField", b_noDataField);
	deserializeAttribute("ForceDataField", b_forceDataField);
	deserializeAttribute("DisableDataField", b_disableDataField);
	
	return true;
}

void NodePin::onConstruction() {
	Component::onConstruction();
}

void NodePin::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	Component::onCopyFrom(source);
}


void NodePin::disconnectAllLinks() {
	for(int i = (int)nodeLinks.size() - 1; i >= 0; i--){
		nodeLinks[i]->disconnect();
	}
}

const char* NodePin::getValueString() {
	static char output[32];
	switch (dataType) {
		case DataType::BOOLEAN: snprintf(output, 32, "%s", read<bool>() ? "true" : "false");
		case DataType::INTEGER: snprintf(output, 32, "%i", read<int>()); break;
		case DataType::REAL: snprintf(output, 32, "%.5f", read<double>()); break;
		default: return "No Value";
	}
	return (const char*)output;
}

std::vector<std::shared_ptr<NodePin>> NodePin::getConnectedPins() {
	std::vector<std::shared_ptr<NodePin>> output;
	if (isInput()){
		for (auto& link : nodeLinks) {
			output.push_back(link->getInputPin());
		}
	}
	else {
		for (auto& link : nodeLinks) {
			output.push_back(link->getOutputPin());
		}
	}
	return output;
}

std::shared_ptr<NodePin> NodePin::getConnectedPin(){
	if(nodeLinks.empty()) return nullptr;
	if(isInput()) return nodeLinks.front()->getInputPin();
	if(isOutput()) return nodeLinks.front()->getOutputPin();
}

void NodePin::updateConnectedPins(){
	for(auto& pin : getConnectedPins()){
		pin->getNode()->updatePin(pin);
	}
}

bool NodePin::isDataTypeCompatible(std::shared_ptr<NodePin> other){
	switch(dataType){
		case DataType::BOOLEAN:
		case DataType::INTEGER:
		case DataType::REAL:
			switch(other->dataType){
				case DataType::BOOLEAN:
				case DataType::INTEGER:
				case DataType::REAL:
					return true;
				default:
					return false;
			}
		default:
			return other->dataType == dataType;
	}
}

bool NodePin::matches(std::string& saveStr, NodePin::DataType type) {
	return saveStr == saveString && type == dataType;
}


bool NodePin::isConnectionValid(std::shared_ptr<NodePin> otherPin){
	
	auto thisPin = std::static_pointer_cast<NodePin>(shared_from_this());
	
	//only allow connection between an input and an output
	if (thisPin->isInput() && otherPin->isInput()) return false;
	else if (thisPin->isOutput() && otherPin->isOutput()) return false;

	//don't allow multiple links on an input pin
	if (thisPin->isInput() && !thisPin->nodeLinks.empty() && !thisPin->acceptsMultipleInputs()) return false;
	else if (otherPin->isInput() && !otherPin->nodeLinks.empty() && !otherPin->acceptsMultipleInputs()) return false;

	//check if the link already exists, don't allow duplicates
	for (std::shared_ptr<NodeLink> link : nodeLinks) {
		if (link->outputPin == thisPin || link->inputPin == otherPin) return false;
	}

	//if all checks pass, check if the data types are compatible to decide validity
	return isDataTypeCompatible(otherPin);
}

std::shared_ptr<NodeLink> NodePin::connectTo(std::shared_ptr<NodePin> otherPin, int UID){
	
	if (!isConnectionValid(otherPin)) return nullptr;
	
	std::shared_ptr<NodeLink> newLink = NodeLink::createInstance();
	
	if(UID == -1) newLink->uniqueID = getNode()->nodeGraph->getNewUniqueID();
	else newLink->uniqueID = UID;
	
	newLink->nodeGraph = parentNode->nodeGraph;
	
	auto thisPin = std::static_pointer_cast<NodePin>(shared_from_this());
	
	newLink->inputPin = thisPin->isOutput() ? thisPin : otherPin;
	newLink->outputPin = otherPin->isInput() ? otherPin : thisPin;
	newLink->inputPinID = newLink->inputPin->getUniqueID();
	newLink->outputPinID = newLink->outputPin->getUniqueID();
	
	nodeLinks.push_back(newLink);
	otherPin->nodeLinks.push_back(newLink);
	
	newLink->inputPin->parentNode->onPinConnection(newLink->inputPin);
	newLink->outputPin->parentNode->onPinConnection(newLink->outputPin);
	
	getNode()->nodeGraph->addLink(newLink);
	
	return newLink;
}

