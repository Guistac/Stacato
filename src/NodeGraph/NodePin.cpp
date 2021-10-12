#include <pch.h>

#include "NodePin.h"
#include "NodeLink.h"
#include "Node.h"

#include <tinyxml2.h>

bool NodePin::isDataTypeCompatible(std::shared_ptr<NodePin> otherData) {
	switch (type) {
		case NodeData::Type::BOOLEAN_VALUE:
			switch (otherData->getType()) {
				case NodeData::Type::BOOLEAN_VALUE:
				case NodeData::Type::INTEGER_VALUE:
				case NodeData::Type::REAL_VALUE: return true;
				case NodeData::Type::ACTUATOR_DEVICELINK:
				case NodeData::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodeData::Type::GPIO_DEVICELINK: return false;
			}
		case NodeData::Type::INTEGER_VALUE:
			switch (otherData->getType()) {
				case NodeData::Type::BOOLEAN_VALUE:
				case NodeData::Type::INTEGER_VALUE:
				case NodeData::Type::REAL_VALUE: return true;
				case NodeData::Type::ACTUATOR_DEVICELINK:
				case NodeData::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodeData::Type::GPIO_DEVICELINK: return false;
			}
		case NodeData::Type::REAL_VALUE:
			switch (otherData->getType()) {
				case NodeData::Type::BOOLEAN_VALUE:
				case NodeData::Type::INTEGER_VALUE:
				case NodeData::Type::REAL_VALUE: return true;
				case NodeData::Type::ACTUATOR_DEVICELINK:
				case NodeData::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodeData::Type::GPIO_DEVICELINK: return false;
			}
		case NodeData::Type::ACTUATOR_DEVICELINK:
			switch (otherData->getType()) {
				case NodeData::Type::BOOLEAN_VALUE:
				case NodeData::Type::INTEGER_VALUE:
				case NodeData::Type::REAL_VALUE: return false;
				case NodeData::Type::ACTUATOR_DEVICELINK: return true;
				case NodeData::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodeData::Type::GPIO_DEVICELINK: return false;
			}
		case NodeData::Type::POSITIONFEEDBACK_DEVICELINK:
			switch (otherData->getType()) {
				case NodeData::Type::BOOLEAN_VALUE:
				case NodeData::Type::INTEGER_VALUE:
				case NodeData::Type::REAL_VALUE: 
				case NodeData::Type::ACTUATOR_DEVICELINK: return false;
				case NodeData::Type::POSITIONFEEDBACK_DEVICELINK: return true;
				case NodeData::Type::GPIO_DEVICELINK: return false;
			}
		case NodeData::Type::GPIO_DEVICELINK:
			switch (otherData->getType()) {
				case NodeData::Type::BOOLEAN_VALUE:
				case NodeData::Type::INTEGER_VALUE:
				case NodeData::Type::REAL_VALUE:
				case NodeData::Type::ACTUATOR_DEVICELINK:
				case NodeData::Type::POSITIONFEEDBACK_DEVICELINK: return false;
				case NodeData::Type::GPIO_DEVICELINK: return true;
			}
	}
}

//setting data (with data conversions)
void NodePin::set(bool boolean) {
	switch (type) {
		case NodeData::Type::BOOLEAN_VALUE: booleanValue = boolean; break;
		case NodeData::Type::INTEGER_VALUE: integerValue = boolean; break;
		case NodeData::Type::REAL_VALUE: realValue = boolean; break;
		default: break;
	}
}

void NodePin::set(long long int integer) {
	switch (type) {
		case NodeData::Type::INTEGER_VALUE: integerValue = integer; break;
		case NodeData::Type::BOOLEAN_VALUE: booleanValue = integer > 0; break;
		case NodeData::Type::REAL_VALUE: realValue = integer; break;
		default: break;
	}
}

void NodePin::set(double real) {
	switch (type) {
		case NodeData::Type::REAL_VALUE: realValue = real; break;
		case NodeData::Type::BOOLEAN_VALUE: booleanValue = real > 0.0; break;
		case NodeData::Type::INTEGER_VALUE: integerValue = real; break;
		default: break;
	}
}

void NodePin::set(std::shared_ptr<ActuatorDevice> device) {
	if (isActuatorDeviceLink()) actuatorDevice = device;
}

void NodePin::set(std::shared_ptr<PositionFeedbackDevice> device) {
	if (isPositionFeedbackDeviceLink()) positionFeedbackDevice = device;
}

void NodePin::set(std::shared_ptr<GpioDevice> device) {
	if (isGpioDeviceLink()) gpioDevice = device;
}

//reading data (with data conversions)
bool NodePin::getBoolean() {
	switch (type) {
		case NodeData::Type::BOOLEAN_VALUE: return booleanValue;
		case NodeData::Type::INTEGER_VALUE: return integerValue > 0;
		case NodeData::Type::REAL_VALUE: return realValue > 0;
		default: return false;
	}
}
long long int NodePin::getInteger() {
	switch (type) {
		case NodeData::Type::INTEGER_VALUE: return integerValue;
		case NodeData::Type::BOOLEAN_VALUE: return (long long int)booleanValue;
		case NodeData::Type::REAL_VALUE: return (long long int)realValue;
		default: return 0;
	}
}
double NodePin::getReal() {
	switch (type) {
		case NodeData::Type::REAL_VALUE: return realValue;
		case NodeData::Type::BOOLEAN_VALUE: return (double)booleanValue;
		case NodeData::Type::INTEGER_VALUE: return (double)integerValue;
		default: return 0.0;
	}
}

std::shared_ptr<ActuatorDevice> NodePin::getActuatorDevice() {
	if (isActuatorDeviceLink()) return actuatorDevice;
	return nullptr;
}

std::shared_ptr<PositionFeedbackDevice> NodePin::getPositionFeedbackDevice() {
	if (isPositionFeedbackDeviceLink()) return positionFeedbackDevice;
	return nullptr;
}

std::shared_ptr<GpioDevice> NodePin::getGpioDevice() {
	if (isGpioDeviceLink()) return gpioDevice;
	return nullptr;
}

const char* NodePin::getValueString() {
	static char output[32];
	switch (type) {
		case NodeData::Type::BOOLEAN_VALUE: strcpy(output, booleanValue ? "True" : "False"); break;
		case NodeData::Type::INTEGER_VALUE: sprintf(output, "%i", integerValue); break;
		case NodeData::Type::REAL_VALUE: sprintf(output, "%.5f", realValue); break;
			//TODO: get values of connected devices
		case NodeData::Type::ACTUATOR_DEVICELINK: getSaveName(); break;
		case NodeData::Type::POSITIONFEEDBACK_DEVICELINK: getSaveName(); break;
		case NodeData::Type::GPIO_DEVICELINK: getSaveName(); break;
	}
	return (const char*)output;
}

std::vector<std::shared_ptr<NodePin>> NodePin::getConnectedPins() {
	std::vector<std::shared_ptr<NodePin>> output;
	if (isInput()) {
		for (auto& link : NodeLinks) {
			output.push_back(link->getInputData());
		}
	}
	else {
		for (auto& link : NodeLinks) {
			output.push_back(link->getOutputData());
		}
	}
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
	}
	xml->SetAttribute("Visible", isVisible());

	if(b_acceptsMultipleInputs) xml->SetAttribute("AcceptsMultipleInputs", true);
	if(b_disablePin) xml->SetAttribute("DisablePin", true);
	if(b_noDataField) xml->SetAttribute("NoDataField", true);
	if(b_forceDataField) xml->SetAttribute("ForceDataField", true);
	if(b_disableDataField) xml->SetAttribute("DisableDataField", true);

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
		case NodeData::BOOLEAN_VALUE:
			bool booleanData;
			if (xml->QueryBoolAttribute(dataTypeString, &booleanData) != XML_SUCCESS) return Logger::warn("Could not find data of type {}", dataTypeString);
			set(booleanData);
			break;
		case NodeData::INTEGER_VALUE:
			long long int integerData;
			if( xml->QueryInt64Attribute(dataTypeString, &integerData) != XML_SUCCESS) return Logger::warn("Could not find data of type {}", dataTypeString);
			set(integerData);
			break;
		case NodeData::REAL_VALUE:
			double realData;
			if (xml->QueryDoubleAttribute(dataTypeString, &realData) != XML_SUCCESS) return Logger::warn("Could not find data of type {}", dataTypeString);
			set(realData);
			break;
	}
	bool visible;
	if (xml->QueryBoolAttribute("Visible", &visible) != XML_SUCCESS) return Logger::warn("Could not load pin visibility");
	b_visible = visible;

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



std::vector<NodeData> NodeDataTypes = {
	{NodeData::Type::BOOLEAN_VALUE, "Boolean", "Boolean"},
	{NodeData::Type::INTEGER_VALUE, "Integer", "Integer"},
	{NodeData::Type::REAL_VALUE, "Real", "Real"},
	{NodeData::Type::ACTUATOR_DEVICELINK, "Actuator", "ActuatorDeviceLink"},
	{NodeData::Type::POSITIONFEEDBACK_DEVICELINK, "Position Feedback", "PositionFeedbackDeviceLink"},
	{NodeData::Type::GPIO_DEVICELINK, "GPIO", "GPIODeviceLink"}
};
std::vector<NodeData>& getNodeDataTypes() {
	return NodeDataTypes;
}
NodeData* getNodeDataType(NodeData::Type type) {
	for (NodeData& dataType : NodeDataTypes) {
		if (type == dataType.type) return &dataType;
	}
	return nullptr;
}
NodeData* getNodeDataType(const char* saveName) {
	for (NodeData& dataType : NodeDataTypes) {
		if (strcmp(saveName, dataType.saveName) == 0) return &dataType;
	}
	return nullptr;
}