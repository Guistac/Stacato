#include <pch.h>

#include "MotionTypes.h"


std::vector<PositionUnit> linearPositionUnits = {
	{PositionUnit::Unit::METER, PositionUnit::Type::LINEAR,			"Meter",		"Meters",		"m",		"Meter"},
	{PositionUnit::Unit::MILLIMETER, PositionUnit::Type::LINEAR,	"Millimeter",	"Millimeters",	"mm",		"Millimeter"}
};

std::vector<PositionUnit> angularPositionUnits = {
	{PositionUnit::Unit::DEGREE, PositionUnit::Type::ANGULAR,		"Degree",		"Degrees",		"\xC2\xB0",	"Degrees"},
	{PositionUnit::Unit::RADIAN, PositionUnit::Type::ANGULAR,		"Radian",		"Radians",		"rad",		"Radians"},
	{PositionUnit::Unit::REVOLUTION, PositionUnit::Type::ANGULAR,	"Revolution",	"Revolutions",	"rev",		"Revolutions"}
};

std::vector<PositionUnit>& getLinearPositionUnits() { return linearPositionUnits; }

std::vector<PositionUnit>& getAngularPositionUnits() { return angularPositionUnits; }

bool isLinearPositionUnit(PositionUnit::Unit t){
	for(auto& linearPositionUnit : linearPositionUnits) if(t == linearPositionUnit.unit) return true;
	return false;
}

bool isAngularPositionUnit(PositionUnit::Unit t){
	for(auto& angularPositionUnit : angularPositionUnits) if(t == angularPositionUnit.unit) return true;
	return false;
}

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





std::vector<PositionReferenceSignal> linearPositionReferenceSignals = {
	{PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT, "Signal At Lower Limit", "LowSignal"},
	{PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT, "Signal At Lower and Upper Limit", "LowHighSignals"}
};
std::vector<PositionReferenceSignal> angularPositionReferenceSignals = {
	{PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT, "Signal At Lower Limit", "LowSignal"},
	{PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT, "Low and High Limit Signals", "LowHighSignals"},
	{PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN, "Signal at Origin", "OriginSignal"},
	{PositionReferenceSignal::Type::NO_SIGNAL, "No Reference Signal", "None"}
};
std::vector<PositionReferenceSignal>& getLinearPositionReferenceSignals() { return linearPositionReferenceSignals; }
std::vector<PositionReferenceSignal>& getAngularPositionReferenceSignals() { return angularPositionReferenceSignals; }
bool isLinearPositionReferenceSignal(PositionReferenceSignal::Type type){
	for(auto& positionReferenceSignal : linearPositionReferenceSignals) if(positionReferenceSignal.type == type) return true;
	return false;
}
bool isAngularPositionReferenceSignal(PositionReferenceSignal::Type type){
	for(auto& positionReferenceSignal : angularPositionReferenceSignals) if(positionReferenceSignal.type == type) return true;
	return false;
}
PositionReferenceSignal* getPositionReferenceSignal(PositionReferenceSignal::Type t) {
	for (PositionReferenceSignal& positionLimit : linearPositionReferenceSignals) {
		if (positionLimit.type == t) return &positionLimit;
	}
	for (PositionReferenceSignal& positionLimit : angularPositionReferenceSignals) {
		if (positionLimit.type == t) return &positionLimit;
	}
	return nullptr;
}
PositionReferenceSignal* getPositionReferenceSignal(const char* saveName) {
	for (PositionReferenceSignal& positionLimit : linearPositionReferenceSignals) {
		if (strcmp(saveName, positionLimit.saveName) == 0) return &positionLimit;
	}
	for (PositionReferenceSignal& positionLimit : angularPositionReferenceSignals) {
		if (strcmp(saveName, positionLimit.saveName) == 0) return &positionLimit;
	}
	return nullptr;
}


std::vector<PositionControl> positionControlTypes = {
	{PositionControl::Type::CLOSED_LOOP, "Closed Loop", "ClosedLoop"},
	{PositionControl::Type::SERVO, "Servo", "Servo"}
};

std::vector<PositionControl>& getPositionControlTypes() { return positionControlTypes; }
PositionControl* getPositionControlType(PositionControl::Type t) {
	for (PositionControl& motionControlType : positionControlTypes) {
		if (motionControlType.type == t) return &motionControlType;
	}
	return nullptr;
}
PositionControl* getPositionControlType(const char* saveName) {
	for (PositionControl& motionControlType : positionControlTypes) {
		if (strcmp(saveName, motionControlType.saveName) == 0) return &motionControlType;
	}
	return nullptr;
}




std::vector<MotionCommand> motionCommandTypes = {
	{MotionCommand::Type::POSITION_COMMAND, "Position Command", "Position"},
	{MotionCommand::Type::VELOCITY_COMMAND, "Velocity Command", "Velocity"}
};

std::vector<MotionCommand>& getMotionCommands() { return motionCommandTypes; }

MotionCommand* getMotionCommand(MotionCommand::Type t) {
	for (MotionCommand& command : motionCommandTypes) {
		if (command.type == t) return &command;
	}
	return nullptr;
}

MotionCommand* getMotionCommand(const char* saveName) {
	for (MotionCommand& command : motionCommandTypes) {
		if (strcmp(saveName, command.saveName) == 0) return &command;
	}
	return nullptr;
}

std::vector<ControlMode> controlModes = {
	{ControlMode::Mode::VELOCITY_TARGET,	"Velocity Target"},
	{ControlMode::Mode::POSITION_TARGET,	"Position Target"},
	{ControlMode::Mode::FAST_STOP,			"Fast Stop"},
	{ControlMode::Mode::MACHINE_CONTROL,	"Machine Control"}
};
std::vector<ControlMode>& getControlModes() {
	return controlModes;
}
ControlMode* getControlMode(ControlMode::Mode m) {
	for (auto& controlMode : controlModes) {
		if (m == controlMode.mode) return &controlMode;
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
		if (strcmp(saveName, direction.saveName) == 0) return &direction;
	}
	return nullptr;
}

std::vector<HomingStep> homingSteps = {
	{Homing::Step::NOT_STARTED,								"Not Started"},
	{Homing::Step::SEARCHING_LOW_LIMIT_COARSE,				"Searching Low Limit Coarse"},
	{Homing::Step::FOUND_LOW_LIMIT_COARSE,					"Found Low Limit Coarse"},
	{Homing::Step::SEARCHING_LOW_LIMIT_FINE,				"Searching Low Limit Fine"},
	{Homing::Step::FOUND_LOW_LIMIT_FINE,					"Found Low Limit Fine"},
	{Homing::Step::RESETTING_POSITION_FEEDBACK,				"Resetting Position Feedback"},
	{Homing::Step::SEARCHING_HIGH_LIMIT_COARSE,				"Searching High Limit Coarse"},
	{Homing::Step::FOUND_HIGH_LIMIT_COARSE,					"Found High Limit Coarse"},
	{Homing::Step::SEARCHING_HIGH_LIMIT_FINE,				"Seraching High Limit Fine"},
	{Homing::Step::FOUND_HIGH_LIMIT_FINE,					"Found High Limit Fine"},
	{Homing::Step::SETTING_HIGH_LIMIT,						"Setting High Limit"},
	{Homing::Step::SEARCHING_REFERENCE_FROM_BELOW_COARSE,	"Searching Reference From Below Coarse"},
	{Homing::Step::FOUND_REFERENCE_FROM_BELOW_COARSE,		"Found Reference From Below Coarse"},
	{Homing::Step::SEARCHING_REFERENCE_FROM_BELOW_FINE,		"Searching Reference From Below Fine"},
	{Homing::Step::FOUND_REFERENCE_FROM_BELOW_FINE,			"Found Reference From Below Fine"},
	{Homing::Step::SETTING_REFERENCE_LOW,					"Setting Reference Low"},
	{Homing::Step::SEARCHING_REFERENCE_FROM_ABOVE_COARSE,	"Searching Reference From Above Coarse"},
	{Homing::Step::FOUND_REFERENCE_FROM_ABOVE_COARSE,		"Found Reference From Above Coarse"},
	{Homing::Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE,		"Searching Reference From Above Fine"},
	{Homing::Step::FOUND_REFERENCE_FROM_ABOVE_FINE,			"Found Reference From Above Fine"},
	{Homing::Step::SETTING_REFERENCE_HIGH,					"Setting Reference High"},
	{Homing::Step::MOVING_TO_REFERENCE_MIDDLE,				"Moving To Reference Middle"},
	{Homing::Step::FINISHED,								"Finished"}
};

HomingStep* getHomingStep(Homing::Step s) {
	for (auto& step : homingSteps) {
		if (s == step.step) return &step;
	}
	return nullptr;
}

std::vector<HomingError> homingErrors = {
	{Homing::Error::NONE,							"No Error"},
	{Homing::Error::HOMING_CANCELED,				"Homing Canceled"},
	{Homing::Error::TRIGGERED_WRONG_LIMIT_SIGNAL,	"Error: Hit Wrong Limit Signal"}
};

HomingError* getHomingError(Homing::Error e) {
	for (auto& error : homingErrors) {
		if (e == error.error) return &error;
	}
	return nullptr;
}
