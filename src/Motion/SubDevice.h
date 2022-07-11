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
	virtual std::string& getName() = 0;
	virtual bool hasFault() = 0;
	virtual std::string& getStatusString() = 0;

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


class GpioDevice : public MotionDevice {
public:
	virtual Type getSubdeviceType() { return Type::GPIO; }
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


class PositionFeedbackDevice : public virtual MotionDevice {
public:

	virtual Type getSubdeviceType() { return SubDevice::Type::POSITION_FEEDBACK; }
	
	virtual PositionFeedbackType getPositionFeedbackType() = 0;

	virtual void overridePosition(double position) = 0;
	
	virtual bool isInsideWorkingRange() = 0;
	virtual double getPositionInRange() = 0;
	virtual double getMinPosition() = 0;
	virtual double getMaxPosition() = 0;
	virtual double getPosition() = 0;
	
	virtual bool canHardReset() = 0;
	virtual void executeHardReset() = 0;
	virtual bool isExecutingHardReset() = 0;
	
	virtual bool canHardOverride() = 0;
	virtual void executeHardOverride(double overridePosition) = 0;
	virtual bool isExecutingHardOverride() = 0;
};


class ActuatorDevice : public virtual MotionDevice {
public:

	virtual Type getType() override { return SubDevice::Type::ACTUATOR; }

	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void quickstop() = 0;

	virtual void setVelocityCommand(double velocityCommand, double accelerationCommand) = 0;
	virtual double getVelocityCommand() = 0;
	virtual double getAccelerationCommand() = 0;
	
	virtual double getVelocityLimit() = 0;
	virtual double getAccelerationLimit() = 0;

	virtual bool hasManualHoldingBrake() = 0;
	virtual bool isHoldingBrakeApplied() = 0;
	virtual void releaseHoldingBrake() = 0;
	virtual void applyHoldingBrake() = 0;

	virtual double getLoad();
};


class ServoActuatorDevice : public ActuatorDevice, public PositionFeedbackDevice {
public:

	virtual Type getSubdeviceType() { return SubDevice::Type::SERVO_ACTUATOR; }

	virtual void setPositionCommand(double positionCommand, double velocityCommand, double accelerationCommand) = 0;
	
	virtual double getFollowingError() = 0;
	virtual double getFollowingErrorRanged() = 0;
};
