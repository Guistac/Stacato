#include <pch.h>

#include "Axis.h"

#include "NodeGraph/DeviceNode.h"

#include "Motion/MotionTypes.h"

#include "Motion/MotionCurve.h"


#include "Fieldbus/EtherCatFieldbus.h"


void Axis::process() {

	//get devices
	std::vector<std::shared_ptr<ActuatorDevice>> actuators;
	for (auto& pin : actuatorDeviceLinks->getConnectedPins()) actuators.push_back(pin->getActuatorDevice());
	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice = nullptr;
	if (feedbackDeviceLink->isConnected()) positionFeedbackDevice = feedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
	std::vector<std::shared_ptr<GpioDevice>> referenceDevices;
	for (auto& pin : referenceDeviceLinks->getConnectedPins()) referenceDevices.push_back(pin->getGpioDevice());

	//handle device state transitions
	if (b_enabled) {
		for (auto actuator : actuators) {
			if (!actuator->b_enabled) disable();
		}
		if (!positionFeedbackDevice->b_ready) disable();
		for (auto referenceDevice : referenceDevices) {
			if (!referenceDevice->b_ready) disable();
		}
	}

	//TODO: the axis should get timing information from the actuator object
	currentProfilePointTime_seconds = EtherCatFieldbus::getReferenceClock_seconds();
	currentProfilePointDeltaT_seconds = currentProfilePointTime_seconds - previousProfilePointTime_seconds;
	previousProfilePointTime_seconds = currentProfilePointTime_seconds;

	if (b_enabled) {
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET: velocityTargetControl(); break;
			case ControlMode::POSITION_TARGET: positionTargetControl(); break;
			case ControlMode::FOLLOW_CURVE: followCurveControl(); break;
			case ControlMode::HOMING: homingControl(); break;
		}
		actuatorCommand->set(profilePosition_degrees);
	}
	else {
		actuatorCommand->set(positionFeedback->getLinks().front()->getInputData()->getReal());
	}

	std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
	std::shared_ptr<ActuatorDevice> actuatorDevice = actuatorDeviceLinks->getLinks().front()->getInputData()->getActuatorDevice();

	double actualPosition = feedbackDevice->getPosition();
	double positionError = profilePosition_degrees - feedbackDevice->getPosition();

	positionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profilePosition_degrees));
	actualPositionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualPosition));
	positionErrorHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, positionError));
	velocityHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileVelocity_degreesPerSecond));
	accelerationHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileAcceleration_degreesPerSecondSquared));
	loadHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actuatorDevice->getLoad()));
}




void Axis::enable() {
	std::thread axisEnabler([this]() {
		//enable devices
		enableAllActuators();
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		//wait for devices to be enabled
		while (std::chrono::system_clock::now() - start < std::chrono::milliseconds(500)) {
			if (areAllDevicesEnabled()) break;
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		//start axis or return feedback about failure mode
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
				std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
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
	});
	axisEnabler.detach();
}

void Axis::onEnable() {
	profilePosition_degrees = positionFeedback->getLinks().front()->getInputData()->getReal();
	targetCurveProfile = MotionCurve::CurveProfile();
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecondSquared = 0.0;
	b_enabled = true;
}

void Axis::disable() {
	b_enabled = false;
	onDisable();
	Logger::info("Axis {} disabled", getName());
}

void Axis::onDisable() {
	disableAllActuators();
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_degreesPerSecond = 0.0;
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecondSquared = 0.0;
}

bool Axis::isEnabled() {
	return b_enabled;
}

bool Axis::areAllDevicesReady() {
	if (actuatorDeviceLinks->isConnected()) {
		for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
			std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
			if (!actuatorDevice->isReady()) return false;
		}
	}
	else return false;
	if (positionFeedbackType != PositionFeedback::Type::NO_FEEDBACK) {
		if (feedbackDeviceLink->isConnected()) {
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
			if (!feedbackDevice->isReady()) return false;
		}
		else return false;
	}
	if (positionReferenceType != PositionReference::Type::NO_LIMIT) {
		if (referenceDeviceLinks->isConnected()) {
			for (auto pin : referenceDeviceLinks->getConnectedPins()) {
				std::shared_ptr<GpioDevice> gpioDevice = pin->getGpioDevice();
				if (!gpioDevice->isReady()) return false;
			}
		}
		else return false;
	}
	return true;
}

bool Axis::areAllDevicesEnabled() {
	if (!areAllDevicesReady()) return false;
	for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
		if (!actuatorDevice->isEnabled()) return false;
	}
	return true;
}

void Axis::enableAllActuators() {
	for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
		actuatorDevice->enable();
	}
}

void Axis::disableAllActuators() {
	for (auto pin : actuatorDeviceLinks->getConnectedPins()) {
		std::shared_ptr<ActuatorDevice> actuatorDevice = pin->getActuatorDevice();
		actuatorDevice->disable();
	}
}

//================================= MANUAL CONTROLS ===================================

void Axis::setVelocity(double velocity_axisUnits) {
	manualVelocityTarget_degreesPerSecond = velocity_axisUnits;
	if (controlMode == ControlMode::POSITION_TARGET) {
		targetCurveProfile = MotionCurve::CurveProfile();
	}
	controlMode = ControlMode::VELOCITY_TARGET;
}

void Axis::velocityTargetControl() {
	if (profileVelocity_degreesPerSecond != manualVelocityTarget_degreesPerSecond) {
		double deltaV_degreesPerSecond = manualControlAcceleration_degreesPerSecond * currentProfilePointDeltaT_seconds;
		if (profileVelocity_degreesPerSecond < manualVelocityTarget_degreesPerSecond) {
			profileVelocity_degreesPerSecond += deltaV_degreesPerSecond;
			profileAcceleration_degreesPerSecondSquared = manualControlAcceleration_degreesPerSecond;
			if (profileVelocity_degreesPerSecond > manualVelocityTarget_degreesPerSecond) profileVelocity_degreesPerSecond = manualVelocityTarget_degreesPerSecond;
		}
		else {
			profileVelocity_degreesPerSecond -= deltaV_degreesPerSecond;
			profileAcceleration_degreesPerSecondSquared = -manualControlAcceleration_degreesPerSecond;
			if (profileVelocity_degreesPerSecond < manualVelocityTarget_degreesPerSecond) profileVelocity_degreesPerSecond = manualVelocityTarget_degreesPerSecond;
		}
	}
	else profileAcceleration_degreesPerSecondSquared = 0.0;
	double deltaP_degrees = profileVelocity_degreesPerSecond * currentProfilePointDeltaT_seconds;
	profilePosition_degrees += deltaP_degrees;
}

void Axis::moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits) {
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_degrees, acceleration_axisUnits, profileVelocity_degreesPerSecond);
	MotionCurve::CurvePoint endPoint(0.0, position_axisUnits, acceleration_axisUnits, 0.0);
	if (MotionCurve::getFastestVelocityConstrainedProfile(startPoint, endPoint, velocity_axisUnits, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_degreesPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void Axis::moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits) {
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_degrees, acceleration_axisUnits, profileVelocity_degreesPerSecond);
	MotionCurve::CurvePoint endPoint(currentProfilePointTime_seconds + movementTime_seconds, position_axisUnits, acceleration_axisUnits, 0.0);
	if (MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, velocityLimit_degreesPerSecond, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_degreesPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void Axis::positionTargetControl() {
	if (MotionCurve::isInsideCurveTime(currentProfilePointTime_seconds, targetCurveProfile)) {
		MotionCurve::CurvePoint curvePoint = MotionCurve::getCurvePointAtTime(currentProfilePointTime_seconds, targetCurveProfile);
		profilePosition_degrees = curvePoint.position;
		profileVelocity_degreesPerSecond = curvePoint.velocity;
		profileAcceleration_degreesPerSecondSquared = curvePoint.acceleration;
	}
	else {
		setVelocity(0.0);
	}
}

//=================================================================

void Axis::followCurveControl() {}

void Axis::homingControl() {}