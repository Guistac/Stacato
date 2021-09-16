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
	bool hasError();
	void clearError();
	const char* getErrorString();

	bool b_detected = false;
	bool b_online = false;
	bool b_ready = false;
	bool b_error = false;
	bool b_clearError = false;
	char errorString[128];
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
	bool isParked() { b_parked; }

	bool b_enabled = false;
	bool b_parked = false;
	bool b_setEnabled = false;
	bool b_setDisabled = false;
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