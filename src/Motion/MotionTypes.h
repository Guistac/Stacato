#pragma once



enum class PositionUnitType{
	LINEAR,
	ANGULAR
};

#define PositionUnitTypeStrings \
	{PositionUnitType::LINEAR, "Linear", "Linear"},\
	{PositionUnitType::ANGULAR, "Angular", "Angular"}\

DEFINE_ENUMERATOR(PositionUnitType, PositionUnitTypeStrings)





enum class PositionUnit{
	DEGREE,
	RADIAN,
	REVOLUTION,
	METER,
	CENTIMETER,
	MILLIMETER
};

#define PositionUnitStrings \
	{PositionUnit::METER,		"Meter",		"Meters",		"m",		"Meter"},\
	{PositionUnit::CENTIMETER,	"Centimeter",	"Centimeters",	"cm",		"Centimeter"},\
	{PositionUnit::MILLIMETER,	"Millimeter",	"Millimeters",	"mm",		"Millimeter"},\
	{PositionUnit::DEGREE,		"Degree",		"Degrees",		"\xC2\xB0",	"Degrees"},\
	{PositionUnit::RADIAN,		"Radian",		"Radians",		"rad",		"Radians"},\
	{PositionUnit::REVOLUTION,	"Revolution",	"Revolutions",	"rev",		"Revolutions"}\

DEFINE_UNIT(PositionUnit, PositionUnitStrings)

bool isLinearPositionUnit(PositionUnit t);
bool isAngularPositionUnit(PositionUnit t);





enum class PositionFeedbackType{
	ABSOLUTE,
	INCREMENTAL
};

#define PositionFeedbackTypeStrings 	{PositionFeedbackType::ABSOLUTE, "Absolute Feedback", "Absolute"},\
										{PositionFeedbackType::INCREMENTAL, "Incremental Feedback", "Incremental"}\

DEFINE_ENUMERATOR(PositionFeedbackType, PositionFeedbackTypeStrings)





enum class PositionReferenceSignal{
	SIGNAL_AT_LOWER_LIMIT,
	SIGNAL_AT_LOWER_AND_UPPER_LIMIT,
	SIGNAL_AT_ORIGIN,
	NO_SIGNAL
};

bool isLinearPositionReferenceSignal(PositionReferenceSignal t);
bool isAngularPositionReferenceSignal(PositionReferenceSignal t);

#define PositionReferenceSignalStrings \
	{PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT, 			"Signal At Lower Limit", 			"LowSignal"},\
	{PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT, 	"Signal At Lower and Upper Limit", 	"LowHighSignals"},\
	{PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT, 			"Signal At Lower Limit", 			"LowSignal"},\
	{PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT, 	"Low and High Limit Signals", 		"LowHighSignals"},\
	{PositionReferenceSignal::SIGNAL_AT_ORIGIN, 				"Signal at Origin", 				"OriginSignal"},\
	{PositionReferenceSignal::NO_SIGNAL, 						"No Reference Signal", 				"None"}\

DEFINE_ENUMERATOR(PositionReferenceSignal, PositionReferenceSignalStrings)





enum class PositionControlType{
	CLOSED_LOOP,
	SERVO
};

#define PositionControlTypeStrings	{PositionControlType::CLOSED_LOOP, "Closed Loop", "ClosedLoop"},\
									{PositionControlType::SERVO, "Servo", "Servo"}\

DEFINE_ENUMERATOR(PositionControlType, PositionControlTypeStrings)





enum class MotionCommand {
	POSITION,
	VELOCITY
};

#define MotionCommandStrings 	{MotionCommand::POSITION, "Position Command", "PositionCommand"},\
								{MotionCommand::VELOCITY, "Velocity Command", "VelocityCommand"}\

DEFINE_ENUMERATOR(MotionCommand, MotionCommandStrings)





struct ControlMode {
	enum class Mode {
		VELOCITY_TARGET,
		POSITION_TARGET,
		FAST_STOP,
		MACHINE_CONTROL
	};
	Mode mode;
	const char displayName[64];
};
std::vector<ControlMode>& getControlModes();
ControlMode* getControlMode(ControlMode::Mode m);





enum class HomingDirection{
	NEGATIVE,
	POSITIVE
};

#define HomingDirectionTypeStrings	{HomingDirection::NEGATIVE, "Negative", "Negative"},\
									{HomingDirection::POSITIVE, "Positive", "Positive"}\

DEFINE_ENUMERATOR(HomingDirection, HomingDirectionTypeStrings)





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
	{HomingStep::NOT_STARTED,							"Not Started"},\
	{HomingStep::SEARCHING_LOW_LIMIT_COARSE,			"Searching Low Limit Coarse"},\
	{HomingStep::FOUND_LOW_LIMIT_COARSE,				"Found Low Limit Coarse"},\
	{HomingStep::SEARCHING_LOW_LIMIT_FINE,				"Searching Low Limit Fine"},\
	{HomingStep::FOUND_LOW_LIMIT_FINE,					"Found Low Limit Fine"},\
	{HomingStep::RESETTING_POSITION_FEEDBACK,			"Resetting Position Feedback"},\
	{HomingStep::SEARCHING_HIGH_LIMIT_COARSE,			"Searching High Limit Coarse"},\
	{HomingStep::FOUND_HIGH_LIMIT_COARSE,				"Found High Limit Coarse"},\
	{HomingStep::SEARCHING_HIGH_LIMIT_FINE,				"Seraching High Limit Fine"},\
	{HomingStep::FOUND_HIGH_LIMIT_FINE,					"Found High Limit Fine"},\
	{HomingStep::SETTING_HIGH_LIMIT,					"Setting High Limit"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE,	"Searching Reference From Below Coarse"},\
	{HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE,		"Found Reference From Below Coarse"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE,	"Searching Reference From Below Fine"},\
	{HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE,		"Found Reference From Below Fine"},\
	{HomingStep::SETTING_REFERENCE_LOW,					"Setting Reference Low"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE,	"Searching Reference From Above Coarse"},\
	{HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE,		"Found Reference From Above Coarse"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE,	"Searching Reference From Above Fine"},\
	{HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE,		"Found Reference From Above Fine"},\
	{HomingStep::SETTING_REFERENCE_HIGH,				"Setting Reference High"},\
	{HomingStep::MOVING_TO_REFERENCE_MIDDLE,			"Moving To Reference Middle"},\
	{HomingStep::FINISHED,								"Finished"}\

DEFINE_ENUMERATOR(HomingStep, HomingStepStrings)





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
