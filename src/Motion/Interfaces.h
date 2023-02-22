#pragma once

#include "MotionTypes.h"

class DeviceInterface{
public:
	enum class Type{
		GPIO,
		MOTION_FEEDBACK,
		ACTUATOR,
		AXIS
	};
	
	virtual Type getType() = 0;
	
	virtual std::string getName() { assert("DeviceModule::getName() : Method is not implemented"); }
	virtual std::string getStatusString() { assert("DeviceModule::getStatusString() : Method is not implemented"); }
	
	DeviceState getState() { return state; }
	
	bool hasFault(){ return b_hasFault; }
	
	bool b_hasFault = false;
	
	bool isOnline(){
		switch(state){
			case DeviceState::OFFLINE:
				return false;
			default:
				return true;
		}
	}
	
	bool isReady(){
		switch(state){
			case DeviceState::ENABLED:
			case DeviceState::ENABLING:
			case DeviceState::READY:
				return true;
			default:
				return false;
		}
	}
	
	bool isEnabling(){ return state == DeviceState::ENABLING; }
	
	bool isEnabled(){
		switch(state){
			case DeviceState::ENABLED:
				return true;
			default:
				return false;
		}
	}
	
	DeviceState state = DeviceState::OFFLINE;
};



class GpioInterface : public DeviceInterface{
public:
	virtual Type getType() override { return Type::GPIO; }
};



class MotionFeedbackInterface : public GpioInterface{
public:
	
	virtual Type getType() override { return Type::MOTION_FEEDBACK; }

	//supported data
	bool supportsPosition(){	return feedbackConfig.b_supportsPositionFeedback; }
	bool supportsVelocity(){ 	return feedbackConfig.b_suppportsVelocityFeedback; }
	bool supportsForce(){		return feedbackConfig.b_supportsForceFeedback; }
	
	//position feedback type
	PositionFeedbackType getPositionFeedbackType(){ return feedbackConfig.positionFeedbackType; }
	
	//position override
	virtual void overridePosition(double newPosition){ 	assert("MotionFeedbackInterface::overridePosition() : method is not implemented"); }
	virtual bool isBusyOverridingPosition(){ 			assert("MotionFeedbackInterface::isBusyOverridingPosition() : method is not implemented");  }
	virtual bool didPositionOverrideSucceed(){			assert("MotionFeedbackInterface::didPositionOverrideSucceed() : method is not implemented"); }
	
	//feedback data retrieval
	Unit getPositionUnit(){ return positionUnit; }
	double getPosition(){ 	return feedbackProcessData.positionActual; }
	double getVelocity(){ 	return feedbackProcessData.velocityActual; }
	double getForce(){ 		return feedbackProcessData.forceActual; }
	
	double getPositionLowerWorkingRangeBound(){ return feedbackConfig.positionLowerWorkingRangeBound; }
	double getPositionUpperWorkingRangeBound(){ return feedbackConfig.positionUpperWorkingRangeBound; }
	double getPositionNormalizedToWorkingRange(){ return (feedbackProcessData.positionActual - feedbackConfig.positionLowerWorkingRangeBound) / (feedbackConfig.positionUpperWorkingRangeBound - feedbackConfig.positionLowerWorkingRangeBound); }
	
	Unit positionUnit = Units::None::None;
	
	struct FeedbackConfiguration{
		bool b_supportsPositionFeedback = false;
		bool b_suppportsVelocityFeedback = false;
		bool b_supportsForceFeedback = false;
		PositionFeedbackType positionFeedbackType = PositionFeedbackType::INCREMENTAL;
		double positionLowerWorkingRangeBound = 0.0;	//lower position bound in position Units
		double positionUpperWorkingRangeBound = 0.0;	//upper position bound in position Units
	}feedbackConfig;
	
	struct Data{
		double positionActual = 0.0; 	//actual position value in position Units
		double velocityActual = 0.0;	//actual velocity value in position Units
		double forceActual = 0.0;		//linear force in Newton or torque in Newton Meters
	}feedbackProcessData;
};



class ActuatorInterface : public MotionFeedbackInterface{
public:
	
	virtual Type getType() override {return Type::ACTUATOR; };
	
	virtual void enable(){		assert("ActuatorInterface::enable() : method is not implemented"); }
	virtual void disable(){	assert("ActuatorInterface::disable() : method is not implemented"); }
	virtual void quickstop(){ }
	virtual void applyHoldingBrake(){}
	virtual void releaseHoldingBrake(){}
	
	enum class ControlMode{
		POSITION,
		VELOCITY,
		FORCE
	};
	
	//supported control and feedback modes
	bool supportsPositionControl(){ 	return actuatorConfig.b_supportsPositionControl; }
	bool supportsVelocityControl(){ 	return actuatorConfig.b_supportsVelocityControl; }
	bool supportsForceControl(){ 		return actuatorConfig.b_supportsForceControl; }
	bool supportsHoldingBrakeControl(){	return actuatorConfig.b_supportsHoldingBrakeControl; }
	bool supportsEffortFeedback(){ 		return actuatorConfig.b_supportsEffortFeedback; }
	
	//actuator limits
	double getAccelerationLimit(){ 	return actuatorConfig.accelerationLimit; }
	double getDecelerationLimit(){ 	return actuatorConfig.decelerationLimit; }
	double getVelocityLimit(){ 		return actuatorConfig.velocityLimit; }
	double getForceLimitPositive(){ return actuatorConfig.forceLimitPositive; }
	double getForceLimitNegative(){ return actuatorConfig.forceLimitNegative; }
	double getFollowingErrorLimit(){ return actuatorConfig.followingErrorLimit; }
	
	//actuator feedback
	double getFollowingError(){ 			return actuatorProcessData.followingErrorActual; }
	double getFollowingErrorNormalized(){ 	return std::abs(actuatorProcessData.followingErrorActual / actuatorConfig.followingErrorLimit); }
	double getEffort(){ 					return actuatorProcessData.effortActual; }
	
	//get Estop state
	bool isEmergencyStopActive(){ return actuatorProcessData.b_isEmergencyStopActive; }
	bool isHoldingBrakeReleased(){ return actuatorProcessData.b_holdingBrakeIsReleased; }
	
	//set motion targets
	void setPositionTarget(double positionTarget){
		actuatorProcessData.positionTarget = positionTarget;
		actuatorProcessData.controlMode = ControlMode::POSITION;
	}
	void setVelocityTarget(double velocityTarget){
		actuatorProcessData.velocityTarget = velocityTarget;
		actuatorProcessData.controlMode = ControlMode::VELOCITY;
	}
	void setForceTarget(double forceTarget){
		actuatorProcessData.forceTarget = forceTarget;
		actuatorProcessData.controlMode = ControlMode::FORCE;
	}
	
	struct ActuatorConfiguration{
		bool b_supportsPositionControl = false;
		bool b_supportsVelocityControl = false;
		bool b_supportsForceControl = false;
		bool b_supportsEffortFeedback = false;
		bool b_supportsHoldingBrakeControl = false;		//does the actuator have a holding brake that can be toggled manually
		double accelerationLimit = 0.0;		//in position units per second squared (absolute value)
		double decelerationLimit = 0.0;		//in position units per second squared (absolute value)
		double velocityLimit = 0.0;			//in position units per second (absolute value)
		double forceLimitPositive = 0.0;	//in newton for linear actuators and newton meters for rotary actuators (absolute value)
		double forceLimitNegative = 0.0;	//in newton for linear actuators and newton meters for rotary actuators (absolute value)
		double followingErrorLimit = 0.0;	//in position units (absolute value)
		bool b_canQuickstop = false;		//can we execute a quickstop command
	}actuatorConfig;
	
	struct ActuatorProcessData{
		//commands
		ControlMode controlMode = ControlMode::VELOCITY; //current control mode selection (not mode feedback)
		double positionTarget = 0.0;	//in position units
		double velocityTarget = 0.0;	//in position units per second
		double forceTarget = 0.0;		//in newton for linear actuators and newton meters for rotary actuators
		//feedback
		bool b_isEmergencyStopActive = false;
		bool b_holdingBrakeIsReleased = false;
		double followingErrorActual = 0.0;	//in position units
		double effortActual = 0.0;			//control effort in a normalized range (1.0 = 100%)
	}actuatorProcessData;
	
};

class AxisInterface : public ActuatorInterface{
public:
	
	virtual Type getType() override { return Type::AXIS; };
	
	bool isHomeable();
	bool canStartHoming();
	bool isHoming();
	void startHoming();
	void stopHoming();
	bool didHomingSucceed();
};

//virtual std::string getName() override;
//virtual std::string getStatusString() override;
//virtual void overridePosition(double newPosition) override;
//virtual bool isBusyOverridingPosition() override;
//virtual bool didPositionOverrideSucceed() override;
//virtual void enable() override;
//virtual void disable() override;

