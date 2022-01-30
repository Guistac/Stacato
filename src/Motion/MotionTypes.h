#pragma once

//================================================================================================================================

enum class PositionUnitType{
	LINEAR,
	ANGULAR
};

#define PositionUnitTypeStrings \
	{PositionUnitType::LINEAR, "Linear", "Linear"},\
	{PositionUnitType::ANGULAR, "Angular", "Angular"}\

DEFINE_ENUMERATOR(PositionUnitType, PositionUnitTypeStrings)

//================================================================================================================================

enum class PositionUnit{
	DEGREE,
	RADIAN,
	REVOLUTION,
	METER,
	CENTIMETER,
	MILLIMETER
};

static std::vector<Unit::Type<PositionUnit>> getPositionUnitTypes(){
	static std::vector<Unit::Type<PositionUnit>> types = {
		{PositionUnit::METER,
			.displayString = "Meter",
			.displayStringPlural = "Meters",
			.abbreviatedString = "m",
			.saveString = "Meter",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 0.0,
			.baseUnitOffset = 0.0
		},
		{PositionUnit::CENTIMETER,
			.displayString = "Centimeter",
			.displayStringPlural = "Centimeters",
			.abbreviatedString = "cm",
			.saveString = "Centimeter",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 0.01,
			.baseUnitOffset = 0.0
		},
		{PositionUnit::MILLIMETER,
			.displayString = "Millimeter",
			.displayStringPlural = "Millimeters",
			.abbreviatedString = "mm",
			.saveString = "Millimeter",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 0.001,
			.baseUnitOffset = 0.0
		},
		{PositionUnit::DEGREE,
			.displayString = "Degree",
			.displayStringPlural = "Degrees",
			.abbreviatedString = "\xC2\xB0",
			.saveString = "Degrees",
			.b_isBaseUnit = true,
			.baseUnitMultiple = 0.0,
			.baseUnitOffset = 0.0},
		{PositionUnit::RADIAN,
			.displayString = "Radian",
			.displayStringPlural = "Radians",
			.abbreviatedString = "rad",
			.saveString = "Radians",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 0.0174533,
			.baseUnitOffset = 0.0
		},
		{PositionUnit::REVOLUTION,
			.displayString = "Revolution",
			.displayStringPlural = "Revolutions",
			.abbreviatedString = "rev",
			.saveString = "Revolutions",
			.b_isBaseUnit = false,
			.baseUnitMultiple = 360.0,
			.baseUnitOffset = 0.0
		}
	};
	return types;
}

DEFINE_UNIT(PositionUnit, getPositionUnitTypes())

inline bool isLinearPositionUnit(PositionUnit t){
	switch(t){
		case PositionUnit::DEGREE:
		case PositionUnit::RADIAN:
		case PositionUnit::REVOLUTION:
			return false;
		case PositionUnit::METER:
		case PositionUnit::CENTIMETER:
		case PositionUnit::MILLIMETER:
			return true;
	}
}

inline bool isAngularPositionUnit(PositionUnit t){
	switch(t){
		case PositionUnit::DEGREE:
		case PositionUnit::RADIAN:
		case PositionUnit::REVOLUTION:
			return true;
		case PositionUnit::METER:
		case PositionUnit::CENTIMETER:
		case PositionUnit::MILLIMETER:
			return false;
	}
}

//================================================================================================================================

enum class PositionFeedbackType{
	ABSOLUTE,
	INCREMENTAL
};

#define PositionFeedbackTypeStrings 	{PositionFeedbackType::ABSOLUTE, "Absolute Feedback", "Absolute"},\
										{PositionFeedbackType::INCREMENTAL, "Incremental Feedback", "Incremental"}\

DEFINE_ENUMERATOR(PositionFeedbackType, PositionFeedbackTypeStrings)

//================================================================================================================================

enum class PositionReferenceSignal{
	SIGNAL_AT_LOWER_LIMIT,
	SIGNAL_AT_LOWER_AND_UPPER_LIMIT,
	SIGNAL_AT_ORIGIN,
	NO_SIGNAL
};

inline bool isLinearPositionReferenceSignal(PositionReferenceSignal t){
	switch(t){
		case PositionReferenceSignal::NO_SIGNAL:
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			return false;
		default:
			return true;
	}
}

inline bool isAngularPositionReferenceSignal(PositionReferenceSignal t){
	return true;
}

//================================================================================================================================

#define PositionReferenceSignalStrings \
	{PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT, 			"Signal At Lower Limit", 			"LowSignal"},\
	{PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT, 	"Low and High Limit Signals", 		"LowHighSignals"},\
	{PositionReferenceSignal::SIGNAL_AT_ORIGIN, 				"Signal at Origin", 				"OriginSignal"},\
	{PositionReferenceSignal::NO_SIGNAL, 						"No Reference Signal", 				"None"}\

DEFINE_ENUMERATOR(PositionReferenceSignal, PositionReferenceSignalStrings)

//================================================================================================================================

enum class MotionCommand {
	POSITION,
	VELOCITY
};

#define MotionCommandStrings 	{MotionCommand::POSITION, "Position Command", "PositionCommand"},\
								{MotionCommand::VELOCITY, "Velocity Command", "VelocityCommand"}\

DEFINE_ENUMERATOR(MotionCommand, MotionCommandStrings)

//================================================================================================================================

enum class ControlMode{
	VELOCITY_TARGET,
	POSITION_TARGET,
	FAST_STOP,
	EXTERNAL
};

#define ControlModeTypes\
	{ControlMode::VELOCITY_TARGET,	"Velocity Target"},\
	{ControlMode::POSITION_TARGET,	"Position Target"},\
	{ControlMode::FAST_STOP,		"Fast Stop"},\
	{ControlMode::EXTERNAL,			"External"}\

DEFINE_ENUMERATOR(ControlMode, ControlModeTypes)

//================================================================================================================================

enum class HomingDirection{
	NEGATIVE,
	POSITIVE
};

#define HomingDirectionTypeStrings	{HomingDirection::NEGATIVE, "Negative", "Negative"},\
									{HomingDirection::POSITIVE, "Positive", "Positive"}\

DEFINE_ENUMERATOR(HomingDirection, HomingDirectionTypeStrings)

//================================================================================================================================

enum class HomingStep{
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

#define HomingStepStrings \
	{HomingStep::NOT_STARTED,							.displayString = "Not Started"},\
	{HomingStep::SEARCHING_LOW_LIMIT_COARSE,			.displayString = "Searching Low Limit Coarse"},\
	{HomingStep::FOUND_LOW_LIMIT_COARSE,				.displayString = "Found Low Limit Coarse"},\
	{HomingStep::SEARCHING_LOW_LIMIT_FINE,				.displayString = "Searching Low Limit Fine"},\
	{HomingStep::FOUND_LOW_LIMIT_FINE,					.displayString = "Found Low Limit Fine"},\
	{HomingStep::RESETTING_POSITION_FEEDBACK,			.displayString = "Resetting Position Feedback"},\
	{HomingStep::SEARCHING_HIGH_LIMIT_COARSE,			.displayString = "Searching High Limit Coarse"},\
	{HomingStep::FOUND_HIGH_LIMIT_COARSE,				.displayString = "Found High Limit Coarse"},\
	{HomingStep::SEARCHING_HIGH_LIMIT_FINE,				.displayString = "Seraching High Limit Fine"},\
	{HomingStep::FOUND_HIGH_LIMIT_FINE,					.displayString = "Found High Limit Fine"},\
	{HomingStep::SETTING_HIGH_LIMIT,					.displayString = "Setting High Limit"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE,	.displayString = "Searching Reference From Below Coarse"},\
	{HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE,		.displayString = "Found Reference From Below Coarse"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE,	.displayString = "Searching Reference From Below Fine"},\
	{HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE,		.displayString = "Found Reference From Below Fine"},\
	{HomingStep::SETTING_REFERENCE_LOW,					.displayString = "Setting Reference Low"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE,	.displayString = "Searching Reference From Above Coarse"},\
	{HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE,		.displayString = "Found Reference From Above Coarse"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE,	.displayString = "Searching Reference From Above Fine"},\
	{HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE,		.displayString = "Found Reference From Above Fine"},\
	{HomingStep::SETTING_REFERENCE_HIGH,				.displayString = "Setting Reference High"},\
	{HomingStep::MOVING_TO_REFERENCE_MIDDLE,			.displayString = "Moving To Reference Middle"},\
	{HomingStep::FINISHED,								.displayString = "Finished"}\

DEFINE_ENUMERATOR(HomingStep, HomingStepStrings)

//================================================================================================================================

enum class HomingError{
	NONE,
	HOMING_CANCELED,
	TRIGGERED_WRONG_LIMIT_SIGNAL
};

#define HomingErrorStrings \
	{HomingError::NONE,							"No Error"},\
	{HomingError::HOMING_CANCELED,				"Homing Canceled"},\
	{HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL,	"Error: Hit Wrong Limit Signal"}\

DEFINE_ENUMERATOR(HomingError, HomingErrorStrings)
