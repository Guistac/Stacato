#pragma once

class DeviceNode;

enum class SubdeviceType {
	ACTUATOR,
	FEEDBACK,
	GPIO
};

class Subdevice {
public:

	Subdevice(const char* n) {
		setName(n);
	}

	char name[128];
	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }

	virtual SubdeviceType getSubdeviceType() = 0;

	std::shared_ptr<DeviceNode> parentDevice;
	void setParentDevice(std::shared_ptr<DeviceNode> pd) { parentDevice = pd; }
	
	bool isDetected();
	bool isOnline();
	bool isReady();

	bool b_detected = false;
	bool b_online = false;
	bool b_ready = false;
};


class ActuatorDevice : public Subdevice{
public:

	ActuatorDevice(const char* n) : Subdevice(n) {}

	virtual SubdeviceType getSubdeviceType() { return SubdeviceType::ACTUATOR; }

	void enable() { b_setEnabled = true; }
	void disable() { b_setDisabled = true; }
	void park() { b_setDisabled = true; b_parked = true; }
	void unpark() { b_parked = false; }

	bool isEnabled() { return b_enabled; }
	bool isParked() { return b_parked; }
	bool isEmergencyStopActive() { return b_emergencyStopActive; }

	double getLoad() { return load; }
	double getVelocityLimit() { return velocityLimit; }
	double getAccelerationLimit() { return accelerationLimit; }

	bool b_enabled = false;
	bool b_parked = false;
	bool b_emergencyStopActive = false;

	bool b_setEnabled = false;
	bool b_setDisabled = false;

	double velocityLimit;
	double accelerationLimit;
	double load;
};

class FeedbackDevice : public Subdevice {
public:

	FeedbackDevice(const char* n) : Subdevice(n) {}

	virtual SubdeviceType getSubdeviceType() { return SubdeviceType::FEEDBACK; }

	void reset() { b_reset = true; }

	bool isInRange() { b_inRange; }


	bool b_reset = false;
	bool b_inRange = true;
};

class GpioDevice : public Subdevice {
public:

	GpioDevice(const char* n) : Subdevice(n) {}

	virtual SubdeviceType getSubdeviceType() { return SubdeviceType::GPIO; }

};