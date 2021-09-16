#include <pch.h>

#include "MotionTypes.h"


std::vector<AxisType> axisTypes = {
	{UnitType::ANGULAR, "Rotating Axis", "Rotating"},
	{UnitType::LINEAR, "Linear Axis", "Linear"}
};

std::vector<AxisType>& getAxisTypes() { return axisTypes; }

AxisType& getAxisType(UnitType t) {
	for (AxisType& axis : axisTypes) {
		if (axis.unitType == t) return axis;
	}
	return axisTypes.back();
}





std::vector<PositionUnit> linearPositionUnits = {
	{PositionUnit::Unit::METER, UnitType::LINEAR,		"Meter",		"Meters",		"Meter"},
	{PositionUnit::Unit::MILLIMETER, UnitType::LINEAR,	"Millimeter",	"Millimeters",	"Millimeter"}
};

std::vector<PositionUnit> angularPositionUnits = {
	{PositionUnit::Unit::DEGREE, UnitType::ANGULAR,		"Degree",		"Degrees",		"Degree"},
	{PositionUnit::Unit::RADIAN, UnitType::ANGULAR,		"Radian",		"Radians",		"Radian"},
	{PositionUnit::Unit::ROTATION, UnitType::ANGULAR,	"Rotation",		"Rotations",	"Rotation"}
};

std::vector<PositionUnit>& getLinearPositionUnits() { return linearPositionUnits; }

std::vector<PositionUnit>& getAngularPositionUnits() { return angularPositionUnits; }

PositionUnit& getPositionUnitType(PositionUnit::Unit u) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (unit.unit == u) return unit;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (unit.unit == u) return unit;
	}
	return linearPositionUnits.back();
}





std::vector<PositionFeedback> positionFeedbackTypes = {
	{PositionFeedback::Type::ABSOLUTE_FEEDBACK, "Absolute Feedback", "Absolute"},
	{PositionFeedback::Type::INCREMENTAL_FEEDBACK, "Incremental Feedback", "Incremental"},
	{PositionFeedback::Type::NO_FEEDBACK, "No Feedback", "None"}
};

std::vector<PositionFeedback>& getPositionFeedbackTypes() { return positionFeedbackTypes; }

PositionFeedback& getPositionFeedbackType(PositionFeedback::Type t) {
	for (PositionFeedback& feedback : positionFeedbackTypes) {
		if (feedback.type == t) return feedback;
	}
	return positionFeedbackTypes.back();
}





std::vector<PositionReference> positionReferenceTypes = {
	{PositionReference::Type::LOW_LIMIT, "Low Limit", "Low"},
	{PositionReference::Type::HIGH_LIMIT, "High Limit", "High"},
	{PositionReference::Type::LOW_AND_HIGH_LIMIT, "Low and High Limit", "LowHigh"},
	{PositionReference::Type::POSITION_REFERENCE, "Position Reference", "Reference"},
	{PositionReference::Type::NO_LIMIT, "No Limit", "None"}
};

std::vector<PositionReference>& getPositionReferenceTypes() { return positionReferenceTypes; }

PositionReference& getPositionReferenceType(PositionReference::Type t) {
	for (PositionReference& reference : positionReferenceTypes) {
		if (reference.type == t) return reference;
	}
	return positionReferenceTypes.back();
}





std::vector<CommandType> commandTypes = {
	{CommandType::Type::POSITION_COMMAND, "Position Command", "Position"},
	{CommandType::Type::VELOCITY_COMMAND, "Velocity Command", "Velocity"}
};

std::vector<CommandType>& getCommandTypes() { return commandTypes; }

CommandType& getCommandType(CommandType::Type t) {
	for (CommandType& command : commandTypes) {
		if (command.type == t) return command;
	}
	return commandTypes.back();
}






std::vector<HomingDirection> homingDirectionTypes = {
	{HomingDirection::Type::NEGATIVE, "Negative", "Negative"},
	{HomingDirection::Type::POSITIVE, "Positive", "Positive"},
	{HomingDirection::Type::DONT_CARE, "Don't Care", "DontCare"}
};

std::vector<HomingDirection>& getHomingDirectionTypes() { return homingDirectionTypes; }

HomingDirection& getHomingDirectionType(HomingDirection::Type t) {
	for (HomingDirection& direction : homingDirectionTypes) {
		if (direction.type == t) return direction;
	}
	return homingDirectionTypes.back();
}
