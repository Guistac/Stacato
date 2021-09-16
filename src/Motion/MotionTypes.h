#pragma once


enum class UnitType {
	LINEAR,
	ANGULAR
};


struct AxisType {
	UnitType unitType;
	const char displayName[64];
	const char displayNamePlural[64];
	const char saveName[64];
};
std::vector<AxisType>& getAxisTypes();
AxisType& getAxisType(UnitType t);


struct PositionUnit {
	enum class Unit {
		DEGREE,
		RADIAN,
		ROTATION,
		METER,
		MILLIMETER
	};
	Unit unit;
	UnitType type;
	const char displayName[64];
	const char displayNamePlural[64];
	const char saveName[64];
};
std::vector<PositionUnit>& getLinearPositionUnits();
std::vector<PositionUnit>& getAngularPositionUnits();
PositionUnit& getPositionUnitType(PositionUnit::Unit u);


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
PositionFeedback& getPositionFeedbackType(PositionFeedback::Type t);
std::vector<PositionFeedback>& getPositionFeedbackTypes();


struct PositionReference {
	enum class Type {
		LOW_LIMIT,
		HIGH_LIMIT,
		LOW_AND_HIGH_LIMIT,
		POSITION_REFERENCE,
		NO_LIMIT
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
PositionReference& getPositionReferenceType(PositionReference::Type t);
std::vector<PositionReference>& getPositionReferenceTypes();


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
CommandType& getCommandType(CommandType::Type t);


struct HomingDirection {
	enum class Type {
		NEGATIVE,
		POSITIVE,
		DONT_CARE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
HomingDirection& getHomingDirectionType(HomingDirection::Type t);
std::vector<HomingDirection>& getHomingDirectionTypes();