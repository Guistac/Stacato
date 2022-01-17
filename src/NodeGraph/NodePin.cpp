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
	switch (type) {
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
	xml->SetAttribute("SaveName", getSaveName());
	xml->SetAttribute("DisplayName", getDisplayName());
	xml->SetAttribute("DataType", getNodeDataType(getType())->saveName);
	xml->SetAttribute("UniqueID", getUniqueID());
	switch (getType()) {
		case NodeData::BOOLEAN_VALUE: xml->SetAttribute(getNodeDataType(getType())->saveName, getBoolean()); break;
		case NodeData::INTEGER_VALUE: xml->SetAttribute(getNodeDataType(getType())->saveName, getInteger()); break;
		case NodeData::REAL_VALUE: xml->SetAttribute(getNodeDataType(getType())->saveName, getReal()); break;
		default: break;
	}
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
	const char* saveNameString;
	if (xml->QueryStringAttribute("SaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load Pin SaveName");
	strcpy(saveName, saveNameString);
	const char* displayNameString;
	if (xml->QueryStringAttribute("DisplayName", &displayNameString) != XML_SUCCESS) return Logger::warn("Could not load Pin DisplayName");
	strcpy(displayName, displayNameString);

	int pinUniqueID;
	if (xml->QueryIntAttribute("UniqueID", &pinUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Pin ID");
	uniqueID = pinUniqueID;

	//here we load dataType again, this should not be necessary since we already matched it or an object declared the type on construction
	const char* dataTypeString;
	if (xml->QueryStringAttribute("DataType", &dataTypeString) != XML_SUCCESS) return Logger::warn("Could not load Pin Datatype");
	if (getNodeDataType(dataTypeString) == nullptr) return Logger::warn("Could not read Pin DataType");
	type = getNodeDataType(dataTypeString)->type;

	switch (type) {
		case NodeData::BOOLEAN_VALUE: {
			bool booleanData;
			if (xml->QueryBoolAttribute(dataTypeString, &booleanData) != XML_SUCCESS) return Logger::warn("Could not find data of type {}", dataTypeString);
			set(booleanData);
		}break;
		case NodeData::INTEGER_VALUE: {
			long long int integerData;
			if (xml->QueryInt64Attribute(dataTypeString, &integerData) != XML_SUCCESS) return Logger::warn("Could not find data of type {}", dataTypeString);
			set(integerData);
			}break;
		case NodeData::REAL_VALUE: {
			double realData;
			if (xml->QueryDoubleAttribute(dataTypeString, &realData) != XML_SUCCESS) return Logger::warn("Could not find data of type {}", dataTypeString);
			set(realData);
			}break;
		default: break;
	}
	if (xml->QueryBoolAttribute("Visible", &b_visible) != XML_SUCCESS) return Logger::warn("Could not load pin visibility");

	//these are optionnally defined, so we don't do success checking
	xml->QueryBoolAttribute("AcceptsMultipleInputs", &b_acceptsMultipleInputs);
	xml->QueryBoolAttribute("DisablePin", &b_disablePin);
	xml->QueryBoolAttribute("NoDataField", &b_noDataField);
	xml->QueryBoolAttribute("ForceDataField", &b_forceDataField);
	xml->QueryBoolAttribute("DisableDataField", &b_disableDataField);

	return true;
}

bool NodePin::matches(const char* saveNameString, NodeData::Type type) {
	return strcmp(saveName, saveNameString) == 0 && type == getType();
}
