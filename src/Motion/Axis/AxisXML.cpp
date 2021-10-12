#include <pch.h>

#include "Motion/Axis/Axis.h"

#include <tinyxml2.h>


bool Axis::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;
	
	XMLElement* axisTypeXML = xml->InsertNewChildElement("Axis");
	axisTypeXML->SetAttribute("Type", getAxisType(axisUnitType)->saveName);
	axisTypeXML->SetAttribute("Unit", getPositionUnitType(axisPositionUnit)->saveName);

	XMLElement* feedbackXML = xml->InsertNewChildElement("Feedback");
	feedbackXML->SetAttribute("Type", getPositionFeedbackType(positionFeedbackType)->saveName);
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		feedbackXML->SetAttribute("Unit", getPositionUnitType(feedbackPositionUnit)->saveName);
		feedbackXML->SetAttribute("UnitsPerAxisUnit", feedbackUnitsPerAxisUnits);
	}

	XMLElement* commandXML = xml->InsertNewChildElement("Command");
	commandXML->SetAttribute("Type", getCommandType(commandType)->saveName);
	commandXML->SetAttribute("Unit", getPositionUnitType(commandPositionUnit)->saveName);
	commandXML->SetAttribute("UnitsPerAxisUnit", commandUnitsPerAxisUnits);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReference");
	positionReferenceXML->SetAttribute("Type", getPositionReferenceType(positionReferenceType)->saveName);
	switch (positionReferenceType) {
		case PositionReference::Type::LOW_LIMIT:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
			break;
		case PositionReference::Type::HIGH_LIMIT:
			positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
			break;
		case PositionReference::Type::LOW_AND_HIGH_LIMIT:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
			positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType)->saveName);
		case PositionReference::Type::POSITION_REFERENCE:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
			positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType)->saveName);
		case PositionReference::Type::NO_LIMIT:
			positionReferenceXML->SetAttribute("MaxPositiveDeviation", allowedPositiveDeviationFromReference_degrees);
			positionReferenceXML->SetAttribute("MaxNegativeDeviation", allowedNegativeDeviationFromReference_degrees);
	}

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_degreesPerSecond", velocityLimit_degreesPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_degreesPerSecondSquared", accelerationLimit_degreesPerSecondSquared);

	XMLElement* defaultManualParametersXML = xml->InsertNewChildElement("DefaultManualParameters");
	defaultManualParametersXML->SetAttribute("Velocity_degreesPerSecond", defaultManualVelocity_degreesPerSecond);
	defaultManualParametersXML->SetAttribute("Acceleration_degreesPerSecondSquared", defaultManualAcceleration_degreesPerSecondSquared);

	return false;
}



bool Axis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->FirstChildElement("Axis");
	if (!axisXML) return Logger::warn("Could not load Axis Attributes");
	const char* axisUnitTypeString;
	

	if (axisXML->QueryStringAttribute("Type", &axisUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Axis Type");
	if (getAxisType(axisUnitTypeString) == nullptr) return Logger::warn("Could not read Axis Type");
	axisUnitType = getAxisType(axisUnitTypeString)->unitType;
	const char* axisUnitString;
	if (axisXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Axis Unit");
	if (getPositionUnitType(axisUnitString) == nullptr) return Logger::warn("Could not read Axis Unit");
	axisPositionUnit = getPositionUnitType(axisUnitString)->unit;


	XMLElement* feedbackXML = xml->FirstChildElement("Feedback");
	if (!feedbackXML) return Logger::warn("Could not load Axis Feedback");
	const char* feedbackTypeString;
	if (feedbackXML->QueryStringAttribute("Type", &feedbackTypeString) != XML_SUCCESS) return Logger::warn("Could not load Feedback Type");
	if (getPositionFeedbackType(feedbackTypeString) == nullptr) return Logger::warn("Could not read Feedback Type");
	positionFeedbackType = getPositionFeedbackType(feedbackTypeString)->type;
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		const char* feedbackUnitString;
		if (feedbackXML->QueryStringAttribute("Unit", &feedbackUnitString) != XML_SUCCESS) return Logger::warn("Could not load Feedback Unit");
		if (getPositionUnitType(feedbackUnitString) == nullptr) return Logger::warn("Could not read Feedback Unit");
		feedbackPositionUnit = getPositionUnitType(feedbackUnitString)->unit;
		if (feedbackXML->QueryDoubleAttribute("UnitsPerAxisUnit", &feedbackUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Axis Unit");
	}


	XMLElement* commandXML = xml->FirstChildElement("Command");
	if (!commandXML) return Logger::warn("Could not load Axis Command Attribute");
	const char* commandTypeString;
	if (commandXML->QueryStringAttribute("Type", &commandTypeString) != XML_SUCCESS) return Logger::warn("Could not load Command Type");
	if (getCommandType(commandTypeString) == nullptr) return Logger::warn("Could not read Command Type");
	commandType = getCommandType(commandTypeString)->type;
	const char* commandUnitString;
	if (commandXML->QueryStringAttribute("Unit", &commandUnitString) != XML_SUCCESS) return Logger::warn("Could not load Command Unit");
	if (getPositionUnitType(commandUnitString) == nullptr) return Logger::warn("Could not read Command Unit");
	commandPositionUnit = getPositionUnitType(commandUnitString)->unit;
	if (commandXML->QueryDoubleAttribute("UnitsPerAxisUnit", &commandUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Command Units Per Axis Units");


	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Axis Position Reference");
	const char* positionReferenceTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionReferenceTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (getPositionReferenceType(positionReferenceTypeString) == nullptr) return Logger::warn("Could not read Position Reference Type");
	positionReferenceType = getPositionReferenceType(positionReferenceTypeString)->type;
	switch (positionReferenceType) {
		case PositionReference::Type::LOW_LIMIT:
			if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVelocity_degreesPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
			break;
		case PositionReference::Type::HIGH_LIMIT:
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVelocity_degreesPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
			break;
		case PositionReference::Type::LOW_AND_HIGH_LIMIT:
		case PositionReference::Type::POSITION_REFERENCE:
			if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
			if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVelocity_degreesPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
			const char* homingDirectionString;
			if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
			homingDirectionType = getHomingDirectionType(homingDirectionString)->type;
			if (homingDirectionType == HomingDirection::Type::UNKNOWN) return Logger::warn("Could not identify homing direction");
			break;
		case PositionReference::Type::NO_LIMIT:
			if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &allowedPositiveDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
			if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &allowedNegativeDeviationFromReference_degrees) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
	}


	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Axis Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_degreesPerSecond", &velocityLimit_degreesPerSecond)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_degreesPerSecondSquared", &accelerationLimit_degreesPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");

	XMLElement* defaultManualParametersXML = xml->FirstChildElement("DefaultManualParameters");
	if (!defaultManualParametersXML) return Logger::warn("Could not load default movement parameters");
	if (defaultManualParametersXML->QueryDoubleAttribute("Velocity_degreesPerSecond", &defaultManualVelocity_degreesPerSecond) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultManualParametersXML->QueryDoubleAttribute("Acceleration_degreesPerSecondSquared", &defaultManualAcceleration_degreesPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	manualControlAcceleration_degreesPerSecond = defaultManualAcceleration_degreesPerSecondSquared;
	targetVelocity = defaultManualVelocity_degreesPerSecond;

	return true;
}