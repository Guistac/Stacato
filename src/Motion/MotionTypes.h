#pragma once

enum class MovementType{
	LINEAR,
	ROTARY
};

#define MovementTypeStrings {MovementType::LINEAR, "Linear Movement", "Linear"},\
							{MovementType::ROTARY, "Rotary Movement", "Rotary"}\

DEFINE_ENUMERATOR(MovementType, MovementTypeStrings)

//================================================================================================================================

enum class PositionFeedbackType{
	ABSOLUTE,
	INCREMENTAL
};

#define PositionFeedbackTypeStrings 	{PositionFeedbackType::ABSOLUTE, "Absolute Feedback", "Absolute"},\
										{PositionFeedbackType::INCREMENTAL, "Incremental Feedback", "Incremental"}\

DEFINE_ENUMERATOR(PositionFeedbackType, PositionFeedbackTypeStrings)



//================================================================================================================================

enum class InterpolationType {
	STEP,
	LINEAR,
	TRAPEZOIDAL,
	BEZIER
};

#define InterpolationTypeStrings \
	{InterpolationType::STEP,			"Step", "Step"},\
	{InterpolationType::LINEAR,			"Linear", "Linear"},\
	{InterpolationType::TRAPEZOIDAL,	"Constant Acceleration", "Trapezoidal"},\
	{InterpolationType::BEZIER,			"Bezier", "Bezier"}\

DEFINE_ENUMERATOR(InterpolationType, InterpolationTypeStrings)

//================================================================================================================================

enum class PositionReferenceSignal{
	SIGNAL_AT_LOWER_LIMIT,
	SIGNAL_AT_LOWER_AND_UPPER_LIMIT,
	SIGNAL_AT_ORIGIN,
	NO_SIGNAL
};

inline bool isLinearPositionReferenceSignal(PositionReferenceSignal t){
	/*
	switch(t){
		case PositionReferenceSignal::NO_SIGNAL:
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			return false;
		default:
			return true;
	}
	 */
	return true;
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

//===============================

enum class SignalApproach{
	FIND_SIGNAL_EDGE,
	FIND_SIGNAL_CENTER
};

#define SignalApproachStrings \
{SignalApproach::FIND_SIGNAL_EDGE, 		"Find Signal Edge",		"FindSignalEdge"},\
{SignalApproach::FIND_SIGNAL_CENTER,	"Find Signal Center",	"FindSignalCenter"}

DEFINE_ENUMERATOR(SignalApproach, SignalApproachStrings)

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
	//SETTING_REFERENCE_LOW,
	SEARCHING_REFERENCE_FROM_ABOVE_COARSE,
	FOUND_REFERENCE_FROM_ABOVE_COARSE,
	SEARCHING_REFERENCE_FROM_ABOVE_FINE,
	FOUND_REFERENCE_FROM_ABOVE_FINE,
	//SETTING_REFERENCE_HIGH,
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
	{HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE,	.displayString = "Searching Reference From Above Coarse"},\
	{HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE,		.displayString = "Found Reference From Above Coarse"},\
	{HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE,	.displayString = "Searching Reference From Above Fine"},\
	{HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE,		.displayString = "Found Reference From Above Fine"},\
	{HomingStep::MOVING_TO_REFERENCE_MIDDLE,			.displayString = "Moving To Reference Middle"},\
	{HomingStep::FINISHED,								.displayString = "Finished"}\

DEFINE_ENUMERATOR(HomingStep, HomingStepStrings)

//================================================================================================================================

enum class HomingError{
	NONE,
	HOMING_CANCELED,
	TRIGGERED_WRONG_LIMIT_SIGNAL,
	HOMING_NOT_SUPORTED
};

#define HomingErrorStrings \
	{HomingError::NONE,							"No Error"},\
	{HomingError::HOMING_CANCELED,				"Homing Canceled"},\
	{HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL,	"Error: Hit Wrong Limit Signal"},\
	{HomingError::HOMING_NOT_SUPORTED, 			"Error: Homing is not supported"}\

DEFINE_ENUMERATOR(HomingError, HomingErrorStrings)

//================================================================================================================================

enum class AnimatableType {
	BOOLEAN,
	INTEGER,
	REAL,
	STATE,
	VECTOR_2D,
	VECTOR_3D,
	POSITION,
	POSITION_2D,
	POSITION_3D,
	VELOCITY,
	VELOCITY_2D,
	VELOCITY_3D,
	COMPOSITE
};

#define AnimatableTypeStrings \
	{AnimatableType::BOOLEAN, 		"Boolean", 				"Boolean"},\
	{AnimatableType::INTEGER, 		"Integer", 				"Integer"},\
	{AnimatableType::REAL, 			"Real", 				"Real"},\
	{AnimatableType::STATE, 		"State", 				"State"},\
	{AnimatableType::VECTOR_2D, 	"2D Vector", 			"2DVector"},\
	{AnimatableType::VECTOR_3D, 	"3D Vector", 			"3DVector"},\
	{AnimatableType::POSITION, 		"Position", 			"Position"},\
	{AnimatableType::POSITION_2D, 	"2D Position", 			"2DPosition"},\
	{AnimatableType::POSITION_3D, 	"3D Position", 			"3DPosition"},\
	{AnimatableType::VELOCITY, 		"Velocity", 			"Velocity"},\
	{AnimatableType::VELOCITY_2D, 	"2D Velocity", 			"2DVelocity"},\
	{AnimatableType::VELOCITY_3D, 	"3D Velocity", 			"3DVelocity"},\
	{AnimatableType::COMPOSITE, 	"Composite Parameter", 	"Composite"}\

DEFINE_ENUMERATOR(AnimatableType, AnimatableTypeStrings)

//================================================================================================================================

enum class ManoeuvreType{
	KEY,
	TARGET,
	SEQUENCE
};

#define ManoeuvreTypeStrings \
	{ManoeuvreType::KEY,		"Key",				"Key"},\
	{ManoeuvreType::TARGET,		"Target",			"Target"},\
	{ManoeuvreType::SEQUENCE,	"Sequence",			"Sequence"}\

DEFINE_ENUMERATOR(ManoeuvreType, ManoeuvreTypeStrings)


//================================================================================================================================

enum class DeviceState{
	OFFLINE,
	NOT_READY,
	READY,
	ENABLING,
	DISABLING,
	ENABLED
};

