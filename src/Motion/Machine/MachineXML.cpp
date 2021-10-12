#include <pch.h>

#include "Motion/Machine/Machine.h"

#include <tinyxml2.h>


bool Machine::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;
	
	XMLElement* machineTypeXML = xml->InsertNewChildElement("Machine");
	machineTypeXML->SetAttribute("Type", getMachineType(machineUnitType)->saveName);
	machineTypeXML->SetAttribute("Unit", getPositionUnitType(machinePositionUnit)->saveName);

	XMLElement* feedbackXML = xml->InsertNewChildElement("Feedback");
	feedbackXML->SetAttribute("Type", getPositionFeedbackType(positionFeedbackType)->saveName);
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		feedbackXML->SetAttribute("Unit", getPositionUnitType(feedbackPositionUnit)->saveName);
		feedbackXML->SetAttribute("UnitsPerMachineUnit", feedbackUnitsPerMachineUnits);
	}

	XMLElement* commandXML = xml->InsertNewChildElement("Command");
	commandXML->SetAttribute("Type", getCommandType(commandType)->saveName);
	commandXML->SetAttribute("Unit", getPositionUnitType(commandPositionUnit)->saveName);
	commandXML->SetAttribute("UnitsPerMachineUnit", commandUnitsPerMachineUnits);

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



bool Machine::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* machineXML = xml->FirstChildElement("Machine");
	if (!machineXML) return Logger::warn("Could not load Machine Attributes");
	const char* machineUnitTypeString;
	

	if (machineXML->QueryStringAttribute("Type", &machineUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Type");
	if (getMachineType(machineUnitTypeString) == nullptr) return Logger::warn("Could not read Machine Type");
	machineUnitType = getMachineType(machineUnitTypeString)->unitType;
	const char* machineUnitString;
	if (machineXML->QueryStringAttribute("Unit", &machineUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (getPositionUnitType(machineUnitString) == nullptr) return Logger::warn("Could not read Machine Unit");
	machinePositionUnit = getPositionUnitType(machineUnitString)->unit;


	XMLElement* feedbackXML = xml->FirstChildElement("Feedback");
	if (!feedbackXML) return Logger::warn("Could not load Machine Feedback");
	const char* feedbackTypeString;
	if (feedbackXML->QueryStringAttribute("Type", &feedbackTypeString) != XML_SUCCESS) return Logger::warn("Could not load Feedback Type");
	if (getPositionFeedbackType(feedbackTypeString) == nullptr) return Logger::warn("Could not read Feedback Type");
	positionFeedbackType = getPositionFeedbackType(feedbackTypeString)->type;
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		const char* feedbackUnitString;
		if (feedbackXML->QueryStringAttribute("Unit", &feedbackUnitString) != XML_SUCCESS) return Logger::warn("Could not load Feedback Unit");
		if (getPositionUnitType(feedbackUnitString) == nullptr) return Logger::warn("Could not read Feedback Unit");
		feedbackPositionUnit = getPositionUnitType(feedbackUnitString)->unit;
		if (feedbackXML->QueryDoubleAttribute("UnitsPerMachineUnit", &feedbackUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	}


	XMLElement* commandXML = xml->FirstChildElement("Command");
	if (!commandXML) return Logger::warn("Could not load Machine Command Attribute");
	const char* commandTypeString;
	if (commandXML->QueryStringAttribute("Type", &commandTypeString) != XML_SUCCESS) return Logger::warn("Could not load Command Type");
	if (getCommandType(commandTypeString) == nullptr) return Logger::warn("Could not read Command Type");
	commandType = getCommandType(commandTypeString)->type;
	const char* commandUnitString;
	if (commandXML->QueryStringAttribute("Unit", &commandUnitString) != XML_SUCCESS) return Logger::warn("Could not load Command Unit");
	if (getPositionUnitType(commandUnitString) == nullptr) return Logger::warn("Could not read Command Unit");
	commandPositionUnit = getPositionUnitType(commandUnitString)->unit;
	if (commandXML->QueryDoubleAttribute("UnitsPerMachineUnit", &commandUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Command Units Per Machine Units");


	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
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
	if (!kinematicLimitsXML) return Logger::warn("Could not load Machine Kinematic Kimits");
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