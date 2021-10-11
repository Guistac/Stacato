#include <pch.h>

#include "Axis.h"

#include "NodeGraph/DeviceNode.h"

#include "Motion/MotionTypes.h"

#include "Motion/MotionCurve.h"


#include "Fieldbus/EtherCatFieldbus.h"


void Axis::process() {

	//TODO: we should get device status and update axis status before processing motion

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
}

void Axis::onEnable() {
	profilePosition_degrees = positionFeedback->getLinks().front()->getInputData()->getReal();
	profileVelocity_degreesPerSecond = 0.0;
	profileAcceleration_degreesPerSecondSquared = 0.0;
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
			std::shared_ptr<PositionFeedbackDevice> feedbackDevice = feedbackDeviceLink->getLinks().front()->getInputData()->getPositionFeedbackDevice();
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

//================================= MANUAL CONTROLS ===================================

void Axis::setVelocity(double velocity_axisUnits) {
	manualVelocityTarget_degreesPerSecond = velocity_axisUnits;
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