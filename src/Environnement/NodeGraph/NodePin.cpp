#include <pch.h>

#include "NodeGraph.h"
#include "NodePin.h"
#include "NodeLink.h"
#include "Environnement/NodeGraph/Node.h"

#include <tinyxml2.h>

void NodePin::disconnectAllLinks() {
	for(int i = (int)nodeLinks.size() - 1; i >= 0; i--){
		nodeLinks[i]->disconnect();
	}
}

const char* NodePin::getValueString() {
	static char output[32];
	switch (dataType) {
		case DataType::BOOLEAN: sprintf(output, "%s", read<bool>() ? "true" : "false");
		case DataType::INTEGER: sprintf(output, "%i", read<int>()); break;
		case DataType::REAL: sprintf(output, "%.5f", read<double>()); break;
		default: return "No Value";
	}
	return (const char*)output;
}

std::vector<std::shared_ptr<NodePin>> NodePin::getConnectedPins() {
	std::vector<std::shared_ptr<NodePin>> output;
	if (isInput()) for (auto& link : nodeLinks) output.push_back(link->getInputData());
	else for (auto& link : nodeLinks) output.push_back(link->getOutputData());
	return output;
}

std::shared_ptr<NodePin> NodePin::getConnectedPin(){
	if(nodeLinks.empty()) return nullptr;
	if(isInput()) return nodeLinks.front()->getInputData();
	if(isOutput()) return nodeLinks.front()->getOutputData();
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

bool NodePin::save(tinyxml2::XMLElement* xml) {
	xml->SetAttribute("SaveString", getSaveString());
	xml->SetAttribute("DisplayString", getDisplayString());
	xml->SetAttribute("DataType", Enumerator::getSaveString(dataType));
	xml->SetAttribute("UniqueID", getUniqueID());
	xml->SetAttribute("Visible", isVisible());
	if (b_acceptsMultipleInputs) xml->SetAttribute("AcceptsMultipleInputs", true);
	if (b_disablePin) xml->SetAttribute("DisablePin", true);
	if (b_noDataField) xml->SetAttribute("NoDataField", true);
	if (b_forceDataField) xml->SetAttribute("ForceDataField", true);
	if (b_disableDataField) xml->SetAttribute("DisableDataField", true);
	return true;
}

bool NodePin::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	//here we load SaveName again, this should not be necessary since we already matched it or it was declared in the static object pin?
	const char* savestr;
	if (xml->QueryStringAttribute("SaveString", &savestr) != XML_SUCCESS) return Logger::warn("Could not load Pin SaveName");
	strcpy(saveString, savestr);
	const char* displaystr;
	if (xml->QueryStringAttribute("DisplayString", &displaystr) != XML_SUCCESS) return Logger::warn("Could not load Pin DisplayName");
	strcpy(displayString, displaystr);

	int pinUniqueID;
	if (xml->QueryIntAttribute("UniqueID", &pinUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Pin ID");
	uniqueID = pinUniqueID;

	//here we load dataType again, this should not be necessary since we already matched it or an object declared the type on construction
	const char* dataTypeString;
	if (xml->QueryStringAttribute("DataType", &dataTypeString) != XML_SUCCESS) return Logger::warn("Could not load Pin Datatype");
	if (!Enumerator::isValidSaveName<NodePin::DataType>(dataTypeString)) return Logger::warn("Could not read Pin DataType");
	dataType = Enumerator::getEnumeratorFromSaveString<NodePin::DataType>(dataTypeString);

	if (xml->QueryBoolAttribute("Visible", &b_visible) != XML_SUCCESS) return Logger::warn("Could not load pin visibility");

	//these are optionnally defined, so we don't do success checking
	xml->QueryBoolAttribute("AcceptsMultipleInputs", &b_acceptsMultipleInputs);
	xml->QueryBoolAttribute("DisablePin", &b_disablePin);
	xml->QueryBoolAttribute("NoDataField", &b_noDataField);
	xml->QueryBoolAttribute("ForceDataField", &b_forceDataField);
	xml->QueryBoolAttribute("DisableDataField", &b_disableDataField);

	return true;
}

bool NodePin::matches(const char* savestr, NodePin::DataType type) {
	return strcmp(saveString, savestr) == 0 && type == dataType;
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
		if (link->outputData == thisPin || link->inputData == otherPin) return false;
	}

	//if all checks pass, check if the data types are compatible to decide validity
	return isDataTypeCompatible(otherPin);
}

std::shared_ptr<NodeLink> NodePin::connectTo(std::shared_ptr<NodePin> otherPin){
	
	if (!isConnectionValid(otherPin)) return nullptr;
	
	std::shared_ptr<NodeLink> newLink = std::make_shared<NodeLink>();
	newLink->uniqueID = getNode()->nodeGraph->getNewUniqueID();
	newLink->nodeGraph = parentNode->nodeGraph;
	
	auto thisPin = std::static_pointer_cast<NodePin>(shared_from_this());
	
	newLink->inputData = thisPin->isOutput() ? thisPin : otherPin;
	newLink->outputData = otherPin->isInput() ? otherPin : thisPin;
	
	nodeLinks.push_back(newLink);
	otherPin->nodeLinks.push_back(newLink);
	
	newLink->inputData->parentNode->onPinConnection(newLink->inputData);
	newLink->outputData->parentNode->onPinConnection(newLink->outputData);
	
	getNode()->nodeGraph->getLinks().push_back(newLink);
	
	return newLink;
}

