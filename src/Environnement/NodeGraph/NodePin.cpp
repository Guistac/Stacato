#include <pch.h>

#include "NodeGraph.h"
#include "NodePin.h"
#include "NodeLink.h"
#include "Environnement/NodeGraph/Node.h"

#include <tinyxml2.h>

void NodePin::disconnectAllLinks() {
	if (parentNode == nullptr || !parentNode->b_isInNodeGraph) return;
	while (isConnected()) {
		Environnement::NodeGraph::disconnect(nodeLinks.front());
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
