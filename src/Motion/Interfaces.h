#pragma once

#include "MotionTypes.h"

class DeviceInterface : public std::enable_shared_from_this<DeviceInterface>{
public:
	enum class Type{
		GPIO,
		MOTION_FEEDBACK,
		ACTUATOR,
		AXIS
	};
	
	virtual Type getType() = 0;
	
	virtual std::string getName() { return "..."; }
	virtual std::string getStatusString() { return "..."; }
	
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
	bool supportsVelocity(){ 	return feedbackConfig.b_supportsVelocityFeedback; }
	bool supportsForce(){		return feedbackConfig.b_supportsForceFeedback; }
	
	//position feedback type
	PositionFeedbackType getPositionFeedbackType(){ return feedbackConfig.positionFeedbackType; }
	
	//position override
	void overridePosition(double newPosition){
		feedbackProcessData.positionOverride = newPosition;
		feedbackProcessData.b_overridePosition = true;
	}
	bool isBusyOverridingPosition(){ return feedbackProcessData.b_positionOverrideBusy; }
	bool didPositionOverrideSucceed() { return feedbackProcessData.b_positionOverrideSucceeded; }
	
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
		bool b_supportsVelocityFeedback = false;
		bool b_supportsForceFeedback = false;
		PositionFeedbackType positionFeedbackType = PositionFeedbackType::INCREMENTAL;
		double positionLowerWorkingRangeBound = 0.0;	//lower position bound in position Units
		double positionUpperWorkingRangeBound = 0.0;	//upper position bound in position Units
	}feedbackConfig;
	
	struct Data{
		double positionActual = 0.0; 	//actual position value in position Units
		double velocityActual = 0.0;	//actual velocity value in position Units
		double forceActual = 0.0;		//linear force in Newton or torque in Newton Meters
		bool b_overridePosition = false;
		bool b_positionOverrideBusy = false;
		bool b_positionOverrideSucceeded = false;
		double positionOverride = 0.0;
	}feedbackProcessData;
};



class ActuatorInterface : public MotionFeedbackInterface{
public:
	
	virtual Type getType() override {return Type::ACTUATOR; };
	
	void enable(){ actuatorProcessData.b_enable = true; }
	void disable(){ actuatorProcessData.b_disable = true; }
	void quickstop(){ actuatorProcessData.b_quickstop = true; }
	void applyHoldingBrake(){ actuatorProcessData.b_applyHoldingBrake = true; }
	void releaseHoldingBrake(){ actuatorProcessData.b_releaseHoldingBrake = true; }
	 
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
	double getVelocityNormalized(){			return feedbackProcessData.velocityActual / actuatorConfig.velocityLimit; }
	
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
		bool b_enable = false;
		bool b_enabling = false;
		bool b_disable = false;
		uint64_t enableRequest_nanos;
		bool b_applyHoldingBrake = false;
		bool b_releaseHoldingBrake = false;
		bool b_quickstop = false;
		//feedback
		bool b_isEmergencyStopActive = false;
		bool b_holdingBrakeIsReleased = false;
		double followingErrorActual = 0.0;	//in position units
		double effortActual = 0.0;			//control effort in a normalized range (1.0 = 100%)
	}actuatorProcessData;
	
};

class AxisInterface : public DeviceInterface{
public:
	
	enum class ControlMode{
		POSITION_CONTROL,
		VELOCITY_CONTROL,
		NONE
	};
	
	Unit getPositionUnit(){ return configuration.positionUnit; }
	
	virtual Type getType() override { return Type::AXIS; };
	
	bool supportsHoming(){ return configuration.b_supportsHoming; }
	bool canStartHoming(){ return processData.b_canStartHoming; }
	bool isHoming(){ return processData.b_isHoming; }
	bool didHomingSucceed(){ return processData.b_didHomingSucceed; }
	void startHoming() { processData.b_startHoming = true; }
	void stopHoming() { processData.b_stopHoming = true; }
	
	void setPositionTarget(double positionTarget, double velocityTarget, double accelerationTarget){
		processData.positionTarget = positionTarget;
		processData.velocityTarget = velocityTarget;
		processData.accelerationTarget = accelerationTarget;
	}
	void setVelocityTarget(double velocityTarget, double accelerationTarget){
		processData.velocityTarget = velocityTarget;
		processData.accelerationTarget = accelerationTarget;
	}
	
	bool isEmergencyStopActive(){ return processData.b_isEmergencyStopActive; }
	
	double getPositionNormalizedToLimits(){
		return (processData.positionActual - configuration.lowerPositionLimit) / (configuration.upperPositionLimit - configuration.lowerPositionLimit);
	}
	double getVelocityNormalizedToLimits(){
		return processData.velocityActual / configuration.velocityLimit;
	}
	double getVelocityTargetNormalizedToLimits(){
		return processData.velocityTarget / configuration.velocityLimit;
	}
	double getPositionActual(){ return processData.positionActual; }
	double getVelocityActual(){ return processData.velocityActual; }
	double getForceActual(){ return processData.forceActual; }
	double getEffortActual(){ return processData.effortActual; }
	
	void enable(){ processData.b_enable = true; }
	void disable(){ processData.b_disable = true; }
	
	double getVelocityLimit(){ return configuration.velocityLimit; }
	double getAccelerationLimit(){ return configuration.accelerationLimit; }
	double getLowerPositionLimit(){ return configuration.lowerPositionLimit; }
	double getUpperPositionLimit(){ return configuration.upperPositionLimit; }
	
	bool isAtLowerLimit(){ return processData.b_isAtLowerLimit; }
	bool isAtUpperLimit(){ return processData.b_isAtUpperLimit; }
	
	std::string getHomingStepString(){ return configuration.homingStateStringCallback(); }
	
private:
	
	friend class AxisNode;
	
	struct AxisConfiguration{
		Unit positionUnit = Units::None::None;
		bool b_supportsPositionFeedback;
		bool b_supportsVelocityFeedback;
		bool b_supportsForceFeedback;
		bool b_supportsEffortFeedback;
		bool b_supportsHoming;
		double lowerPositionLimit;
		double upperPositionLimit;
		double velocityLimit;
		double accelerationLimit;
		double decelerationLimit;
		ControlMode controlMode;
		std::function<std::string(void)> homingStateStringCallback;
	}configuration;
	
	struct AxisProcessData{
		double positionActual;
		double velocityActual;
		double forceActual;
		double effortActual;
		double positionTarget;
		double velocityTarget;
		double accelerationTarget;
		bool b_canStartHoming;
		bool b_startHoming;
		bool b_stopHoming;
		bool b_isHoming;
		bool b_didHomingSucceed;
		bool b_isEmergencyStopActive;
		bool b_enable;
		bool b_disable;
		bool b_isAtLowerLimit;
		bool b_isAtUpperLimit;
	}processData;
};

//virtual std::string getName() override;
//virtual std::string getStatusString() override;
//virtual void overridePosition(double newPosition) override;
//virtual bool isBusyOverridingPosition() override;
//virtual bool didPositionOverrideSucceed() override;
//virtual void enable() override;
//virtual void disable() override;

