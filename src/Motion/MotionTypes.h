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
	OFFLINE = 0,
	NOT_READY = 1,
	DISABLING = 2,
	READY = 3,
	ENABLING = 4,
	ENABLED = 5
};

inline std::string deviceStateToString(DeviceState state){
	switch(state){
		case DeviceState::OFFLINE: return "Offline";
		case DeviceState::NOT_READY: return "Not Ready";
		case DeviceState::DISABLING: return "Disabling";
		case DeviceState::READY: return "Ready";
		case DeviceState::ENABLING: return "Enabling";
		case DeviceState::ENABLED: return "Enabled";
	}
}

