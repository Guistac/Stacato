#pragma once

#include "MotionTypes.h"

class Device;

class SubDevice{
public:
	
	enum class Type {
		GPIO,
		ACTUATOR,
		SERVO_ACTUATOR,
		VELOCITY_FEEDBACK,
		POSITION_FEEDBACK,
	};
	virtual Type getType() = 0;
	
	virtual MotionState getState() { return state; }
	virtual std::string getName() = 0;
	virtual bool hasFault() { return b_hasFault; }
	virtual std::string getStatusString() = 0;

	//set the parent device, which will provide further information on device status
	void setParentDevice(std::shared_ptr<Device> pd) { parentDevice = pd; }
	std::shared_ptr<Device> parentDevice = nullptr;
	
	virtual bool isEmergencyStopped() { return b_emergencyStopActive; }

	bool isOnline() {
		switch(getState()){
			case MotionState::OFFLINE: return false;
			case MotionState::NOT_READY:
			case MotionState::READY:
			case MotionState::ENABLED: return true;
		}
	}
	bool isReady() {
		switch(getState()){
			case MotionState::OFFLINE:
			case MotionState::NOT_READY: return false;
			case MotionState::READY:
			case MotionState::ENABLED: return true;
		}
	}
	bool isEnabled(){
		switch(getState()){
			case MotionState::OFFLINE:
			case MotionState::NOT_READY:
			case MotionState::READY: return false;
			case MotionState::ENABLED: return true;
		}
	}
	
	MotionState state = MotionState::OFFLINE;
	bool b_hasFault;
	bool b_emergencyStopActive;
};


class GpioDevice : public SubDevice {
public:
	GpioDevice() : SubDevice(){}
	
	virtual Type getType() override { return SubDevice::Type::GPIO; }
};


class MotionDevice : public SubDevice {
public:
	
	MotionDevice(Unit unit) : SubDevice(), positionUnit(unit){}
	
	Unit getPositionUnit() { return positionUnit; }
	
	//settings
	Unit positionUnit;
};


class VelocityFeedbackDevice : public virtual MotionDevice{
public:
	
	VelocityFeedbackDevice(Unit unit) : MotionDevice(unit){}
	
	virtual Type getType() override { return SubDevice::Type::VELOCITY_FEEDBACK; }
	
	double getVelocity() { return velocity; }
	bool isMoving() { return b_isMoving; }
	
	//current data
	bool b_isMoving = false;
	double velocity = 0.0;
};


class PositionFeedbackDevice : public VelocityFeedbackDevice {
public:
	
	PositionFeedbackDevice(Unit unit, PositionFeedbackType feedbackType) : VelocityFeedbackDevice(unit), positionFeedbackType(feedbackType){}

	Type getType() override { return SubDevice::Type::POSITION_FEEDBACK; }
	
	PositionFeedbackType getPositionFeedbackType() { return positionFeedbackType; }
	
	double getPosition() { return position - positionOffset; }
	
	virtual bool canHardReset() { return false; }
	virtual void executeHardReset() {}
	virtual bool isExecutingHardReset() { return false; }
	
	virtual bool canHardOverride() { return false; }
	virtual void executeHardOverride(double overridePosition) {}
	virtual bool isExecutingHardOverride() { return false; }
	
	void softOverridePosition(double softPosition) { positionOffset = position - softPosition; }
	
	bool isInsideWorkingRange() { return position >= minWorkingRange && position <= maxWorkingRange; }
	double getPositionInWorkingRange() { return (position - minWorkingRange) / (maxWorkingRange - minWorkingRange); }
	double getMinPosition() { return minWorkingRange - positionOffset; }
	double getMaxPosition() { return maxWorkingRange - positionOffset; }
	
	//settings
	PositionFeedbackType positionFeedbackType;
	double positionOffset = 0.0;
	double minWorkingRange;
	double maxWorkingRange;
	
	//current data
	double position = 0.0;
};


class ActuatorDevice : public virtual MotionDevice {
public:
	
	ActuatorDevice(Unit unit) : MotionDevice(unit){}

	virtual Type getType() override { return SubDevice::Type::ACTUATOR; }

	void enable() { b_enable = true; }
	void disable() { b_disable = true; }
	void quickstop() { b_quickstop = true; }
	
	void setVelocityCommand(double velocityCommand, double accelerationCommand) {
		targetVelocity = velocityCommand;
		targetAcceleration = accelerationCommand;
	}
	double getVelocityLimit() { return velocityLimit; }
	double getAccelerationLimit() { return accelerationLimit; }
	double getDecelerationLimit() {
		if(b_decelerationLimitEqualsAccelerationLimit) return accelerationLimit;
		else return decelerationLimit;
	}
	bool decelerationLimitEqualsAccelerationLimit(){ return b_decelerationLimitEqualsAccelerationLimit; }
	
	bool hasManualHoldingBrake() { return b_hasHoldingBrake; }
	bool isHoldingBrakeReleased() { return b_holdingBrakeIsReleased; }
	void releaseHoldingBrake() { b_releaseHoldingBrake = true; }
	void applyHoldingBrake() { b_applyHoldingBrake = true; }
	
	double getLoad() { return load; }
	
	//state change commands
	bool b_enable = false;
	bool b_disable = false;
	bool b_quickstop = false;
		
	//settings
	double velocityLimit;
	double accelerationLimit;
	double decelerationLimit;
	bool b_decelerationLimitEqualsAccelerationLimit;
	
	//current data
	double load;
	
	//target data
	double targetVelocity;
	double targetAcceleration;
	
	bool b_hasHoldingBrake = false;
	bool b_holdingBrakeIsReleased = false;
	bool b_applyHoldingBrake = false;
	bool b_releaseHoldingBrake = false;
};


class ServoActuatorDevice : public ActuatorDevice, public PositionFeedbackDevice {
public:
	
	ServoActuatorDevice(Unit unit, PositionFeedbackType feedbackType) : ActuatorDevice(unit), PositionFeedbackDevice(unit, feedbackType){}

	virtual Type getType() override { return SubDevice::Type::SERVO_ACTUATOR; }
	
	void setPositionCommand(double positionCommand, double velocityCommand, double accelerationCommand) {
		targetPosition = positionCommand + positionOffset;
		targetVelocity = velocityCommand;
		targetAcceleration = accelerationCommand;
	};
	
	double getFollowingError() { return followingError; }
	double getMaxFollowingError() { return maxFollowingError; }
	double getFollowingErrorInRange() { return std::abs(followingError / maxFollowingError); }
	
	//settings
	double maxFollowingError;
	
	//current data
	double followingError;
	
	//target data
	double targetPosition;
};










//————————————— New Device Module classes
// these are way simpled and more flexible

class DeviceModule{
public:
	enum class Type{
		GPIO,
		MOTION_FEEDBACK,
		ACTUATOR
	};

	enum class State{
		OFFLINE,
		NOT_READY,
		READY,
		ENABLING,
		DISABLING,
		ENABLED
	};
	
	virtual Type getType() = 0;
	
	virtual std::string getName() { assert("DeviceModule::getName() : Method is not implemented"); }
	virtual std::string getStatusString() { assert("DeviceModule::getStatusString() : Method is not implemented"); }
	
	State getState() { return state; }
	
protected:
	State state = State::OFFLINE;
};

class GpioModule : public DeviceModule{
public:
	virtual Type getType() override { return Type::GPIO; }
	
	bool isOnline(){ return state != State::OFFLINE; }
	bool isEnabled(){ return state == State::ENABLED; }
};

class MotionFeedbackModule : public GpioModule{
public:
	
	virtual Type getType() override { return Type::MOTION_FEEDBACK; }

	//supported data
	bool supportsPosition(){	return feedbackConfig.b_supportsPosition; }
	bool supportsVelocity(){ 	return feedbackConfig.b_suppportsVelocity; }
	bool supportsForce(){		return feedbackConfig.b_supportsForce; }
	bool supportsEffort(){ 		return feedbackConfig.b_supportsEffort; }
	
	//position feedback type
	PositionFeedbackType getPositionFeedbackType(){ return feedbackConfig.positionFeedbackType; }
	
	//position override
	virtual void overridePosition(double newPosition){ 	assert("MotionFeedbackModule::overridePosition() : method is not implemented"); }
	virtual bool isBusyOverridingPosition(){ 			assert("MotionFeedbackModule::isBusyOverridingPosition() : method is not implemented");  }
	virtual bool didPositionOverrideSucceed(){			assert("MotionFeedbackModule::didPositionOverrideSucceed() : method is not implemented"); }
	
	//feedback data retrieval
	Unit getPositionUnit(){ return positionUnit; }
	double getPosition(){ 	return feedbackProcessData.positionActual; }
	double getVelocity(){ 	return feedbackProcessData.velocityActual; }
	double getForce(){ 		return feedbackProcessData.forceActual; }
	double getEffort(){ 	return feedbackProcessData.effortActual; }
	
protected:
	
	Unit positionUnit = Units::None::None;
	
	struct FeedbackConfiguration{
		bool b_supportsPosition = false;
		bool b_suppportsVelocity = false;
		bool b_supportsForce = false;
		bool b_supportsEffort = false;
		PositionFeedbackType positionFeedbackType = PositionFeedbackType::INCREMENTAL;
		double positionLowerWorkingRangeBound = 0.0;	//lower position bound in position Units
		double positionUpperWorkingRangeBound = 0.0;	//upper position bound in position Units
	}feedbackConfig;
	
	struct Data{
		double positionActual = 0.0; 	//actual position value in position Units
		double velocityActual = 0.0;	//actual velocity value in position Units
		double forceActual = 0.0;		//linear force in Newton or torque in Newton Meters
		double effortActual = 0.0;		//control effort in a normalized range (1.0 = 100%)
	}feedbackProcessData;
};

class ActuatorModule : public MotionFeedbackModule{
public:
	
	virtual Type getType() override {return Type::ACTUATOR; };
	
	bool isReady(){ return state != State::OFFLINE && state != State::NOT_READY; }
	bool isEnabling(){ return state == State::ENABLING; }
	bool isDisabling(){ return state == State::DISABLING; }
	
	virtual void enable() {		assert("ActuatorModule::enable() : method is not implemented"); }
	virtual void disable() {	assert("ActuatorModule::disable() : method is not implemented"); }
	
	enum class ControlMode{
		POSITION,
		VELOCITY,
		FORCE
	};
	
	//supported control modes
	bool supportsPositionControl(){ return actuatorConfig.b_supportsPosition = false; }
	bool supportsVelocityControl(){ return actuatorConfig.b_supportsVelocity = false; }
	bool supportsForceControl(){ 	return actuatorConfig.b_supportsForce = false; }
	
	//actuator limits
	double getAccelerationLimit(){ 	return actuatorConfig.accelerationLimit; }
	double getDecelerationLimit(){ 	return actuatorConfig.decelerationLimit; }
	double getVelocityLimit(){ 		return actuatorConfig.velocityLimit; }
	double getForceLimitPositive(){ return actuatorConfig.forceLimitPositive; }
	double getForceLimitNegative(){ return actuatorConfig.forceLimitNegative; }
	
	//get Estop state
	bool isEmergencyStopActive(){ return actuatorProcessData.b_isEmergencyStopActive; }
	
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
	
protected:
	
	struct ActuatorConfiguration{
		bool b_supportsPosition = false;
		bool b_supportsVelocity = false;
		bool b_supportsForce = false;
		double accelerationLimit = 0.0;		//in position units per second squared (absolute value)
		double decelerationLimit = 0.0;		//in position units per second squared (absolute value)
		double velocityLimit = 0.0;			//in position units per second (absolute value)
		double forceLimitPositive = 0.0;	//in newton for linear actuators and newton meters for rotary actuators (absolute value)
		double forceLimitNegative = 0.0;	//in newton for linear actuators and newton meters for rotary actuators (absolute value)
	}actuatorConfig;
	
	struct ActuatorControl{
		ControlMode controlMode = ControlMode::VELOCITY; //current control mode selection (not mode feedback)
		double positionTarget = 0.0;	//in position units
		double velocityTarget = 0.0;	//in position units per second
		double forceTarget = 0.0;		//in newton for linear actuators and newton meters for rotary actuators
		bool b_isEmergencyStopActive = false;
	}actuatorProcessData;
	
};


//virtual std::string getName() override;
//virtual std::string getStatusString() override;
//virtual void overridePosition(double newPosition) override;
//virtual bool isBusyOverridingPosition() override;
//virtual bool didPositionOverrideSucceed() override;
//virtual void enable() override;
//virtual void disable() override;

