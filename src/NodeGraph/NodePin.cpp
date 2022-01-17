#include <pch.h>

#include "NodeGraph.h"
#include "NodePin.h"
#include "NodeLink.h"
#include "Node.h"

#include <tinyxml2.h>

void NodePin::disconnectAllLinks() {
	if (parentNode == nullptr || parentNode->parentNodeGraph == nullptr) return;
	while (isConnected()) {
		parentNode->parentNodeGraph->disconnect(NodeLinks.front());
	}
}

const char* NodePin::getValueString() {
	static char output[32];
	switch (dataType) {
		case DataType::BOOLEAN: sprintf(output, "%s", get<bool>() ? "true" : "false");
		case DataType::INTEGER: sprintf(output, "%i", get<int>()); break;
		case DataType::REAL: sprintf(output, "%.5f", get<double>()); break;
		default: return "No Value";
	}
	return (const char*)output;
}

std::vector<std::shared_ptr<NodePin>> NodePin::getConnectedPins() {
	std::vector<std::shared_ptr<NodePin>> output;
	if (isInput()) for (auto& link : NodeLinks) output.push_back(link->getInputData());
	else for (auto& link : NodeLinks) output.push_back(link->getOutputData());
	return output;
}

std::shared_ptr<NodePin> NodePin::getConnectedPin(){
	if(NodeLinks.empty()) return nullptr;
	if(isInput()) return NodeLinks.front()->getInputData();
	if(isOutput()) return NodeLinks.front()->getOutputData();
}

std::vector<std::shared_ptr<Node>> NodePin::getNodesLinkedAtOutputs() {
	std::vector<std::shared_ptr<Node>> linkedNodes;
	for (auto link : NodeLinks) {
		linkedNodes.push_back(link->getOutputData()->getNode());
	}
	return linkedNodes;
}

std::vector<std::shared_ptr<Node>> NodePin::getNodesLinkedAtInputs() {
	std::vector<std::shared_ptr<Node>> linkedNodes;
	for (auto link : NodeLinks) {
		linkedNodes.push_back(link->getInputData()->getNode());
	}
	return linkedNodes;
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
	//TODO: differentiate between pins that were added by default and pins that were added after node creation
	using namespace tinyxml2;

	//here we load SaveName again, this should not be necessary since we already matched it or it was declared in the static object pin?
	const char* savestr;
	if (xml->QueryStringAttribute("SaveString", &savestr) != XML_SUCCESS) return Logger::warn("Could not load Pin SaveName");
	strcpy(saveString, savestr);
	const char* displaystr;
	if (xml->QueryStringAttribute("DisplayName", &displaystr) != XML_SUCCESS) return Logger::warn("Could not load Pin DisplayName");
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
