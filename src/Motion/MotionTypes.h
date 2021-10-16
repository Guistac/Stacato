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


struct PositionReference {
	enum class Type {
		LOW_LIMIT_SIGNAL,
		HIGH_LIMIT_SIGNAL,
		LOW_AND_HIGH_LIMIT_SIGNALS,
		REFERENCE_SIGNAL,
		FEEDBACK_REFERENCE,
		NO_LIMIT
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionReference>& getLinearPositionReferences();
std::vector<PositionReference>& getAngularPositionReferences();
PositionReference* getPositionReference(PositionReference::Type t);
PositionReference* getPositionReference(const char* savedName);
bool isLinearPositionReference(PositionReference::Type t);
bool isAngularPositionReference(PositionReference::Type t);

struct MotionControl {
	enum class Type {
		CLOSED_LOOP_CONTROL,
		OPEN_LOOP_CONTROL,
		SERVO_CONTROL
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<MotionControl>& getMotionControlTypes();
MotionControl* getMotionControlType(MotionControl::Type t);
MotionControl* getMotionControlType(const char*);

struct CommandType {
	enum class Type {
		POSITION_COMMAND,
		VELOCITY_COMMAND
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<CommandType>& getCommandTypes();
CommandType* getCommandType(CommandType::Type t);
CommandType* getCommandType(const char*);

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


struct HomingStep {
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
		MOVING_BACK_BELOW_REFERENCE,
		SEARCHING_REFERENCE_FROM_BELOW_FINE,
		FOUND_REFERENCE_FROM_BELOW_FINE,
		SETTING_REFERENCE_LOW,
		MOVING_ABOVE_REFERENCE,
		SEARCHING_REFERENCE_FROM_ABOVE_COARSE,
		FOUND_REFERENCE_FROM_ABOVE_COARSE,
		MOVING_BACK_ABOVE_REFERENCE,
		SEARCHING_REFERENCE_FROM_ABOVE_FINE,
		FOUND_REFERENCE_FROM_ABOVE_FINE,
		SETTING_REFERENCE_HIGH,
		MOVING_BELOW_REFERENCE,
		MOVING_TO_REFERENCE_MIDDLE,

		FINISHED
	};
};
HomingStep* getHomingStep(HomingStep::Step s);