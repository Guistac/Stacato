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
		INCREMENTAL_FEEDBACK,
		NO_FEEDBACK
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionFeedback>& getPositionFeedbackTypes();
PositionFeedback* getPositionFeedbackType(PositionFeedback::Type t);
PositionFeedback* getPositionFeedbackType(const char* savedName);


struct PositionLimitType {
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
std::vector<PositionLimitType>& getLinearPositionLimitTypes();
std::vector<PositionLimitType>& getAngularPositionLimitTypes();
PositionLimitType* getPositionLimitType(PositionLimitType::Type t);
PositionLimitType* getPositionLimitType(const char* savedName);
bool isLinearPositionLimit(PositionLimitType::Type t);
bool isAngularPositionLimit(PositionLimitType::Type t);

struct MotionControlType {
	enum class Type {
		CLOSED_LOOP_CONTROL,
		OPEN_LOOP_CONTROL,
		SERVO_CONTROL
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<MotionControlType>& getMotionControlTypes();
MotionControlType* getMotionControlType(MotionControlType::Type t);
MotionControlType* getMotionControlType(const char*);

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
std::vector<HomingDirection>& getHomingDirectionTypes();
HomingDirection* getHomingDirectionType(HomingDirection::Type t);
HomingDirection* getHomingDirectionType(const char* savedName);