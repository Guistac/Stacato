#pragma once

#include "MotionTypes.h"

class DeviceNode;

class Subdevice {
public:

	Subdevice(const char* n) { setName(n); }

	enum class Type {
		ACTUATOR,
		POSITION_FEEDBACK,
		GPIO
	};
	virtual Type getSubdeviceType() = 0;

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	char name[128];
	
	//set the parent device, which will provide further information on device status
	void setParentDevice(std::shared_ptr<DeviceNode> pd) { parentDevice = pd; }
	std::shared_ptr<DeviceNode> parentDevice;

	//has the device been discovered / seen
	bool isDetected();
	//is the device actively communicating
	bool isOnline();
	//is the device ready to provide inputs and accept ouputs
	bool isReady();

	bool b_detected = false;
	bool b_online = false;
	bool b_ready = false;
};


class ActuatorDevice : public Subdevice{
public:

	ActuatorDevice(const char* n, PositionUnit::Unit unit) : Subdevice(n), positionUnit(unit) {}
	virtual Type getSubdeviceType() { return Type::ACTUATOR; }

	PositionUnit::Unit getPositionUnit() { return positionUnit; }
	PositionUnit::Unit positionUnit;

	//enable power
	void enable() { b_setEnabled = true; }						
	//disable power
	void disable() { b_setDisabled = true; }						
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

	//get the normalized load of the device
	double getLoad() { return load; }								
	//get the velocity limit in device position units per second
	double getVelocityLimit() { return velocityLimit_positionUnitsPerSecond; }
	//get the acceleration limit in device position units per second squared
	double getAccelerationLimit() { return accelerationLimit_positionUnitsPerSecondSquared; }

	bool b_enabled = false;
	bool b_parked = false;
	bool b_emergencyStopActive = false;

	bool b_setEnabled = false;
	bool b_setDisabled = false;
	bool b_setQuickstop = false;

	double velocityLimit_positionUnitsPerSecond;
	double accelerationLimit_positionUnitsPerSecondSquared;
	double load;
};

class PositionFeedbackDevice : public Subdevice {
public:

	PositionFeedbackDevice(const char* n, PositionUnit::Unit unit) : Subdevice(n), positionUnit(unit) {}
	virtual Type getSubdeviceType() { return Type::POSITION_FEEDBACK; }

	PositionUnit::Unit getPositionUnit() { return positionUnit; }
	PositionUnit::Unit positionUnit;

	//set the current position of the actuator as the offset, this will zero the position
	void captureOffset() { positionOffset_positionUnits = positionRaw_positionUnits; }
	//is the position feedback inside its operational range
	bool isInRange() { return positionRaw_positionUnits < rangeMax_positionUnits&& positionRaw_positionUnits > rangeMin_positionUnits; }
	//get the normalized position in the operational range of the feedback device
	double getPositionInRange() { return (positionRaw_positionUnits - rangeMin_positionUnits) / (rangeMax_positionUnits - rangeMin_positionUnits); }
	//get the position with included offset in specified units
	double getPosition() { return positionRaw_positionUnits - positionOffset_positionUnits; }

	double positionRaw_positionUnits;
	double positionOffset_positionUnits = 0.0;

	double rangeMax_positionUnits;
	double rangeMin_positionUnits;
};

class GpioDevice : public Subdevice {
public:

	GpioDevice(const char* n) : Subdevice(n) {}
	virtual Type getSubdeviceType() { return Type::GPIO; }

};