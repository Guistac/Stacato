#pragma once

#include "MotionTypes.h"

class Device;

class SubDevice{
public:

	SubDevice(std::string n) : name(n){}
	
	enum class Type {
		GPIO,
		ACTUATOR,
		SERVO_ACTUATOR,
		VELOCITY_FEEDBACK,
		POSITION_FEEDBACK,
	};
	virtual Type getType() = 0;
	
	virtual MotionState getState() { return state; }
	virtual std::string& getName() { return name; }
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
	std::string name;
	bool b_hasFault;
	bool b_emergencyStopActive;
};


class GpioDevice : public SubDevice {
public:
	GpioDevice(std::string name) : SubDevice(name){}
	
	virtual Type getType() override { return SubDevice::Type::GPIO; }
};


class MotionDevice : public SubDevice {
public:
	
	MotionDevice(std::string name, Unit unit) : SubDevice(name), positionUnit(unit){}
	
	Unit getPositionUnit() { return positionUnit; }
	
	//settings
	Unit positionUnit;
};


class VelocityFeedbackDevice : public virtual MotionDevice{
public:
	
	VelocityFeedbackDevice(std::string name, Unit unit) : MotionDevice(name, unit){}
	
	virtual Type getType() override { return SubDevice::Type::VELOCITY_FEEDBACK; }
	
	double getVelocity() { return velocity; }
	bool isMoving() { return b_isMoving; }
	
	//current data
	bool b_isMoving = false;
	double velocity;
};


class PositionFeedbackDevice : public VelocityFeedbackDevice {
public:
	
	PositionFeedbackDevice(std::string name, Unit unit, PositionFeedbackType feedbackType) : VelocityFeedbackDevice(name, unit), positionFeedbackType(feedbackType){}

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
	double position;
};


class ActuatorDevice : public virtual MotionDevice {
public:
	
	ActuatorDevice(std::string name, Unit unit) : MotionDevice(name, unit){}

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
	
	ServoActuatorDevice(std::string name, Unit unit, PositionFeedbackType feedbackType) : ActuatorDevice(name, unit), PositionFeedbackDevice(name, unit, feedbackType){}

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
