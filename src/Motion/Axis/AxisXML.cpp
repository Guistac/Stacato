#include <pch.h>

#include "Motion/Axis/Axis.h"

#include <tinyxml2.h>


bool Axis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not find Kinematic Kimits Attribute");
	double vel, acc;
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_degreesPerSecond", &vel) != XML_SUCCESS) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_degreesPerSecondSquared", &acc) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");
	XMLElement* defaultMovementParametersXML = xml->FirstChildElement("DefaultMovementParameters");
	if (!defaultMovementParametersXML) return Logger::warn("Could not load default movement parameters");
	double defVel, defAcc;
	if (defaultMovementParametersXML->QueryDoubleAttribute("defaultVelocity_degreesPerSecond", &defVel) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultMovementParametersXML->QueryDoubleAttribute("defaultAcceleration_degreesPerSecondSquared", &defAcc) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReference");
	if (!positionReferenceXML) return Logger::warn("Could not load Position Reference Attributes");
	const char* referenceType;
	bool identifiedLimitType = false;
	PositionReference::Type refType;
	if (positionReferenceXML->QueryStringAttribute("ReferenceType", &referenceType) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	for (PositionReference& ref : getPositionReferenceTypes()) {
		if (strcmp(referenceType, ref.saveName) == 0) {
			refType = ref.type;
			identifiedLimitType = true;
		}
	}
	if (!identifiedLimitType) return Logger::warn("Could not identify Reference Type");
	double maxPDeviation;
	double maxNDeviation;
	double homingVel;
	HomingDirection::Type homingDir;
	bool homingDirIdentified = false;

	switch (refType) {
	case PositionReference::Type::LOW_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &maxPDeviation) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVel) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReference::Type::HIGH_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &maxNDeviation) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVel) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReference::Type::LOW_AND_HIGH_LIMIT:
	case PositionReference::Type::POSITION_REFERENCE:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &maxPDeviation) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &maxNDeviation) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_degreesPerSecond", &homingVel) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		const char* homingDirString;
		if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirString) != XML_SUCCESS) return Logger::warn("Could not load homing direction");
		for (HomingDirection& dir : getHomingDirectionTypes()) {
			if (strcmp(dir.saveName, homingDirString) == 0) {
				homingDir = dir.type;
				homingDirIdentified = true;
			}
		}
		if (!homingDirIdentified) return Logger::warn("Could not identify homing direction");
		break;
	case PositionReference::Type::NO_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("MaxPositiveDeviation", &maxPDeviation) != XML_SUCCESS) return Logger::warn("Could not load max positive deviation");
		if (positionReferenceXML->QueryDoubleAttribute("MaxNegativeDeviation", &maxNDeviation) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation");
	}

	velocityLimit_degreesPerSecond = vel;
	accelerationLimit_degreesPerSecondSquared = acc;
	defaultMovementVelocity_degreesPerSecond = defVel;
	defaultMovementAcceleration_degreesPerSecondSquared = defAcc;
	positionReferenceType = refType;
	maxPositiveDeviationFromReference_degrees = maxPDeviation;
	maxNegativeDeviationFromReference_degrees = maxNDeviation;
	//TOSO: uninitialized value homingVel and homing Dir
	//homingVelocity_degreesPerSecond = homingVel;
	//homingDirectionType = homingDir;

	return true;
}

bool Axis::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_degreesPerSecond", velocityLimit_degreesPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_degreesPerSecondSquared", accelerationLimit_degreesPerSecondSquared);
	XMLElement* defaultMovementParametersXML = xml->InsertNewChildElement("DefaultMovementParameters");
	defaultMovementParametersXML->SetAttribute("defaultVelocity_degreesPerSecond", defaultMovementVelocity_degreesPerSecond);
	defaultMovementParametersXML->SetAttribute("defaultAcceleration_degreesPerSecondSquared", defaultMovementAcceleration_degreesPerSecondSquared);
	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReference");
	positionReferenceXML->SetAttribute("ReferenceType", getPositionReferenceType(positionReferenceType).saveName);
	switch (positionReferenceType) {
	case PositionReference::Type::LOW_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		break;
	case PositionReference::Type::HIGH_LIMIT:
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		break;
	case PositionReference::Type::LOW_AND_HIGH_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType).saveName);
	case PositionReference::Type::POSITION_REFERENCE:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("HomingVelocity_degreesPerSecond", homingVelocity_degreesPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirectionType(homingDirectionType).saveName);
	case PositionReference::Type::NO_LIMIT:
		positionReferenceXML->SetAttribute("MaxPositiveDeviation", maxPositiveDeviationFromReference_degrees);
		positionReferenceXML->SetAttribute("MaxNegativeDeviation", maxNegativeDeviationFromReference_degrees);
	}

	return false;
}