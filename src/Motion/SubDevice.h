#pragma once

#include "MotionTypes.h"

class Device;

class Subdevice {
public:

	Subdevice(const char* n) { setName(n); }

	enum class Type {
		ACTUATOR,
		POSITION_FEEDBACK,
		GPIO,
		SERVO_ACTUATOR
	};
	virtual Type getSubdeviceType() = 0;

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	char name[128];
	
	//set the parent device, which will provide further information on device status
	void setParentDevice(std::shared_ptr<Device> pd) { parentDevice = pd; }
	std::shared_ptr<Device> parentDevice = nullptr;

	//has the device been discovered / seen
	bool isDetected(){ return b_detected; }
	//is the device actively communicating
	bool isOnline() { return b_online; }
	//is the device ready to provide inputs and accept ouputs
	bool isReady() { return b_ready; }

	bool b_detected = false;
	bool b_online = false;
	bool b_ready = false;
};

class PositionDevice: public Subdevice {
public:
	PositionDevice(const char* name, PositionUnit unit) : Subdevice(name), positionUnit(unit) {}
	PositionUnit getPositionUnit() { return positionUnit; }
	PositionUnit positionUnit;
};

class ActuatorDevice : public virtual PositionDevice {
public:

	ActuatorDevice(const char* name, PositionUnit unit) : PositionDevice(name, unit) {}
	virtual Subdevice::Type getSubdeviceType() { return Subdevice::Type::ACTUATOR; }

	//enable power
	void enable() { b_setEnabled = true; }						
	//disable power
	void disable() { b_setDisabled = true; }
	//set velocity command
	virtual void setVelocityCommand(double velocityCommand) { velocityCommand_deviceUnitsPerSecond = velocityCommand; }
	//get velocity command
	virtual double getVelocityCommand() { return velocityCommand_deviceUnitsPerSecond; }
	//don't allow powering the actuator
	void park() { b_setDisabled = true; b_parked = true; }			
	//allow powereing of the actuator
	void unpark() { b_parked = false; }								
	//force a controlled stop
	void quickstop() { b_setQuickstop = true; }						

	//is the actuator powered
	bool isEnabled() { return b_enabled; }
	//is the actuator allowed to power on
	bool isParked() { return b_parked; }
	//is the device disabled by an external emergency stop signal
	bool isEmergencyStopActive() { return b_emergencyStopActive; }
	//are the brakes active
	bool areBrakesActive(){ return b_brakesActive; }

	//get the normalized load of the device
	double getLoad() { return load; }								
	//get the velocity limit in device position units per second
	double getVelocityLimit() { return velocityLimit_positionUnitsPerSecond; }
	//get the minimum velocity allowed by the actuator in units per second
	double getMinVelocity(){ return minVelocity_positionUnitsPerSecond; }
	//get the acceleration limit in device position units per second squared
	double getAccelerationLimit() { return accelerationLimit_positionUnitsPerSecondSquared; }

	bool b_enabled = false;
	bool b_parked = false;
	bool b_emergencyStopActive = false;
	bool b_brakesActive = false;
	double velocityLimit_positionUnitsPerSecond = 0.0;
	double minVelocity_positionUnitsPerSecond = 0.0;
	double accelerationLimit_positionUnitsPerSecondSquared = 0.0;
	double load = 0.0;

	bool b_setEnabled = false;
	bool b_setDisabled = false;
	bool b_setQuickstop = false;
	double velocityCommand_deviceUnitsPerSecond = 0.0;
};



class PositionFeedbackDevice : public virtual PositionDevice {
public:

	PositionFeedbackDevice(const char* name, PositionUnit unit, PositionFeedbackType feedback) : PositionDevice(name, unit), feedbackType(feedback) {}
	virtual Subdevice::Type getSubdeviceType() { return Subdevice::Type::POSITION_FEEDBACK; }

	PositionFeedbackType feedbackType;
	PositionFeedbackType getPositionFeedbackType() { return feedbackType; }

	//set the current position of the encoder by adjusting the zero offset
	void setPosition(double position) { positionOffset_positionUnits = positionRaw_positionUnits - position; }
	//reset the zero position offset of the encoder
	void resetOffset() { positionOffset_positionUnits = 0.0; }
	//is the position feedback inside its operational range
	bool isInRange() { return positionRaw_positionUnits < rangeMax_positionUnits&& positionRaw_positionUnits > rangeMin_positionUnits; }
	//get the normalized position in the operational range of the feedback device
	double getPositionInRange() { return (positionRaw_positionUnits - rangeMin_positionUnits) / (rangeMax_positionUnits - rangeMin_positionUnits); }
	//get smallest encoder position withing working range relative to zero offset
	double getMinPosition() { return rangeMin_positionUnits - positionOffset_positionUnits; }
	//get largest encoder position withing working range relative to zero offset
	double getMaxPosition() { return rangeMax_positionUnits - positionOffset_positionUnits; }
	//get the position with included offset in specified units
	double getPosition() { return positionRaw_positionUnits - positionOffset_positionUnits; }
	//get velocity of encoder movement
	double getVelocity() { return velocity_positionUnitsPerSecond; }
	//is the position moving
	bool isMoving() { return b_moving; }
	
	bool canHardReset(){ return b_canHardReset; }
	void hardReset(){ if(canHardReset()) b_doHardReset = true; }
	bool isHardResetting() {
		if(canHardReset()) return b_isHardResetting;
		else return false;
	}

	double positionRaw_positionUnits = 0.0;
	double positionOffset_positionUnits = 0.0;
	double rangeMax_positionUnits;
	double rangeMin_positionUnits;
	double velocity_positionUnitsPerSecond = 0.0;
	bool b_moving = false;
	
	bool b_doHardReset = false;
	bool b_canHardReset = false;
	bool b_isHardResetting = false;
};



class GpioDevice : public Subdevice {
public:
	GpioDevice(const char* n) : Subdevice(n) {}
	virtual Type getSubdeviceType() { return Type::GPIO; }
};



class ServoActuatorDevice : public ActuatorDevice, public PositionFeedbackDevice {
public:

	ServoActuatorDevice(const char* name, PositionUnit unit, PositionFeedbackType feedback) : PositionDevice(name, unit), ActuatorDevice(name, unit), PositionFeedbackDevice(name, unit, feedback) {}
	virtual Subdevice::Type getSubdeviceType() { return Subdevice::Type::SERVO_ACTUATOR; }

	//set command
	virtual void setPositionCommand(double positionCommand, double velocityCommand) {
		positionCommand_deviceUnits = positionCommand;
		velocityCommand_deviceUnitsPerSecond = velocityCommand;
	}
	//get velocity command
	virtual double getPositionCommandRaw() { return positionCommand_deviceUnits + positionOffset_positionUnits; }
	
	double getFollowingError(){ return getPositionCommandRaw() - positionRaw_positionUnits; }
	double getFollowingErrorNormalized(){ return getFollowingError() / maxfollowingError; }
	
	double maxfollowingError = 0.0;
	
	
private:
	double positionCommand_deviceUnits;
	
	virtual void setVelocityCommand(double velocityCommand){}
};
