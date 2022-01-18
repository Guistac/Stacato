#include <pch.h>

#include "VelocityControlledAxis.h"

#include "Motion/SubDevice.h"

void VelocityControlledAxis::initialize() {
	addIoData(actuatorDeviceLink);
	std::shared_ptr<VelocityControlledAxis> thisAxis = std::dynamic_pointer_cast<VelocityControlledAxis>(shared_from_this());
	velocityControlledAxisLink->assignData(thisAxis);
	addIoData(velocityControlledAxisLink);
	addIoData(velocity);
}

void VelocityControlledAxis::process() {
	if (!isAxisPinConnected()) {
		//manual controls are available
		sendActuatorCommands();
	}
}

void VelocityControlledAxis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isActuatorDeviceConnected()) output.push_back(getActuatorDevice()->parentDevice);
}

bool VelocityControlledAxis::isReady() {
	if (isActuatorDeviceConnected() && getActuatorDevice()->isReady()) return true;
	return false;
}

bool VelocityControlledAxis::isMoving() {
	return false;
	//return getActuatorDevice()->isMoving
}

void VelocityControlledAxis::enable() {
	if (!isReady()) return;
	std::thread axisEnabler([this]() {
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		actuator->enable();
		using namespace std::chrono;
		system_clock::time_point enableTime = system_clock::now();
		while (!actuator->isEnabled()) {
			if (system_clock::now() - enableTime > milliseconds(100)) {
				b_enabled = false;
				Logger::warn("Could not enable axis");
				return;
			}
		}
		b_enabled = true;
		Logger::info("Axis Enabled");
	});
	axisEnabler.detach();
}

void VelocityControlledAxis::disable() {
	getActuatorDevice()->disable();
	b_enabled = false;
}

void VelocityControlledAxis::onEnable() {}
void VelocityControlledAxis::onDisable() {}

void VelocityControlledAxis::sendActuatorCommands() {
	getActuatorDevice()->setCommand(profileVelocity_axisUnitsPerSecond);
}

void VelocityControlledAxis::setVelocityTarget(double velocity_axisUnitsPerSecond) {}
void VelocityControlledAxis::fastStop() {}
void VelocityControlledAxis::fastStopControl() {}
