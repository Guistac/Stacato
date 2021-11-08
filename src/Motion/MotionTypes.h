#pragma once

struct PositionUnit {
	enum class Type {
		LINEAR,
		ANGULAR
	};
	enum class Unit {
		DEGREE,
		RADIAN,
		REVOLUTION,
		METER,
		MILLIMETER
	};
	Unit unit;
	PositionUnit::Type type;
	const char displayName[64];
	const char displayNamePlural[64];
	const char shortForm[8];
	const char saveName[64];
};
std::vector<PositionUnit>& getLinearPositionUnits();
std::vector<PositionUnit>& getAngularPositionUnits();
PositionUnit* getPositionUnit(PositionUnit::Unit u);
PositionUnit* getPositionUnit(const char* savedName);
const char* getPositionUnitStringSingular(PositionUnit::Unit u);
const char* getPositionUnitStringPlural(PositionUnit::Unit u);
const char* getPositionUnitStringShort(PositionUnit::Unit u);


struct PositionUnitType {
	PositionUnit::Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionUnitType>& getPositionUnitTypes();
PositionUnitType* getPositionUnitType(PositionUnit::Type t);
PositionUnitType* getPositionUnitType(const char* saveName);


struct PositionFeedback {
	enum class Type {
		ABSOLUTE_FEEDBACK,
		INCREMENTAL_FEEDBACK
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionFeedback>& getPositionFeedbackTypes();
PositionFeedback* getPositionFeedbackType(PositionFeedback::Type t);
PositionFeedback* getPositionFeedbackType(const char* savedName);


struct PositionReferenceSignal {
	enum class Type {
		SIGNAL_AT_LOWER_LIMIT,
		SIGNAL_AT_LOWER_AND_UPPER_LIMIT,
		SIGNAL_AT_ORIGIN,
		NO_SIGNAL
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionReferenceSignal>& getLinearPositionReferenceSignals();
std::vector<PositionReferenceSignal>& getAngularPositionReferenceSignals();
PositionReferenceSignal* getPositionReferenceSignal(PositionReferenceSignal::Type t);
PositionReferenceSignal* getPositionReferenceSignal(const char* savedName);
bool isLinearPositionReferenceSignal(PositionReferenceSignal::Type t);
bool isAngularPositionReferenceSignal(PositionReferenceSignal::Type t);

struct PositionControl {
	enum class Type {
		CLOSED_LOOP,
		SERVO
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionControl>& getPositionControlTypes();
PositionControl* getPositionControlType(PositionControl::Type t);
PositionControl* getPositionControlType(const char* s);

struct MotionCommand {
	enum class Type {
		POSITION_COMMAND,
		VELOCITY_COMMAND
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<MotionCommand>& getMotionCommand();
MotionCommand* getMotionCommand(MotionCommand::Type t);
MotionCommand* getMotionCommand(const char*);

struct ControlMode {
	enum class Mode {
		MANUAL_VELOCITY_TARGET,
		MANUAL_POSITION_TARGET,
		FAST_STOP
	};
	Mode mode;
	const char displayName[64];
	const char saveName[64];
};
std::vector<ControlMode>& getControlModes();
ControlMode* getControlMode(ControlMode::Mode m);
ControlMode* getControlMode(const char*);


struct HomingDirection {
	enum class Type {
		NEGATIVE,
		POSITIVE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<HomingDirection>& getHomingDirections();
HomingDirection* getHomingDirection(HomingDirection::Type t);
HomingDirection* getHomingDirection(const char* savedName);


namespace Homing {
	enum class Step {
		NOT_STARTED,

		//steps for searching low limit
		SEARCHING_LOW_LIMIT_COARSE,
		FOUND_LOW_LIMIT_COARSE,
		SEARCHING_LOW_LIMIT_FINE,
		FOUND_LOW_LIMIT_FINE,
		RESETTING_POSITION_FEEDBACK,

		//stepts for searching high limit
		SEARCHING_HIGH_LIMIT_COARSE,
		FOUND_HIGH_LIMIT_COARSE,
		SEARCHING_HIGH_LIMIT_FINE,
		FOUND_HIGH_LIMIT_FINE,
		SETTING_HIGH_LIMIT,

		//steps for searching position reference
		SEARCHING_REFERENCE_FROM_BELOW_COARSE,
		FOUND_REFERENCE_FROM_BELOW_COARSE,
		SEARCHING_REFERENCE_FROM_BELOW_FINE,
		FOUND_REFERENCE_FROM_BELOW_FINE,
		SETTING_REFERENCE_LOW,
		SEARCHING_REFERENCE_FROM_ABOVE_COARSE,
		FOUND_REFERENCE_FROM_ABOVE_COARSE,
		SEARCHING_REFERENCE_FROM_ABOVE_FINE,
		FOUND_REFERENCE_FROM_ABOVE_FINE,
		SETTING_REFERENCE_HIGH,
		MOVING_TO_REFERENCE_MIDDLE,

		FINISHED
	};

	enum class Error {
		NONE,
		HOMING_CANCELED,
		TRIGGERED_WRONG_LIMIT_SIGNAL
	};
}

struct HomingStep {
	Homing::Step step;
	const char displayName[64];
};
HomingStep* getHomingStep(Homing::Step s);

struct HomingError {
	Homing::Error error;
	const char displayName[64];
};
HomingError* getHomingError(Homing::Error e);
