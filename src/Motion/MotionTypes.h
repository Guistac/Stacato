#pragma once


enum class UnitType {
	LINEAR,
	ANGULAR,
	UNKNOWN
};


struct AxisType {
	UnitType unitType;
	const char displayName[64];
	const char saveName[64];
};
std::vector<AxisType>& getAxisTypes();
AxisType* getAxisType(UnitType t);
AxisType* getAxisType(const char* savedName);


struct PositionUnit {
	enum class Unit {
		DEGREE,
		RADIAN,
		REVOLUTION,
		METER,
		MILLIMETER,
		UNKNOWN
	};
	Unit unit;
	UnitType type;
	const char displayName[64];
	const char displayNamePlural[64];
	const char saveName[64];
};
std::vector<PositionUnit>& getLinearPositionUnits();
std::vector<PositionUnit>& getAngularPositionUnits();
PositionUnit* getPositionUnitType(PositionUnit::Unit u);
PositionUnit* getPositionUnitType(const char* savedName);


struct PositionFeedback {
	enum class Type {
		ABSOLUTE_FEEDBACK,
		INCREMENTAL_FEEDBACK,
		NO_FEEDBACK,
		UNKNOWN
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
		LOW_LIMIT,
		HIGH_LIMIT,
		LOW_AND_HIGH_LIMIT,
		POSITION_REFERENCE,
		NO_LIMIT,
		UNKNOWN
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<PositionReference>& getPositionReferenceTypes();
PositionReference* getPositionReferenceType(PositionReference::Type t);
PositionReference* getPositionReferenceType(const char* savedName);


struct CommandType {
	enum class Type {
		POSITION_COMMAND,
		VELOCITY_COMMAND,
		UNKNOWN
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
		POSITIVE,
		DONT_CARE,
		UNKNOWN
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
std::vector<HomingDirection>& getHomingDirectionTypes();
HomingDirection* getHomingDirectionType(HomingDirection::Type t);
HomingDirection* getHomingDirectionType(const char* savedName);