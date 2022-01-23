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
	
	double mecanumWheelDistanceFromClawPivot = 0.0;
	double mecanumWheelClawPivotRadiusAngleWhenClosed = 0.0;
	double mecanumWheelCircumference = 0.0;
	
	const PositionUnitType clawPositionUnitType = PositionUnitType::ANGULAR;
	PositionUnit clawPositionUnit = PositionUnit::DEGREE;
	double clawFeedbackUnitsPerClawUnit = 0.0;
	double clawVelocityLimit = 0.0;
	double clawAccelerationLimit = 0.0;
	double clawPositionLimit = 0.0; //min claw position is zero, max position is measured by homing procedure
	
	double clawManualAcceleration = 0.0;
	double linearAxisManualAcceleration = 0.0;
	
	double clawRapidAcceleration = 0.0;
	double clawRapidVelocity = 0.0;
	double linearAxisRapidAcceleration = 0.0;
	double linearAxisRapidVelocity = 0.0;
	
	//=== Motion
	
	Motion::Profile linearAxisMotionProfile;
	float getLinearAxisPositionProgress(){}
	float getLinearAxisVelocityProgress(){}
	float linearAxisManualVelocityTarget = 0.0;
	float linearAxisManualPositionTarget = 0.0;
	void setLinearAxisVelocityTarget(float target){}
	void moveLinearAxisToPosition(float target){}
	
	Motion::Profile clawAxisMotionProfile;
	float getClawAxisPositionProgress(){}
	float getClawAxisVelocityProgress(){}
	double clawAxisManualVelocityTarget = 0.0;
	
};
