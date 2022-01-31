#pragma once

#include "Motion/Machine/Machine.h"
#include "Motion/Profile/Profile.h"

class LinearMecanumClaw : public Machine {
public:
	
	DEFINE_MACHINE_NODE(LinearMecanumClaw, "Linear Mecanum Claw", "LinearMecanumClaw", "Special")
	
	//=== Input Pins & Data
	
	std::shared_ptr<bool> clawClosedSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> clawOpenSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> linearAxisPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT, "Linear Axis");
	std::shared_ptr<NodePin> clawAxisPin = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT, "Claw Axis");
	std::shared_ptr<NodePin> clawPositionFeedbackPin = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_INPUT, "Claw Position Feedback");
	std::shared_ptr<NodePin> clawReferenceDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Claw Reference Device");
	std::shared_ptr<NodePin> clawClosedPin = std::make_shared<NodePin>(clawClosedSignal, NodePin::Direction::NODE_INPUT, "Claw Closed");
	std::shared_ptr<NodePin> clawOpenPin = std::make_shared<NodePin>(clawOpenSignal, NodePin::Direction::NODE_INPUT, "Claw Open");
	
	//=== Output Pins & Data
	
	std::shared_ptr<double> railPosition = std::make_shared<double>(0.0);
	std::shared_ptr<double> railVelocity = std::make_shared<double>(0.0);
	std::shared_ptr<double> clawPosition = std::make_shared<double>(0.0);
	std::shared_ptr<double> clawVelocity = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> railPositionPin = std::make_shared<NodePin>(railPosition, NodePin::Direction::NODE_OUTPUT, "Rail Position");
	std::shared_ptr<NodePin> railVelocityPin = std::make_shared<NodePin>(railVelocity, NodePin::Direction::NODE_OUTPUT, "Rail Velcoity");
	std::shared_ptr<NodePin> clawPositionPin = std::make_shared<NodePin>(clawPosition, NodePin::Direction::NODE_OUTPUT, "Claw Position");
	std::shared_ptr<NodePin> clawVelocityPin = std::make_shared<NodePin>(clawVelocity, NodePin::Direction::NODE_OUTPUT, "Claw Velocity");
	
	//=== Animatable Machine Parameters
	
	std::shared_ptr<AnimatableParameter> linearAxisPositionParameter = std::make_shared<AnimatableParameter>("Position Costière", ParameterDataType::KINEMATIC_POSITION_CURVE, "m");
	std::shared_ptr<AnimatableParameter> clawAxisPositionParameter = std::make_shared<AnimatableParameter>("Angle Coeur", ParameterDataType::KINEMATIC_POSITION_CURVE, "deg");
	
	//=== Device Checking & Gettings
	
	bool isLinearAxisConnected(){ return linearAxisPin->isConnected(); }
	bool isClawAxisConnected(){ return clawAxisPin->isConnected(); }
	bool isClawFeedbackConnected() { return clawPositionFeedbackPin->isConnected(); }
	bool isClawReferenceConnected(){ return clawReferenceDevicePin->isConnected(); }
	
	std::shared_ptr<PositionControlledAxis> getLinearAxis(){ return linearAxisPin->getConnectedPin()->getSharedPointer<PositionControlledAxis>(); }
	std::shared_ptr<VelocityControlledAxis> getClawAxis(){ return clawAxisPin->getConnectedPin()->getSharedPointer<VelocityControlledAxis>(); }
	std::shared_ptr<PositionFeedbackDevice> getClawFeedbackDevice() { return clawPositionFeedbackPin->getConnectedPin()->getSharedPointer<PositionFeedbackDevice>(); }
	std::shared_ptr<GpioDevice> getClawReferenceDevice(){ return clawReferenceDevicePin->getConnectedPin()->getSharedPointer<GpioDevice>(); }
	
	bool areHardwareNodesConnected(){
		if(!isLinearAxisConnected()) return false;
		if(!isClawAxisConnected()) return false;
		if(!isClawFeedbackConnected()) return false;
		if(!isClawReferenceConnected()) return false;
	}
	
	//=== Parameters
	
	const PositionUnitType clawPositionUnitType = PositionUnitType::ANGULAR;
	PositionUnit clawPositionUnit = PositionUnit::DEGREE;
	double clawFeedbackUnitsPerClawUnit = 0.0;
	double clawVelocityLimit = 0.0;
	double clawAccelerationLimit = 0.0;
	double clawPositionLimit = 0.0; //min claw position is zero, max position is measured by homing procedure
	double clawPositionLoopProportionalGain = 0.0;
	double clawPositionLoopIntegralGain = 0.0;
	double integratedClawPositionErrorLimit = 5.0;
	double clawMaxPositionFollowingError = 0.0;
	double clawPositionErrorThreshold = 0.0;
	double clawHomingVelocity = 0.0;
	
	double mecanumWheelDistanceFromClawPivot = 0.0;
	double mecanumWheelClawPivotRadiusAngleWhenClosed = 0.0;
	double mecanumWheelCircumference = 0.0;
	
	double clawManualAcceleration = 0.0;
	double linearAxisManualAcceleration = 0.0;
	
	double clawRapidAcceleration = 0.0;
	double clawRapidVelocity = 0.0;
	double linearAxisRapidAcceleration = 0.0;
	double linearAxisRapidVelocity = 0.0;
	
	void sanitizeParameters();
	
	double clawFeedbackUnitsToClawUnits(double clawFeedbackValue){ return clawFeedbackValue / clawFeedbackUnitsPerClawUnit; }
	double clawUnitsToClawFeedbackUnits(double clawValue){ return clawValue * clawFeedbackUnitsPerClawUnit; }
	
	//=== Motion
	
	double profileTime_seconds = -1.0;
	double profileDeltaTime_seconds = -1.0;
	
	Motion::Profile linearAxisMotionProfile;
	ControlMode linearControlMode = ControlMode::VELOCITY_TARGET;
	float linearAxisManualVelocityTarget = 0.0;
	
	Motion::Profile clawAxisMotionProfile;
	ControlMode clawControlMode = ControlMode::VELOCITY_TARGET;
	double clawAxisManualVelocityTarget = 0.0;
	
	double integratedClawPositionError = 0.0;
	
	void setLinearVelocity(double velocity);
	void moveLinearToTargetInTime(double positionTarget, double timeTarget);
	void moveLinearToTargetWithVelocity(double positionTarget, double velocityTarget);
	void fastStopLinear();
	
	void setClawVelocity(double velocity);
	void moveClawToTargetInTime(double positionTarget, double timeTarget);
	void moveClawToTargetWithVelocity(double positionTarget, double velocityTarget);
	void fastStopClaw();
	
	enum class ClawHomingStep{
		NOT_STARTED,
		SEARCHING_HEART_CLOSED_LIMIT,
		FOUND_HEART_CLOSED_LIMIT,
		RESETTING_HEART_FEEDBACK,
		HOMING_LINEAR_AXIS,
		HOMING_FINISHED,
		HOMING_FAILED
	};
	
	void startHoming();
	void stopHoming();
	bool isHoming(){ return b_isHoming; }
	bool b_isHoming = false;
	ClawHomingStep homingStep = ClawHomingStep::NOT_STARTED;
	const char* getHomingStepString();
	void onHomingSuccess();
	void onHomingFailure();
	
	void homingControl();
	
	double getLinearAxisPosition();
	double getLinearAxisVelocity();
	double getLinearAxisFollowingError();
	double getClawAxisPosition();
	double getClawAxisVelocity();
	
	float getLinearAxisPositionProgress();
	float getLinearAxisVelocityProgress();
	float getLinearAxisFollowingErrorProgress();
	float getClawAxisPositionProgress();
	float getClawAxisVelocityProgress();
	
	PositionUnit getLinearAxisPositionUnit();
	PositionUnit getClawAxisPositionUnit();
	
	bool hasLinearAxisTargetMovement() { return !linearAxisMotionProfile.isInterpolationFinished(profileTime_seconds); }
	bool hasClawAxisTargetMovement() { return !clawAxisMotionProfile.isInterpolationFinished(profileTime_seconds); }
	double getLinearAxisRemainingTargetMovementTime() { return linearAxisMotionProfile.getRemainingInterpolationTime(profileTime_seconds); }
	double getClawAxisRemainingTargetMovementTime() { return clawAxisMotionProfile.getRemainingInterpolationTime(profileTime_seconds); }
	double getLinearAxisTargetMovementProgress() { return linearAxisMotionProfile.getInterpolationProgress(profileTime_seconds); }
	double getClawAxisTargetMovementProgress() { return clawAxisMotionProfile.getInterpolationProgress(profileTime_seconds); }
	double getLinearAxisMovementTargetNormalized();
	double getClawAxisMovementTargetNormalized();
	double getClawAxisFollowingError(){ return clawAxisMotionProfile.getPosition() - getClawAxisPosition(); }
	double getClawAxisFollowingErrorProgress(){ return std::abs(getClawAxisFollowingError()) / clawMaxPositionFollowingError; }
	
	//=== Gui Stuff
	float linearManualVelocityDisplay = 0.0;
	float linearPositionTargetDisplay = 0.0;
	float linearVelocityTargetDisplay = 0.0;
	float linearTimeTargetDisplay = 0.0;
	float clawManualVelocityDisplay = 0.0;
	float clawPositionTargetDisplay = 0.0;
	float clawVelocityTargetDisplay = 0.0;
	float clawTimeTargetDisplay = 0.0;
	
};


#define ClawHomingStepStrings \
	{LinearMecanumClaw::ClawHomingStep::NOT_STARTED, 					"Homing Not Started"},\
	{LinearMecanumClaw::ClawHomingStep::SEARCHING_HEART_CLOSED_LIMIT, 	"Searching Heart Closed Limit"},\
	{LinearMecanumClaw::ClawHomingStep::FOUND_HEART_CLOSED_LIMIT, 		"Found Heart Closed Limit"},\
	{LinearMecanumClaw::ClawHomingStep::RESETTING_HEART_FEEDBACK, 		"Resetting Hearth Position Feedback"},\
	{LinearMecanumClaw::ClawHomingStep::HOMING_LINEAR_AXIS, 			"Homing Linear Axis"},\
	{LinearMecanumClaw::ClawHomingStep::HOMING_FINISHED, 				"Finished Homing"},\
	{LinearMecanumClaw::ClawHomingStep::HOMING_FAILED, 					"Homing Failed"}\

DEFINE_ENUMERATOR(LinearMecanumClaw::ClawHomingStep, ClawHomingStepStrings)


//reenable after claw error overrun
//plot for claw machine !!!
//error correction disable treshold for claw axis
//error correction disable theshold for linear axis
//kill claw velocity when sensor triggered
//homing sequence for claw machine
//sanitize claw parameters
//encoder reset when homing claw
