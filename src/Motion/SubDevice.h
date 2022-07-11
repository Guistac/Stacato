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
	
	virtual MotionState getState() = 0;
	virtual std::string getName() = 0;
	virtual bool hasFault() = 0;
	virtual std::string getStatusString() = 0;

	//set the parent device, which will provide further information on device status
	void setParentDevice(std::shared_ptr<Device> pd) { parentDevice = pd; }
	std::shared_ptr<Device> parentDevice = nullptr;

	bool isOnline() {
		switch(getState()){
			case MotionState::OFFLINE: return false;
			case MotionState::EMERGENY_STOP:
			case MotionState::NOT_READY:
			case MotionState::READY:
			case MotionState::ENABLED: return true;
		}
	}
	bool isReady() {
		switch(getState()){
			case MotionState::OFFLINE:
			case MotionState::EMERGENY_STOP:
			case MotionState::NOT_READY: return false;
			case MotionState::READY:
			case MotionState::ENABLED: return true;
		}
	}
	bool isEnabled(){
		switch(getState()){
			case MotionState::OFFLINE:
			case MotionState::EMERGENY_STOP:
			case MotionState::NOT_READY:
			case MotionState::READY: return false;
			case MotionState::ENABLED: return true;
		}
	}
	bool isEmergencyStopped(){
		switch(getState()){
			case MotionState::OFFLINE: return false;
			case MotionState::EMERGENY_STOP: return true;
			case MotionState::NOT_READY:
			case MotionState::READY:
			case MotionState::ENABLED: return false;
		}
	}
};


class GpioDevice : public SubDevice {
public:
	virtual Type getType() override { return SubDevice::Type::GPIO; }
};


class MotionDevice : public SubDevice {
public:
	virtual Unit getPositionUnit() = 0;
};


class VelocityFeedbackDevice : public virtual MotionDevice{
public:
	
	virtual Type getType() override { return SubDevice::Type::VELOCITY_FEEDBACK; }
	
	virtual double getVelocity() = 0;
	virtual bool isMoving() = 0;
};


class PositionFeedbackDevice : public VelocityFeedbackDevice {
public:

	virtual Type getType() override { return SubDevice::Type::POSITION_FEEDBACK; }
	
	virtual PositionFeedbackType getPositionFeedbackType() = 0;
	
	virtual bool isInsideWorkingRange() = 0;
	virtual double getPositionInWorkingRange() = 0;
	virtual double getMinPosition() = 0;
	virtual double getMaxPosition() = 0;
	virtual double getPosition() = 0;
	
	virtual bool canHardReset() { return false; }
	virtual void executeHardReset() {}
	virtual bool isExecutingHardReset() { return false; }
	
	virtual bool canHardOverride() { return false; }
	virtual void executeHardOverride(double overridePosition) {}
	virtual bool isExecutingHardOverride() { return false; }
	
	virtual void softOverridePosition(double position) = 0;
};


class ActuatorDevice : public virtual MotionDevice {
public:

	virtual Type getType() override { return SubDevice::Type::ACTUATOR; }

	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void quickstop() = 0;
	
	virtual double getVelocityLimit() = 0;
	virtual double getAccelerationLimit() = 0;
	virtual void setVelocityCommand(double velocityCommand, double accelerationCommand) = 0;
	
	virtual bool hasManualHoldingBrake() { return false; }
	virtual bool isHoldingBrakeApplied() { return false; }
	virtual void releaseHoldingBrake() {}
	virtual void applyHoldingBrake() {}

	virtual double getLoad() = 0;
};


class ServoActuatorDevice : public ActuatorDevice, public PositionFeedbackDevice {
public:

	virtual Type getType() override { return SubDevice::Type::SERVO_ACTUATOR; }

	virtual void setPositionCommand(double positionCommand, double velocityCommand, double accelerationCommand) = 0;
	
	virtual double getFollowingError() = 0;
	virtual double getMaxFollowingError() = 0;
	virtual double getFollowingErrorInRange() = 0;
};
