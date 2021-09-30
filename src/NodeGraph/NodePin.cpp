#include <pch.h>

#include "NodePin.h"
#include "NodeLink.h"
#include "Node.h"

#include <tinyxml2.h>

bool NodePin::isDataTypeCompatible(std::shared_ptr<NodePin> otherData) {
	switch (type) {
		case NodePinType::Type::BOOLEAN_VALUE:
			switch (otherData->getType()) {
				case NodePinType::Type::BOOLEAN_VALUE:
				case NodePinType::Type::INTEGER_VALUE:
				case NodePinType::Type::REAL_VALUE: return true;
				case NodePinType::Type::ACTUATOR_DEVICELINK:
				case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodePinType::Type::GPIO_DEVICELINK: return false;
			}
		case NodePinType::Type::INTEGER_VALUE:
			switch (otherData->getType()) {
				case NodePinType::Type::BOOLEAN_VALUE:
				case NodePinType::Type::INTEGER_VALUE:
				case NodePinType::Type::REAL_VALUE: return true;
				case NodePinType::Type::ACTUATOR_DEVICELINK:
				case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodePinType::Type::GPIO_DEVICELINK: return false;
			}
		case NodePinType::Type::REAL_VALUE:
			switch (otherData->getType()) {
				case NodePinType::Type::BOOLEAN_VALUE:
				case NodePinType::Type::INTEGER_VALUE:
				case NodePinType::Type::REAL_VALUE: return true;
				case NodePinType::Type::ACTUATOR_DEVICELINK:
				case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodePinType::Type::GPIO_DEVICELINK: return false;
			}
		case NodePinType::Type::ACTUATOR_DEVICELINK:
			switch (otherData->getType()) {
				case NodePinType::Type::BOOLEAN_VALUE:
				case NodePinType::Type::INTEGER_VALUE:
				case NodePinType::Type::REAL_VALUE: return false;
				case NodePinType::Type::ACTUATOR_DEVICELINK: return true;
				case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK:
				case NodePinType::Type::GPIO_DEVICELINK: return false;
			}
		case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK:
			switch (otherData->getType()) {
				case NodePinType::Type::BOOLEAN_VALUE:
				case NodePinType::Type::INTEGER_VALUE:
				case NodePinType::Type::REAL_VALUE: 
				case NodePinType::Type::ACTUATOR_DEVICELINK: return false;
				case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK: return true;
				case NodePinType::Type::GPIO_DEVICELINK: return false;
			}
		case NodePinType::Type::GPIO_DEVICELINK:
			switch (otherData->getType()) {
				case NodePinType::Type::BOOLEAN_VALUE:
				case NodePinType::Type::INTEGER_VALUE:
				case NodePinType::Type::REAL_VALUE:
				case NodePinType::Type::ACTUATOR_DEVICELINK:
				case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK: return false;
				case NodePinType::Type::GPIO_DEVICELINK: return true;
			}
	}
}

//setting data (with data conversions)
void NodePin::set(bool boolean) {
	switch (type) {
		case NodePinType::Type::BOOLEAN_VALUE: booleanValue = boolean; break;
		case NodePinType::Type::INTEGER_VALUE: integerValue = boolean; break;
		case NodePinType::Type::REAL_VALUE: realValue = boolean; break;
		default: break;
	}
}

void NodePin::set(long long int integer) {
	switch (type) {
		case NodePinType::Type::INTEGER_VALUE: integerValue = integer; break;
		case NodePinType::Type::BOOLEAN_VALUE: booleanValue = integer > 0; break;
		case NodePinType::Type::REAL_VALUE: realValue = integer; break;
		default: break;
	}
}

void NodePin::set(double real) {
	switch (type) {
		case NodePinType::Type::REAL_VALUE: realValue = real; break;
		case NodePinType::Type::BOOLEAN_VALUE: booleanValue = real > 0.0; break;
		case NodePinType::Type::INTEGER_VALUE: integerValue = real; break;
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
		case NodePinType::Type::BOOLEAN_VALUE: return booleanValue;
		case NodePinType::Type::INTEGER_VALUE: return integerValue > 0;
		case NodePinType::Type::REAL_VALUE: return realValue > 0;
		default: return false;
	}
}
long long int NodePin::getInteger() {
	switch (type) {
		case NodePinType::Type::INTEGER_VALUE: return integerValue;
		case NodePinType::Type::BOOLEAN_VALUE: return (long long int)booleanValue;
		case NodePinType::Type::REAL_VALUE: return (long long int)realValue;
		default: return 0;
	}
}
double NodePin::getReal() {
	switch (type) {
		case NodePinType::Type::REAL_VALUE: return realValue;
		case NodePinType::Type::BOOLEAN_VALUE: return (double)booleanValue;
		case NodePinType::Type::INTEGER_VALUE: return (double)integerValue;
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
		case NodePinType::Type::BOOLEAN_VALUE: strcpy(output, booleanValue ? "True" : "False"); break;
		case NodePinType::Type::INTEGER_VALUE: sprintf(output, "%i", integerValue); break;
		case NodePinType::Type::REAL_VALUE: sprintf(output, "%.5f", realValue); break;
			//TODO: get values of connected devices
		case NodePinType::Type::ACTUATOR_DEVICELINK: getSaveName(); break;
		case NodePinType::Type::POSITIONFEEDBACK_DEVICELINK: getSaveName(); break;
		case NodePinType::Type::GPIO_DEVICELINK: getSaveName(); break;
	}
	return (const char*)output;
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
	xml->SetAttribute("DataType", getTypeName());
	xml->SetAttribute("UniqueID", getUniqueID());
	switch (getType()) {
		case NodePinType::BOOLEAN_VALUE: xml->SetAttribute(getTypeName(), getBoolean()); break;
		case NodePinType::INTEGER_VALUE: xml->SetAttribute(getTypeName(), getInteger()); break;
		case NodePinType::REAL_VALUE: xml->SetAttribute(getTypeName(), getReal()); break;
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

	const char* saveNameString;
	if (xml->QueryStringAttribute("SaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load Pin SaveName");
	strcpy(saveName, saveNameString);
	const char* displayNameString;
	if (xml->QueryStringAttribute("DisplayName", &displayNameString) != XML_SUCCESS) return Logger::warn("Could not load Pin DisplayName");
	strcpy(displayName, displayNameString);

	int pinUniqueID;
	if (xml->QueryIntAttribute("UniqueID", &pinUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Pin ID");
	uniqueID = pinUniqueID;
	const char* dataTypeName;
	if (xml->QueryStringAttribute("DataType", &dataTypeName) != XML_SUCCESS) return Logger::warn("Could not load Pin Datatype");
	switch (getType()) {
		case NodePinType::BOOLEAN_VALUE:
			bool booleanData;
			if (xml->QueryBoolAttribute(dataTypeName, &booleanData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(booleanData);
			break;
		case NodePinType::INTEGER_VALUE:
			long long int integerData;
			if( xml->QueryInt64Attribute(dataTypeName, &integerData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(integerData);
			break;
		case NodePinType::REAL_VALUE:
			double realData;
			if (xml->QueryDoubleAttribute(dataTypeName, &realData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
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

bool NodePin::matches(const char* saveNameString, const char* dataTypeString) {
	return strcmp(saveName, saveNameString) == 0 && strcmp(dataTypeString, getTypeName()) == 0;
}



std::vector<NodePinType> NodePinTypes = {
	{NodePinType::Type::BOOLEAN_VALUE, "Boolean", "Boolean"},
	{NodePinType::Type::INTEGER_VALUE, "Integer", "Integer"},
	{NodePinType::Type::REAL_VALUE, "Real", "Real"},
	{NodePinType::Type::ACTUATOR_DEVICELINK, "Actuator", "ActuatorDeviceLink"},
	{NodePinType::Type::POSITIONFEEDBACK_DEVICELINK, "Position Feedback", "PositionFeedbackDeviceLink"},
	{NodePinType::Type::GPIO_DEVICELINK, "GPIO", "GPIODeviceLink"}
};
NodePinType* getDataType(NodePinType::Type type) {
	for (NodePinType& dataType : NodePinTypes) {
		if (type == dataType.type) return &dataType;
	}
	return nullptr;
}
NodePinType* getDataType(const char* saveName) {
	for (NodePinType& dataType : NodePinTypes) {
		if (strcmp(saveName, dataType.saveName)) return &dataType;
	}
	return nullptr;
}