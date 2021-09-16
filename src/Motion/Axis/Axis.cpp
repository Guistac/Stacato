#include <pch.h>

#include "Axis.h"

#include "NodeGraph/DeviceNode.h"

#include "Motion/MotionTypes.h"





void Axis::process() {
	double updateTime_seconds = Timing::getTime_seconds();
	double deltaT_seconds = updateTime_seconds - lastUpdateTime_seconds;

	if (b_enabled) {

		if (profileVelocity_degreesPerSecond != velocityControlTarget_degreesPerSecond) {
			double deltaV_degreesPerSecond;
			deltaV_degreesPerSecond = defaultMovementAcceleration_degreesPerSecondSquared * deltaT_seconds;
			if (profileVelocity_degreesPerSecond < velocityControlTarget_degreesPerSecond) {
				profileVelocity_degreesPerSecond += deltaV_degreesPerSecond;
				if (profileVelocity_degreesPerSecond > velocityControlTarget_degreesPerSecond) profileVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
			}
			else {
				profileVelocity_degreesPerSecond -= deltaV_degreesPerSecond;
				if (profileVelocity_degreesPerSecond < velocityControlTarget_degreesPerSecond) profileVelocity_degreesPerSecond = velocityControlTarget_degreesPerSecond;
			}
		}
		double deltaP_degrees = profileVelocity_degreesPerSecond * deltaT_seconds;

		profilePosition_degrees += deltaP_degrees;

		positionCommand->set(profilePosition_degrees);
	}
	else {
		positionCommand->set(positionFeedback->getLinks().front()->getInputData()->getReal());
	}

	lastUpdateTime_seconds = updateTime_seconds;
}




void Axis::enable() {
	Logger::info("Enabling axis {}", getName());
	bool canAxisBeEnabled = true;
	if (actuatorDeviceLinks->isConnected()) {
		for (auto link : actuatorDeviceLinks->getLinks()) {
			std::shared_ptr<ActuatorDevice> actuatorDevice = link->getInputData()->getActuatorDevice();
			if (!actuatorDevice->isEnabled()) {
				canAxisBeEnabled = false;
				Logger::warn("Actuator subdevice '{}' of device '{}' is not enabled", actuatorDevice->getName(), actuatorDevice->parentDevice->getName());
			}
		}
	}
	else {
		canAxisBeEnabled = false;
		Logger::warn("No Actuators are connected to axis '{}'", getName());
	}
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		if (feedbackDeviceLink->isConnected()) {
			std::shared_ptr<FeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getFeedbackDevice();
			if (!feedbackDevice->isReady()) {
				canAxisBeEnabled = false;
				Logger::warn("Position feedback subdevice '{}' of device '{}' is not ready", feedbackDevice->getName(), feedbackDevice->parentDevice->getName());
			}
		}
		else {
			canAxisBeEnabled = false;
			Logger::warn("No Position Feedback device is connected to axis '{}'", getName());
		}
	}
	if (positionReferenceType != PositionReference::Type::NO_LIMIT) {
		if (referenceDeviceLinks->isConnected()) {
			for (auto link : referenceDeviceLinks->getLinks()) {
				std::shared_ptr<GpioDevice> gpioDevice = link->getInputData()->getGpioDevice();
				if (!gpioDevice->isReady()) {
					canAxisBeEnabled = false;
					Logger::warn("Position reference subdevice '{}' of device '{}' is not ready", gpioDevice->getName(), gpioDevice->parentDevice->getName());
				}
			}
		}
		else {
			canAxisBeEnabled = false;
			Logger::warn("No Position reference device is connected to axis '{}'", getName());
		}
	}
	if (!canAxisBeEnabled) Logger::warn("Axis '{}' cannot be enabled", getName());
	else {
		onEnable();
		Logger::info("Axis '{}' was enabled", getName());
	}
}

void Axis::onEnable() {
	profilePosition_degrees = positionFeedback->getLinks().front()->getInputData()->getReal();
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecond = 0.0;
	b_enabled = true;
}

void Axis::disable() {
	b_enabled = false;
	Logger::info("Axis {} disabled", getName());
}

bool Axis::isEnabled() {
	return b_enabled;
}


bool Axis::areAllDevicesReady() {
	if (actuatorDeviceLinks->isConnected()) {
		for (auto link : actuatorDeviceLinks->getLinks()) {
			std::shared_ptr<ActuatorDevice> actuatorDevice = link->getInputData()->getActuatorDevice();
			if (!actuatorDevice->isEnabled()) return false;
		}
	}
	else return false;
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		if (feedbackDeviceLink->isConnected()) {
			std::shared_ptr<FeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getFeedbackDevice();
			if (!feedbackDevice->isReady()) return false;
		}
		else return false;
	}
	if (positionReferenceType != PositionReference::Type::NO_LIMIT) {
		if (referenceDeviceLinks->isConnected()) {
			for (auto link : referenceDeviceLinks->getLinks()) {
				std::shared_ptr<GpioDevice> gpioDevice = link->getInputData()->getGpioDevice();
				if (!gpioDevice->isReady()) return false;
			}
		}
		else return false;
	}
	return true;
}

void Axis::enableAllActuators() {
	for (auto link : actuatorDeviceLinks->getLinks()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = link->getInputData()->getActuatorDevice();
		actuatorDevice->enable();
	}
}

void Axis::disableAllActuators() {
	for (auto link : actuatorDeviceLinks->getLinks()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = link->getInputData()->getActuatorDevice();
		actuatorDevice->disable();
	}
}