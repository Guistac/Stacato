#include <pch.h>

#include "ioData.h"
#include "ioLink.h"
#include "ioNode.h"

#include <tinyxml2.h>

bool ioData::isDataTypeCompatible(std::shared_ptr<ioData> otherData) {
	switch (type) {
		case ioDataType::Type::BOOLEAN_VALUE:
			switch (otherData->getType()) {
				case ioDataType::Type::BOOLEAN_VALUE:
				case ioDataType::Type::INTEGER_VALUE:
				case ioDataType::Type::REAL_VALUE: return true;
				case ioDataType::Type::ACTUATOR_DEVICELINK:
				case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK:
				case ioDataType::Type::GPIO_DEVICELINK: return false;
			}
		case ioDataType::Type::INTEGER_VALUE:
			switch (otherData->getType()) {
				case ioDataType::Type::BOOLEAN_VALUE:
				case ioDataType::Type::INTEGER_VALUE:
				case ioDataType::Type::REAL_VALUE: return true;
				case ioDataType::Type::ACTUATOR_DEVICELINK:
				case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK:
				case ioDataType::Type::GPIO_DEVICELINK: return false;
			}
		case ioDataType::Type::REAL_VALUE:
			switch (otherData->getType()) {
				case ioDataType::Type::BOOLEAN_VALUE:
				case ioDataType::Type::INTEGER_VALUE:
				case ioDataType::Type::REAL_VALUE: return true;
				case ioDataType::Type::ACTUATOR_DEVICELINK:
				case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK:
				case ioDataType::Type::GPIO_DEVICELINK: return false;
			}
		case ioDataType::Type::ACTUATOR_DEVICELINK:
			switch (otherData->getType()) {
				case ioDataType::Type::BOOLEAN_VALUE:
				case ioDataType::Type::INTEGER_VALUE:
				case ioDataType::Type::REAL_VALUE: return false;
				case ioDataType::Type::ACTUATOR_DEVICELINK: return true;
				case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK:
				case ioDataType::Type::GPIO_DEVICELINK: return false;
			}
		case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK:
			switch (otherData->getType()) {
				case ioDataType::Type::BOOLEAN_VALUE:
				case ioDataType::Type::INTEGER_VALUE:
				case ioDataType::Type::REAL_VALUE: 
				case ioDataType::Type::ACTUATOR_DEVICELINK: return false;
				case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK: return true;
				case ioDataType::Type::GPIO_DEVICELINK: return false;
			}
		case ioDataType::Type::GPIO_DEVICELINK:
			switch (otherData->getType()) {
				case ioDataType::Type::BOOLEAN_VALUE:
				case ioDataType::Type::INTEGER_VALUE:
				case ioDataType::Type::REAL_VALUE:
				case ioDataType::Type::ACTUATOR_DEVICELINK:
				case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK: return false;
				case ioDataType::Type::GPIO_DEVICELINK: return true;
			}
	}
}

//setting data (with data conversions)
void ioData::set(bool boolean) {
	switch (type) {
		case ioDataType::Type::BOOLEAN_VALUE: booleanValue = boolean; break;
		case ioDataType::Type::INTEGER_VALUE: integerValue = boolean; break;
		case ioDataType::Type::REAL_VALUE: realValue = boolean; break;
		default: break;
	}
}

void ioData::set(long long int integer) {
	switch (type) {
		case ioDataType::Type::INTEGER_VALUE: integerValue = integer; break;
		case ioDataType::Type::BOOLEAN_VALUE: booleanValue = integer > 0; break;
		case ioDataType::Type::REAL_VALUE: realValue = integer; break;
		default: break;
	}
}

void ioData::set(double real) {
	switch (type) {
		case ioDataType::Type::REAL_VALUE: realValue = real; break;
		case ioDataType::Type::BOOLEAN_VALUE: booleanValue = real > 0.0; break;
		case ioDataType::Type::INTEGER_VALUE: integerValue = real; break;
		default: break;
	}
}

void ioData::set(std::shared_ptr<ActuatorDevice> device) {
	if (isActuatorDeviceLink()) actuatorDevice = device;
}

void ioData::set(std::shared_ptr<PositionFeedbackDevice> device) {
	if (isPositionFeedbackDeviceLink()) positionFeedbackDevice = device;
}

void ioData::set(std::shared_ptr<GpioDevice> device) {
	if (isGpioDeviceLink()) gpioDevice = device;
}

//reading data (with data conversions)
bool ioData::getBoolean() {
	switch (type) {
		case ioDataType::Type::BOOLEAN_VALUE: return booleanValue;
		case ioDataType::Type::INTEGER_VALUE: return integerValue > 0;
		case ioDataType::Type::REAL_VALUE: return realValue > 0;
		default: return false;
	}
}
long long int ioData::getInteger() {
	switch (type) {
		case ioDataType::Type::INTEGER_VALUE: return integerValue;
		case ioDataType::Type::BOOLEAN_VALUE: return (long long int)booleanValue;
		case ioDataType::Type::REAL_VALUE: return (long long int)realValue;
		default: return 0;
	}
}
double ioData::getReal() {
	switch (type) {
		case ioDataType::Type::REAL_VALUE: return realValue;
		case ioDataType::Type::BOOLEAN_VALUE: return (double)booleanValue;
		case ioDataType::Type::INTEGER_VALUE: return (double)integerValue;
		default: return 0.0;
	}
}

std::shared_ptr<ActuatorDevice> ioData::getActuatorDevice() {
	if (isActuatorDeviceLink()) return actuatorDevice;
	return nullptr;
}

std::shared_ptr<PositionFeedbackDevice> ioData::getPositionFeedbackDevice() {
	if (isPositionFeedbackDeviceLink()) return positionFeedbackDevice;
	return nullptr;
}

std::shared_ptr<GpioDevice> ioData::getGpioDevice() {
	if (isGpioDeviceLink()) return gpioDevice;
	return nullptr;
}

const char* ioData::getValueString() {
	static char output[32];
	switch (type) {
		case ioDataType::Type::BOOLEAN_VALUE: strcpy(output, booleanValue ? "True" : "False"); break;
		case ioDataType::Type::INTEGER_VALUE: sprintf(output, "%i", integerValue); break;
		case ioDataType::Type::REAL_VALUE: sprintf(output, "%.5f", realValue); break;
			//TODO: get values of connected devices
		case ioDataType::Type::ACTUATOR_DEVICELINK: getSaveName(); break;
		case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK: getSaveName(); break;
		case ioDataType::Type::GPIO_DEVICELINK: getSaveName(); break;
	}
	return (const char*)output;
}

std::vector<std::shared_ptr<ioNode>> ioData::getNodesLinkedAtOutputs() {
	std::vector<std::shared_ptr<ioNode>> linkedNodes;
	for (auto link : ioLinks) {
		linkedNodes.push_back(link->getOutputData()->getNode());
	}
	return linkedNodes;
}

std::vector<std::shared_ptr<ioNode>> ioData::getNodesLinkedAtInputs() {
	std::vector<std::shared_ptr<ioNode>> linkedNodes;
	for (auto link : ioLinks) {
		linkedNodes.push_back(link->getInputData()->getNode());
	}
	return linkedNodes;
}

bool ioData::save(tinyxml2::XMLElement* xml) {
	xml->SetAttribute("SaveName", getSaveName());
	xml->SetAttribute("DisplayName", getDisplayName());
	xml->SetAttribute("DataType", getTypeName());
	xml->SetAttribute("UniqueID", getUniqueID());
	switch (getType()) {
		case ioDataType::BOOLEAN_VALUE: xml->SetAttribute(getTypeName(), getBoolean()); break;
		case ioDataType::INTEGER_VALUE: xml->SetAttribute(getTypeName(), getInteger()); break;
		case ioDataType::REAL_VALUE: xml->SetAttribute(getTypeName(), getReal()); break;
	}
	xml->SetAttribute("Visible", isVisible());

	if(b_acceptsMultipleInputs) xml->SetAttribute("AcceptsMultipleInputs", true);
	if(b_disablePin) xml->SetAttribute("DisablePin", true);
	if(b_noDataField) xml->SetAttribute("NoDataField", true);
	if(b_forceDataField) xml->SetAttribute("ForceDataField", true);
	if(b_disableDataField) xml->SetAttribute("DisableDataField", true);

	return true;
}

bool ioData::load(tinyxml2::XMLElement* xml) {
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
		case ioDataType::BOOLEAN_VALUE:
			bool booleanData;
			if (xml->QueryBoolAttribute(dataTypeName, &booleanData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(booleanData);
			break;
		case ioDataType::INTEGER_VALUE:
			long long int integerData;
			if( xml->QueryInt64Attribute(dataTypeName, &integerData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(integerData);
			break;
		case ioDataType::REAL_VALUE:
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

bool ioData::matches(const char* saveNameString, const char* dataTypeString) {
	return strcmp(saveName, saveNameString) == 0 && strcmp(dataTypeString, getTypeName()) == 0;
}



std::vector<ioDataType> ioDataTypes = {
	{ioDataType::Type::BOOLEAN_VALUE, "Boolean", "Boolean"},
	{ioDataType::Type::INTEGER_VALUE, "Integer", "Integer"},
	{ioDataType::Type::REAL_VALUE, "Real", "Real"},
	{ioDataType::Type::ACTUATOR_DEVICELINK, "Actuator", "ActuatorDeviceLink"},
	{ioDataType::Type::POSITIONFEEDBACK_DEVICELINK, "Position Feedback", "PositionFeedbackDeviceLink"},
	{ioDataType::Type::GPIO_DEVICELINK, "GPIO", "GPIODeviceLink"}
};
ioDataType* getDataType(ioDataType::Type type) {
	for (ioDataType& dataType : ioDataTypes) {
		if (type == dataType.type) return &dataType;
	}
	return nullptr;
}
ioDataType* getDataType(const char* saveName) {
	for (ioDataType& dataType : ioDataTypes) {
		if (strcmp(saveName, dataType.saveName)) return &dataType;
	}
	return nullptr;
}