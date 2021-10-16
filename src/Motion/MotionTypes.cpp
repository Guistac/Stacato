#include <pch.h>

#include "MotionTypes.h"




std::vector<PositionUnit> linearPositionUnits = {
	{PositionUnit::Unit::METER, PositionUnit::Type::LINEAR,			"Meter",		"Meters",		"m",		"Meter"},
	{PositionUnit::Unit::MILLIMETER, PositionUnit::Type::LINEAR,	"Millimeter",	"Millimeters",	"mm",		"Millimeter"}
};

std::vector<PositionUnit> angularPositionUnits = {
	{PositionUnit::Unit::DEGREE, PositionUnit::Type::ANGULAR,		"Degree",		"Degrees",		"deg",		"Degrees"},
	{PositionUnit::Unit::RADIAN, PositionUnit::Type::ANGULAR,		"Radian",		"Radians",		"rad",		"Radians"},
	{PositionUnit::Unit::REVOLUTION, PositionUnit::Type::ANGULAR,	"Revolution",	"Revolutions",	"rev",		"Revolutions"}
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
const char* getPositionUnitStringSingular(PositionUnit::Unit u) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (unit.unit == u) return unit.displayName;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (unit.unit == u) return unit.displayName;
	}
	return "";
}
const char* getPositionUnitStringPlural(PositionUnit::Unit u) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (unit.unit == u) return unit.displayNamePlural;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (unit.unit == u) return unit.displayNamePlural;
	}
	return "";
}
const char* getPositionUnitStringShort(PositionUnit::Unit u) {
	for (PositionUnit& unit : linearPositionUnits) {
		if (unit.unit == u) return unit.shortForm;
	}
	for (PositionUnit& unit : angularPositionUnits) {
		if (unit.unit == u) return unit.shortForm;
	}
	return "";
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
	{PositionFeedback::Type::INCREMENTAL_FEEDBACK, "Incremental Feedback", "Incremental"}
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





std::vector<PositionReference> linearPositionReferences = {
	{PositionReference::Type::LOW_LIMIT_SIGNAL, "Low Limit Signal", "Low"},
	{PositionReference::Type::HIGH_LIMIT_SIGNAL, "High Limit Signal", "High"},
	{PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS, "Low and High Limit Signals", "LowHigh"}
};
std::vector<PositionReference> angularPositionReferences = {
	{PositionReference::Type::LOW_LIMIT_SIGNAL, "Low Limit Signal", "Low"},
	{PositionReference::Type::HIGH_LIMIT_SIGNAL, "High Limit Signal", "High"},
	{PositionReference::Type::LOW_AND_HIGH_LIMIT_SIGNALS, "Low and High Limit Signals", "LowHigh"},
	{PositionReference::Type::REFERENCE_SIGNAL, "Reference Signal", "Reference"},
	{PositionReference::Type::FEEDBACK_REFERENCE, "Feedback Reference", "Feedback"},
	{PositionReference::Type::NO_LIMIT, "No Position Limits", "None"}
};
std::vector<PositionReference>& getLinearPositionReferences() { return linearPositionReferences; }
std::vector<PositionReference>& getAngularPositionReferences() { return angularPositionReferences; }
PositionReference* getPositionReference(PositionReference::Type t) {
	for (PositionReference& positionLimit : linearPositionReferences) {
		if (positionLimit.type == t) return &positionLimit;
	}
	for (PositionReference& positionLimit : angularPositionReferences) {
		if (positionLimit.type == t) return &positionLimit;
	}
	return nullptr;
}
PositionReference* getPositionReference(const char* saveName) {
	for (PositionReference& positionLimit : linearPositionReferences) {
		if (strcmp(saveName, positionLimit.saveName) == 0) return &positionLimit;
	}
	for (PositionReference& positionLimit : angularPositionReferences) {
		if (strcmp(saveName, positionLimit.saveName) == 0) return &positionLimit;
	}
	return nullptr;
}
bool isLinearPositionReference(PositionReference::Type t) {
	for (PositionReference& positionLimit : linearPositionReferences) {
		if (positionLimit.type == t) return true;
	}
	return false;
}
bool isAngularPositionReference(PositionReference::Type t) {
	for (PositionReference& positionLimit : angularPositionReferences) {
		if (positionLimit.type == t) return true;
	}
	return false;
}




std::vector<MotionControl> motionControlTypes = {
	{MotionControl::Type::CLOSED_LOOP_CONTROL, "Closed Loop", "ClosedLoop"},
	{MotionControl::Type::OPEN_LOOP_CONTROL, "Open Loop", "OpenLoop"},
	{MotionControl::Type::SERVO_CONTROL, "Servo Control", "Servo"}
};

std::vector<MotionControl>& getMotionControlTypes() { return motionControlTypes; }
MotionControl* getMotionControlType(MotionControl::Type t) {
	for (MotionControl& motionControlType : motionControlTypes) {
		if (motionControlType.type == t) return &motionControlType;
	}
	return nullptr;
}
MotionControl* getMotionControlType(const char* saveName) {
	for (MotionControl& motionControlType : motionControlTypes) {
		if (strcmp(saveName, motionControlType.saveName) == 0) return &motionControlType;
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
	{HomingDirection::Type::POSITIVE, "Positive", "Positive"}
};

std::vector<HomingDirection>& getHomingDirections() { return homingDirectionTypes; }

HomingDirection* getHomingDirection(HomingDirection::Type t) {
	for (HomingDirection& direction : homingDirectionTypes) {
		if (direction.type == t) return &direction;
	}
	return nullptr;
}

HomingDirection* getHomingDirection(const char* saveName) {
	for (HomingDirection& direction : homingDirectionTypes) {
		if (strcmp(saveName, direction.saveName)) return &direction;
	}
	return nullptr;
}
