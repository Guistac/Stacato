#include <pch.h>

#include "ioData.h"
#include "ioLink.h"
#include "ioNode.h"

#include <tinyxml2.h>

bool ioData::isDataTypeCompatible(std::shared_ptr<ioData> otherData) {
	switch (type) {
		case BOOLEAN_VALUE:
			switch (otherData->getType()) {
				case BOOLEAN_VALUE:
				case INTEGER_VALUE:
				case REAL_VALUE: return true;
				case ACTUATOR_DEVICELINK:
				case POSITIONFEEDBACK_DEVICELINK:
				case GPIO_DEVICELINK: return false;
			}
		case INTEGER_VALUE:
			switch (otherData->getType()) {
				case BOOLEAN_VALUE:
				case INTEGER_VALUE:
				case REAL_VALUE: return true;
				case ACTUATOR_DEVICELINK:
				case POSITIONFEEDBACK_DEVICELINK:
				case GPIO_DEVICELINK: return false;
			}
		case REAL_VALUE:
			switch (otherData->getType()) {
				case BOOLEAN_VALUE:
				case INTEGER_VALUE:
				case REAL_VALUE: return true;
				case ACTUATOR_DEVICELINK:
				case POSITIONFEEDBACK_DEVICELINK:
				case GPIO_DEVICELINK: return false;
			}
		case ACTUATOR_DEVICELINK:
			switch (otherData->getType()) {
				case BOOLEAN_VALUE:
				case INTEGER_VALUE:
				case REAL_VALUE: return false;
				case ACTUATOR_DEVICELINK: return true;
				case POSITIONFEEDBACK_DEVICELINK:
				case GPIO_DEVICELINK: return false;
			}
		case POSITIONFEEDBACK_DEVICELINK:
			switch (otherData->getType()) {
				case BOOLEAN_VALUE:
				case INTEGER_VALUE:
				case REAL_VALUE: 
				case ACTUATOR_DEVICELINK: return false;
				case POSITIONFEEDBACK_DEVICELINK: return true;
				case GPIO_DEVICELINK: return false;
			}
		case GPIO_DEVICELINK:
			switch (otherData->getType()) {
				case BOOLEAN_VALUE:
				case INTEGER_VALUE:
				case REAL_VALUE:
				case ACTUATOR_DEVICELINK:
				case POSITIONFEEDBACK_DEVICELINK: return false;
				case GPIO_DEVICELINK: return true;
			}
	}
}

//setting data (with data conversions)
void ioData::set(bool boolean) {
	switch (type) {
		case BOOLEAN_VALUE: booleanValue = boolean; break;
		case INTEGER_VALUE: integerValue = boolean; break;
		case REAL_VALUE: realValue = boolean; break;
		default: break;
	}
}

void ioData::set(long long int integer) {
	switch (type) {
		case INTEGER_VALUE: integerValue = integer; break;
		case BOOLEAN_VALUE: booleanValue = integer > 0; break;
		case REAL_VALUE: realValue = integer; break;
		default: break;
	}
}

void ioData::set(double real) {
	switch (type) {
		case REAL_VALUE: realValue = real; break;
		case BOOLEAN_VALUE: booleanValue = real > 0.0; break;
		case INTEGER_VALUE: integerValue = real; break;
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
		case BOOLEAN_VALUE: return booleanValue;
		case INTEGER_VALUE: return integerValue > 0;
		case REAL_VALUE: return realValue > 0;
		default: return false;
	}
}
long long int ioData::getInteger() {
	switch (type) {
		case INTEGER_VALUE: return integerValue;
		case BOOLEAN_VALUE: return (long long int)booleanValue;
		case REAL_VALUE: return (long long int)realValue;
		default: return 0;
	}
}
double ioData::getReal() {
	switch (type) {
		case REAL_VALUE: return realValue;
		case BOOLEAN_VALUE: return (double)booleanValue;
		case INTEGER_VALUE: return (double)integerValue;
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
	case BOOLEAN_VALUE: strcpy(output, booleanValue ? "True" : "False"); break;
	case INTEGER_VALUE: sprintf(output, "%i", integerValue); break;
	case REAL_VALUE: sprintf(output, "%.5f", realValue); break;
	case ACTUATOR_DEVICELINK: getName(); break;
	case POSITIONFEEDBACK_DEVICELINK: getName(); break;
	case GPIO_DEVICELINK: getName(); break;
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
	xml->SetAttribute("Name", getName());
	xml->SetAttribute("DataType", getTypeName());
	xml->SetAttribute("UniqueID", getUniqueID());
	switch (getType()) {
		case DataType::BOOLEAN_VALUE: xml->SetAttribute(getTypeName(), getBoolean()); break;
		case DataType::INTEGER_VALUE: xml->SetAttribute(getTypeName(), getInteger()); break;
		case DataType::REAL_VALUE: xml->SetAttribute(getTypeName(), getReal()); break;
	}
	xml->SetAttribute("Visible", isVisible());
	return true;
}

bool ioData::load(tinyxml2::XMLElement* xml) {
	//TODO: differentiate between pins that were added by default and pins that were added after node creation
	using namespace tinyxml2;
	int pinUniqueID;
	if (xml->QueryIntAttribute("UniqueID", &pinUniqueID) != XML_SUCCESS) return Logger::warn("Could not load Pin ID");
	uniqueID = pinUniqueID;
	const char* dataTypeName;
	if (xml->QueryStringAttribute("DataType", &dataTypeName) != XML_SUCCESS) return Logger::warn("Could not load Pin Datatype");
	switch (getType()) {
		case DataType::BOOLEAN_VALUE:
			bool booleanData;
			if (xml->QueryBoolAttribute(dataTypeName, &booleanData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(booleanData);
			break;
		case DataType::INTEGER_VALUE:
			long long int integerData;
			if( xml->QueryInt64Attribute(dataTypeName, &integerData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(integerData);
			break;
		case DataType::REAL_VALUE:
			double realData;
			if (xml->QueryDoubleAttribute(dataTypeName, &realData) != XML_SUCCESS) return Logger::warn("Could not load data of type {}", dataTypeName);
			set(realData);
			break;
	}
	bool visible;
	if (xml->QueryBoolAttribute("Visible", &visible) != XML_SUCCESS) return Logger::warn("Could not load pin visibility");
	b_visible = visible;
	return true;
}

bool ioData::matches(const char* name, const char* dataTypeString) {
	return strcmp(name, getName()) == 0 && strcmp(dataTypeString, getTypeName()) == 0;
}