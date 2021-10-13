#include <pch.h>

#include "MotionTypes.h"




std::vector<PositionUnit> linearPositionUnits = {
	{PositionUnit::Unit::METER, PositionUnit::Type::LINEAR,			"Meter",		"Meters",		"Meter"},
	{PositionUnit::Unit::MILLIMETER, PositionUnit::Type::LINEAR,	"Millimeter",	"Millimeters",	"Millimeter"}
};

std::vector<PositionUnit> angularPositionUnits = {
	{PositionUnit::Unit::DEGREE, PositionUnit::Type::ANGULAR,		"Degree",		"Degrees",		"Degrees"},
	{PositionUnit::Unit::RADIAN, PositionUnit::Type::ANGULAR,		"Radian",		"Radians",		"Radians"},
	{PositionUnit::Unit::REVOLUTION, PositionUnit::Type::ANGULAR,	"Rotation",		"Rotations",	"Rotations"}
};

std::vector<PositionUnit>& getLinearPositionUnits() { return linearPositionUnits; }

std::vector<PositionUnit>& getAngularPositionUnits() { return angularPositionUnits; }

PositionUnit* getPositionUnit(PositionUnit::Unit u) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (unit.unit == u) return &unit;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (unit.unit == u) return &unit;
	}
	return nullptr;
}

PositionUnit* getPositionUnit(const char* saveName) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (strcmp(unit.saveName, saveName) == 0) return &unit;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (strcmp(unit.saveName, saveName) == 0) return &unit;
	}
	return nullptr;
}

std::vector<PositionUnitType> positionUnitTypes = {
	{PositionUnit::Type::LINEAR, "Linear", "Linear"},
	{PositionUnit::Type::ANGULAR, "Angular", "Angular"}
};
std::vector<PositionUnitType>& getPositionUnitTypes() { return positionUnitTypes; }
PositionUnitType* getPositionUnitType(PositionUnit::Type t) {
	for (PositionUnitType& type : positionUnitTypes) {
		if (t == type.type) return &type;
	}
	return nullptr;
}
PositionUnitType* getPositionUnitType(const char* saveName) {
	for (PositionUnitType& type : positionUnitTypes) {
		if (strcmp(saveName, type.saveName) == 0) return &type;
	}
	return nullptr;
}





std::vector<PositionFeedback> positionFeedbackTypes = {
	{PositionFeedback::Type::ABSOLUTE_FEEDBACK, "Absolute Feedback", "Absolute"},
	{PositionFeedback::Type::INCREMENTAL_FEEDBACK, "Incremental Feedback", "Incremental"},
	{PositionFeedback::Type::NO_FEEDBACK, "No Feedback", "None"}
};

std::vector<PositionFeedback>& getPositionFeedbackTypes() { return positionFeedbackTypes; }

PositionFeedback* getPositionFeedbackType(PositionFeedback::Type t) {
	for (PositionFeedback& feedback : positionFeedbackTypes) {
		if (feedback.type == t) return &feedback;
	}
	return nullptr;
}

PositionFeedback* getPositionFeedbackType(const char* saveName) {
	for (PositionFeedback& feedback : positionFeedbackTypes) {
		if (strcmp(saveName, feedback.saveName) == 0) return &feedback;
	}
	return nullptr;
}





std::vector<PositionReference> positionReferenceTypes = {
	{PositionReference::Type::LOW_LIMIT, "Low Limit", "Low"},
	{PositionReference::Type::HIGH_LIMIT, "High Limit", "High"},
	{PositionReference::Type::LOW_AND_HIGH_LIMIT, "Low and High Limit", "LowHigh"},
	{PositionReference::Type::POSITION_REFERENCE, "Position Reference", "Reference"},
	{PositionReference::Type::NO_LIMIT, "No Limit", "None"}
};

std::vector<PositionReference>& getPositionReferenceTypes() { return positionReferenceTypes; }

PositionReference* getPositionReferenceType(PositionReference::Type t) {
	for (PositionReference& reference : positionReferenceTypes) {
		if (reference.type == t) return &reference;
	}
	return nullptr;
}

PositionReference* getPositionReferenceType(const char* saveName) {
	for (PositionReference& reference : positionReferenceTypes) {
		if (strcmp(saveName, reference.saveName) == 0) return &reference;
	}
	return nullptr;
}





std::vector<CommandType> commandTypes = {
	{CommandType::Type::POSITION_COMMAND, "Position Command", "Position"},
	{CommandType::Type::VELOCITY_COMMAND, "Velocity Command", "Velocity"}
};

std::vector<CommandType>& getCommandTypes() { return commandTypes; }

CommandType* getCommandType(CommandType::Type t) {
	for (CommandType& command : commandTypes) {
		if (command.type == t) return &command;
	}
	return nullptr;
}

CommandType* getCommandType(const char* saveName) {
	for (CommandType& command : commandTypes) {
		if (strcmp(saveName, command.saveName) == 0) return &command;
	}
	return nullptr;
}






std::vector<HomingDirection> homingDirectionTypes = {
	{HomingDirection::Type::NEGATIVE, "Negative", "Negative"},
	{HomingDirection::Type::POSITIVE, "Positive", "Positive"},
	{HomingDirection::Type::DONT_CARE, "Don't Care", "DontCare"}
};

std::vector<HomingDirection>& getHomingDirectionTypes() { return homingDirectionTypes; }

HomingDirection* getHomingDirectionType(HomingDirection::Type t) {
	for (HomingDirection& direction : homingDirectionTypes) {
		if (direction.type == t) return &direction;
	}
	return nullptr;
}

HomingDirection* getHomingDirectionType(const char* saveName) {
	for (HomingDirection& direction : homingDirectionTypes) {
		if (strcmp(saveName, direction.saveName)) return &direction;
	}
	return nullptr;
}
