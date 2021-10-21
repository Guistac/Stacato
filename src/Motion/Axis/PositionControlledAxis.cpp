#include <pch.h>

#include "PositionControlledAxis.h"
#include "NodeGraph/Device.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void PositionControlledAxis::assignIoData() {
	//inputs
	addIoData(actuatorDeviceLink);
	addIoData(servoActuatorDeviceLink);
	addIoData(positionFeedbackDeviceLink);
	addIoData(referenceDeviceLink);
	addIoData(lowLimitSignalPin);
	addIoData(highLimitSignalPin);
	addIoData(referenceSignalPin);
	//outputs
	//these pins are always present
	std::shared_ptr<Axis> thisAxis = std::dynamic_pointer_cast<Axis>(shared_from_this());
	axisLink->set(thisAxis);
	addIoData(axisLink);
	addIoData(position);
	addIoData(velocity);
	setPositionControlType(positionControl);
	setPositionReferenceSignalType(positionReferenceSignal);
}

void PositionControlledAxis::process() {

	//check connection requirements and abort processing if the requirements are not met
	bool needsFeedback = needsPositionFeedbackDevice();
	bool needsReference = needsReferenceDevice();
	bool needsActuator = needsActuatorDevice();
	bool needsServo = needsServoActuatorDevice();
	if (needsFeedback && !isPositionFeedbackDeviceConnected()) return;
	if (needsReference && !isReferenceDeviceConnected()) return;
	if (needsActuator && !isActuatorDeviceConnected()) return;
	if (needsServo && !isServoActuatorDeviceConnected()) return;

	//get devices
	std::shared_ptr<ActuatorDevice> actuatorDevice;
	std::shared_ptr<GpioDevice> referenceDevice;
	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice;
	std::shared_ptr<ServoActuatorDevice> servoActuatorDevice;
	if (needsFeedback) positionFeedbackDevice = getPositionFeedbackDevice();
	if (needsReference) referenceDevice = getReferenceDevice();
	if (needsActuator) actuatorDevice = getActuatorDevice();
	if (needsServo) servoActuatorDevice = getServoActuatorDevice();

	//get reference signals
	if (needsReference) updateReferenceSignals();

	//handle device state transitions
	if (b_enabled) {
		if (needsActuator && !actuatorDevice->b_enabled) disable();
		if (needsFeedback && !positionFeedbackDevice->b_ready) disable();
		if (needsReference && !referenceDevice->b_ready) disable();
		if (needsServo && (!servoActuatorDevice->b_ready || !servoActuatorDevice->b_enabled)) {
			if (!isHoming())	disable();
		}
	}

	if (needsFeedback) {
		//convert feedback units to axis units
		actualPosition_axisUnits = positionFeedbackDevice->getPosition() / feedbackUnitsPerAxisUnits;
		actualVelocity_axisUnitsPerSecond = positionFeedbackDevice->getVelocity() / feedbackUnitsPerAxisUnits;
	}
	else if (needsServo) {
		actualPosition_axisUnits = servoActuatorDevice->getPosition() / actuatorUnitsPerAxisUnits;
		actualVelocity_axisUnitsPerSecond = servoActuatorDevice->getVelocity() / actuatorUnitsPerAxisUnits;
	}

	if (needsActuator) currentProfilePointTime_seconds = actuatorDevice->getCommandRequestTime_seconds();
	else if (needsServo) currentProfilePointTime_seconds = servoActuatorDevice->getCommandRequestTime_seconds();
	currentProfilePointDeltaT_seconds = currentProfilePointTime_seconds - previousProfilePointTime_seconds;
	previousProfilePointTime_seconds = currentProfilePointTime_seconds;

	//if a machine is connected and machine control mode is enabled, we let the connected machine do its processing
	//otherwise the axis controls itself
	if (!axisLink->isConnected()) control();
}

void PositionControlledAxis::control() {
	//update profile generator
	if (b_enabled) {
		if (b_isHoming) homingControl();
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET:
				velocityTargetControl();
				break;
			case ControlMode::POSITION_TARGET:
				positionTargetControl();
				break;
		}
	}
	else {
		profilePosition_axisUnits = actualPosition_axisUnits;
		profileVelocity_axisUnitsPerSecond = actualVelocity_axisUnitsPerSecond;
	}
	position->set(actualPosition_axisUnits);
	velocity->set(actualVelocity_axisUnitsPerSecond);

	double actualLoad = 0.0;
	switch (positionControl) {
		case PositionControl::Type::SERVO:
			actualLoad = getServoActuatorDevice()->getLoad();
			getServoActuatorDevice()->setCommand(profilePosition_axisUnits * actuatorUnitsPerAxisUnits);
			break;
		case PositionControl::Type::CLOSED_LOOP:
			actualLoad = getActuatorDevice()->getLoad();
			//for closed loop, implement pid control with adjustable gains
			//getActuatorDevice()->setCommand(profileVelocity_axisUnitsPerSecond * actuatorUnitsPerAxisUnits);
			break;
	}
	positionError_axisUnits = profilePosition_axisUnits - actualPosition_axisUnits / feedbackUnitsPerAxisUnits;

	//update metrics
	positionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profilePosition_axisUnits));
	actualPositionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualPosition_axisUnits));
	positionErrorHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, positionError_axisUnits));
	velocityHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileVelocity_axisUnitsPerSecond));
	accelerationHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileAcceleration_axisUnitsPerSecondSquared));
	loadHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualLoad));
}

//=================================== STATE CONTROL ============================================

void PositionControlledAxis::enable() {
	std::thread machineEnabler([this]() {
		//enable actuator
		if (needsActuatorDevice()) getServoActuatorDevice()->enable();
		else if (needsServoActuatorDevice()) getServoActuatorDevice()->enable();
		//wait for actuator to be enabled
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		if (needsActuatorDevice()) {
			while (!getActuatorDevice()->isEnabled()) {
				if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) return Logger::warn("Could not enable Machine '{}', actuator did not enable on time", getName());
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		else if (needsServoActuatorDevice()) {
			while (!getServoActuatorDevice()->isEnabled()) {
				if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) return Logger::warn("Could not enable Machine '{}', servo actuator did not enable on time", getName());
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		if (!isReady()) Logger::warn("Machine '{}' cannot be enabled", getName());
		else onEnable();
	});
	machineEnabler.detach();
}

void PositionControlledAxis::onEnable() {
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_axisUnitsPerSecond = 0.0;
	profileVelocity_axisUnitsPerSecond = 0.0;
	profileAcceleration_axisUnitsPerSecondSquared = 0.0;
	b_enabled = true;
	Logger::info("Machine '{}' was enabled", getName());
}

void PositionControlledAxis::disable() {
	b_enabled = false;
	if (needsActuatorDevice()) getActuatorDevice()->disable();
	else if (needsServoActuatorDevice()) getServoActuatorDevice()->disable();
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_axisUnitsPerSecond = 0.0;
	profileVelocity_axisUnitsPerSecond = 0.0;
	profileAcceleration_axisUnitsPerSecondSquared = 0.0;
	Logger::info("Machine {} disabled", getName());
}

bool PositionControlledAxis::isEnabled() {
	return b_enabled;
}

bool PositionControlledAxis::isReady() {
	if (needsActuatorDevice() && (!isActuatorDeviceConnected() || !getActuatorDevice()->isReady())) return false;
	else if (needsServoActuatorDevice() && (!isServoActuatorDeviceConnected() || !getServoActuatorDevice()->isReady())) return false;
	if (needsPositionFeedbackDevice() && (!isPositionFeedbackDeviceConnected() || !getPositionFeedbackDevice()->isReady())) return false;
	if (needsReferenceDevice() && (!isReferenceDeviceConnected() || !getReferenceDevice()->isReady())) return false;
	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		if (!lowLimitSignalPin->isConnected()) return false;
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		if (!lowLimitSignalPin->isConnected()) return false;
		if (!highLimitSignalPin->isConnected()) return false;
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		if (!referenceSignalPin->isConnected()) return false;
		break;
	}
	return true;
}

//========================== DEVICES =============================

bool PositionControlledAxis::needsPositionFeedbackDevice() {
	switch (positionControl) {
	case PositionControl::Type::CLOSED_LOOP:
		return true;
	case PositionControl::Type::SERVO:
		return false;
	}
}

bool PositionControlledAxis::isPositionFeedbackDeviceConnected() {
	return positionFeedbackDeviceLink->isConnected();
}

std::shared_ptr<PositionFeedbackDevice> PositionControlledAxis::getPositionFeedbackDevice() {
	return positionFeedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
}

bool PositionControlledAxis::needsReferenceDevice() {
	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		return true;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		return false;
	}
}

bool PositionControlledAxis::isReferenceDeviceConnected() {
	return referenceDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> PositionControlledAxis::getReferenceDevice() {
	return referenceDeviceLink->getConnectedPins().front()->getGpioDevice();
}

bool PositionControlledAxis::needsActuatorDevice() {
	switch (positionControl) {
	case PositionControl::Type::CLOSED_LOOP:
		return true;
	case PositionControl::Type::SERVO:
		return false;
	}
}

bool PositionControlledAxis::isActuatorDeviceConnected() {
	return actuatorDeviceLink->isConnected();
}
std::shared_ptr<ActuatorDevice> PositionControlledAxis::getActuatorDevice() {
	return actuatorDeviceLink->getConnectedPins().front()->getActuatorDevice();
}

bool PositionControlledAxis::needsServoActuatorDevice() {
	switch (positionControl) {
	case PositionControl::Type::CLOSED_LOOP:
		return false;
	case PositionControl::Type::SERVO:
		return true;
	}
}

bool PositionControlledAxis::isServoActuatorDeviceConnected() {
	return servoActuatorDeviceLink->isConnected();
}

std::shared_ptr<ServoActuatorDevice> PositionControlledAxis::getServoActuatorDevice() {
	return servoActuatorDeviceLink->getConnectedPins().front()->getServoActuatorDevice();
}



//============================= DEVICE AND SIGNAL LINKS ================================

void PositionControlledAxis::setPositionControlType(PositionControl::Type type) {
	switch (type) {
	case PositionControl::Type::CLOSED_LOOP:
		actuatorDeviceLink->setVisible(true);
		positionFeedbackDeviceLink->setVisible(true);
		servoActuatorDeviceLink->disconnectAllLinks();
		servoActuatorDeviceLink->setVisible(false);
		break;
	case PositionControl::Type::SERVO:
		actuatorDeviceLink->disconnectAllLinks();
		actuatorDeviceLink->setVisible(false);
		positionFeedbackDeviceLink->disconnectAllLinks();
		positionFeedbackDeviceLink->setVisible(false);
		servoActuatorDeviceLink->setVisible(true);
		feedbackUnitsPerAxisUnits = actuatorUnitsPerAxisUnits;
		break;
	}
	positionControl = type;
}

void PositionControlledAxis::setPositionReferenceSignalType(PositionReferenceSignal::Type type) {
	switch (type) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		referenceDeviceLink->setVisible(true);
		lowLimitSignalPin->setVisible(true);
		highLimitSignalPin->disconnectAllLinks();
		highLimitSignalPin->setVisible(false);
		referenceSignalPin->disconnectAllLinks();
		referenceSignalPin->setVisible(false);
		maxNegativeDeviation_axisUnits = 0.0;
		enableNegativeLimit = true;
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		referenceDeviceLink->setVisible(true);
		lowLimitSignalPin->setVisible(true);
		highLimitSignalPin->setVisible(true);
		referenceSignalPin->disconnectAllLinks();
		referenceSignalPin->setVisible(false);
		maxNegativeDeviation_axisUnits = 0.0;
		enableNegativeLimit = true;
		enablePositiveLimit = true;
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		referenceDeviceLink->setVisible(true);
		lowLimitSignalPin->disconnectAllLinks();
		lowLimitSignalPin->setVisible(false);
		highLimitSignalPin->disconnectAllLinks();
		highLimitSignalPin->setVisible(false);
		referenceSignalPin->setVisible(true);
		break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		referenceDeviceLink->disconnectAllLinks();
		referenceDeviceLink->setVisible(false);
		lowLimitSignalPin->disconnectAllLinks();
		lowLimitSignalPin->setVisible(false);
		highLimitSignalPin->disconnectAllLinks();
		highLimitSignalPin->setVisible(false);
		referenceSignalPin->disconnectAllLinks();
		referenceSignalPin->setVisible(false);
		break;
	}
	positionReferenceSignal = type;
}




//============================== LIMITS, ORIGIN AND REFERENCES ================================

void PositionControlledAxis::updateReferenceSignals() {
	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		previousLowLimitSignal = lowLimitSignal;
		if (lowLimitSignalPin->isConnected()) lowLimitSignalPin->set(lowLimitSignalPin->getConnectedPins().front()->getBoolean());
		lowLimitSignal = lowLimitSignalPin->getBoolean();
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		previousLowLimitSignal = lowLimitSignal;
		if (lowLimitSignalPin->isConnected()) lowLimitSignalPin->set(lowLimitSignalPin->getConnectedPins().front()->getBoolean());
		lowLimitSignal = lowLimitSignalPin->getBoolean();
		previousHighLimitSignal = highLimitSignal;
		if (highLimitSignalPin->isConnected()) highLimitSignalPin->set(highLimitSignalPin->getConnectedPins().front()->getBoolean());
		highLimitSignal = highLimitSignalPin->getBoolean();
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		previousReferenceSignal = referenceSignal;
		if (referenceSignalPin->isConnected()) referenceSignalPin->set(referenceSignalPin->getConnectedPins().front()->getBoolean());
		referenceSignal = referenceSignalPin->getBoolean();
		break;
	default: break;
	}
}

bool PositionControlledAxis::isMoving() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		return getServoActuatorDevice()->isMoving();
		break;
	case PositionControl::Type::CLOSED_LOOP:
		getPositionFeedbackDevice()->isMoving();
		break;
	}
}

double PositionControlledAxis::getPositionProgress() {
	double lowLimit = getLowPositionLimit();
	double highLimit = getHighPositionLimit();
	double current = actualPosition_axisUnits;
	return (current - lowLimit) / (highLimit - lowLimit);
}


double PositionControlledAxis::getLowPositionLimit() {
	double lowLimit = -std::numeric_limits<double>::infinity();
	if (enableNegativeLimit) lowLimit = maxNegativeDeviation_axisUnits;
	if (limitToFeedbackWorkingRange) {
		double lowFeedbackLimit = getLowFeedbackPositionLimit();
		if (lowLimit < lowFeedbackLimit) lowLimit = lowFeedbackLimit;
	}
	return lowLimit;
}

double PositionControlledAxis::getHighPositionLimit() {
	double highLimit = std::numeric_limits<double>::infinity();
	if (enablePositiveLimit) highLimit = maxPositiveDeviation_axisUnits;
	if (limitToFeedbackWorkingRange) {
		double highFeedbackLimit = getHighFeedbackPositionLimit();
		if (highLimit > highFeedbackLimit) highLimit = highFeedbackLimit;
	}
	return highLimit;
}

double PositionControlledAxis::getLowFeedbackPositionLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMinPosition() / feedbackUnitsPerAxisUnits;
		else return -std::numeric_limits<double>::infinity();
	case PositionControl::Type::CLOSED_LOOP:
		if (isPositionFeedbackDeviceConnected()) return getPositionFeedbackDevice()->getMinPosition() / feedbackUnitsPerAxisUnits;
		else return -std::numeric_limits<double>::infinity();
	}
}

double PositionControlledAxis::getHighFeedbackPositionLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMaxPosition() / feedbackUnitsPerAxisUnits;
		else return std::numeric_limits<double>::infinity();
	case PositionControl::Type::CLOSED_LOOP:
		if (isPositionFeedbackDeviceConnected()) return getPositionFeedbackDevice()->getMaxPosition() / feedbackUnitsPerAxisUnits;
		else return std::numeric_limits<double>::infinity();
	}
}


void PositionControlledAxis::setCurrentPosition(double distance) {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		getServoActuatorDevice()->setPosition(distance * feedbackUnitsPerAxisUnits);
		break;
	case PositionControl::Type::CLOSED_LOOP:
		getPositionFeedbackDevice()->setPosition(distance * feedbackUnitsPerAxisUnits);
		break;
	}
	profilePosition_axisUnits = distance;
}

void PositionControlledAxis::setCurrentPositionAsNegativeLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		maxNegativeDeviation_axisUnits = getServoActuatorDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	case PositionControl::Type::CLOSED_LOOP:
		maxNegativeDeviation_axisUnits = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	}
}

void PositionControlledAxis::setCurrentPositionAsPositiveLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		maxPositiveDeviation_axisUnits = getServoActuatorDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	case PositionControl::Type::CLOSED_LOOP:
		maxPositiveDeviation_axisUnits = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	}
}

void PositionControlledAxis::scaleFeedbackToMatchPosition(double position_axisUnits) {
	double feedbackDevicePosition_feedbackUnits;
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		feedbackDevicePosition_feedbackUnits = getServoActuatorDevice()->getPosition();
		feedbackUnitsPerAxisUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
		actuatorUnitsPerAxisUnits = feedbackUnitsPerAxisUnits;
		break;
	case PositionControl::Type::CLOSED_LOOP:
		feedbackDevicePosition_feedbackUnits = getPositionFeedbackDevice()->getPosition();
		feedbackUnitsPerAxisUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
		break;
	}
	profilePosition_axisUnits = feedbackDevicePosition_feedbackUnits / feedbackUnitsPerAxisUnits;
}



//================================= VELOCITY TARGET CONTROL ===================================

void PositionControlledAxis::setVelocity(double velocity_axisUnits) {
	manualVelocityTarget_axisUnitsPerSecond = velocity_axisUnits;
	if (controlMode == ControlMode::POSITION_TARGET) {
		targetCurveProfile = MotionCurve::CurveProfile();
	}
	controlMode = ControlMode::VELOCITY_TARGET;
}

void PositionControlledAxis::velocityTargetControl() {
	if (profileVelocity_axisUnitsPerSecond != manualVelocityTarget_axisUnitsPerSecond) {
		double deltaV_axisUnitsPerSecond = manualControlAcceleration_axisUnitsPerSecond * currentProfilePointDeltaT_seconds;
		if (profileVelocity_axisUnitsPerSecond < manualVelocityTarget_axisUnitsPerSecond) {
			profileVelocity_axisUnitsPerSecond += deltaV_axisUnitsPerSecond;
			profileAcceleration_axisUnitsPerSecondSquared = manualControlAcceleration_axisUnitsPerSecond;
			if (profileVelocity_axisUnitsPerSecond > manualVelocityTarget_axisUnitsPerSecond) profileVelocity_axisUnitsPerSecond = manualVelocityTarget_axisUnitsPerSecond;
		}
		else {
			profileVelocity_axisUnitsPerSecond -= deltaV_axisUnitsPerSecond;
			profileAcceleration_axisUnitsPerSecondSquared = -manualControlAcceleration_axisUnitsPerSecond;
			if (profileVelocity_axisUnitsPerSecond < manualVelocityTarget_axisUnitsPerSecond) profileVelocity_axisUnitsPerSecond = manualVelocityTarget_axisUnitsPerSecond;
		}
	}
	else profileAcceleration_axisUnitsPerSecondSquared = 0.0;
	double deltaP_axisUnits = profileVelocity_axisUnitsPerSecond * currentProfilePointDeltaT_seconds;
	profilePosition_axisUnits += deltaP_axisUnits;
}



//================================= POSITION TARGET CONTROL ===================================

void PositionControlledAxis::moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits) {
	if (position_axisUnits > maxPositiveDeviation_axisUnits && enablePositiveLimit) position_axisUnits = maxPositiveDeviation_axisUnits;
	else if (position_axisUnits < maxNegativeDeviation_axisUnits && enableNegativeLimit) position_axisUnits = maxNegativeDeviation_axisUnits;
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_axisUnits, acceleration_axisUnits, profileVelocity_axisUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(0.0, position_axisUnits, acceleration_axisUnits, 0.0);
	if (MotionCurve::getFastestVelocityConstrainedProfile(startPoint, endPoint, velocity_axisUnits, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_axisUnitsPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void PositionControlledAxis::moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits) {
	if (position_axisUnits > maxPositiveDeviation_axisUnits && enablePositiveLimit) position_axisUnits = maxPositiveDeviation_axisUnits;
	else if (position_axisUnits < maxNegativeDeviation_axisUnits && enableNegativeLimit) position_axisUnits = maxNegativeDeviation_axisUnits;
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_axisUnits, acceleration_axisUnits, profileVelocity_axisUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(currentProfilePointTime_seconds + movementTime_seconds, position_axisUnits, acceleration_axisUnits, 0.0);
	if (MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, velocityLimit_axisUnitsPerSecond, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_axisUnitsPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void PositionControlledAxis::positionTargetControl() {
	if (MotionCurve::isInsideCurveTime(currentProfilePointTime_seconds, targetCurveProfile)) {
		MotionCurve::CurvePoint curvePoint = MotionCurve::getCurvePointAtTime(currentProfilePointTime_seconds, targetCurveProfile);
		profilePosition_axisUnits = curvePoint.position;
		profileVelocity_axisUnitsPerSecond = curvePoint.velocity;
		profileAcceleration_axisUnitsPerSecondSquared = curvePoint.acceleration;
	}
	else {
		profileVelocity_axisUnitsPerSecond = 0.0;
		profileAcceleration_axisUnitsPerSecondSquared = 0.0;
	}
}



//=================================================================


void PositionControlledAxis::followCurveControl() {}


//==================================== HOMING =====================================

void PositionControlledAxis::startHoming() {
	b_isHoming = true;
	homingStep = Homing::Step::NOT_STARTED;
	homingError = Homing::Error::NONE;
}

void PositionControlledAxis::cancelHoming() {
	b_isHoming = false;
	homingStep = Homing::Step::NOT_STARTED;
	controlMode = ControlMode::VELOCITY_TARGET;
	setVelocity(0.0);
}

bool PositionControlledAxis::isHoming() {
	return b_isHoming;
}

bool PositionControlledAxis::didHomingSucceed() {
	return !isHoming() && homingStep == Homing::Step::FINISHED;
}

bool PositionControlledAxis::didHomingFail() {
	return homingError != Homing::Error::NONE;
}

void PositionControlledAxis::onHomingSuccess() {
	b_isHoming = false;
	homingStep = Homing::Step::FINISHED;
}

void PositionControlledAxis::onHomingError() {
	b_isHoming = false;
	homingStep = Homing::Step::NOT_STARTED;
	disable();
}

void PositionControlledAxis::homingControl() {
	using namespace Homing;

	switch (positionReferenceSignal) {

		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:

			switch (homingStep) {
			case Step::NOT_STARTED:
				homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
				break;
			case Step::SEARCHING_LOW_LIMIT_COARSE:
				setVelocity(-homingVelocity_axisUnitsPerSecond);
				if (!previousLowLimitSignal && lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_COARSE;
				break;
			case Step::FOUND_LOW_LIMIT_COARSE:
				setVelocity(0.0);
				if (!isMoving()) {
					if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
						getServoActuatorDevice()->enable();
						profilePosition_axisUnits = actualPosition_axisUnits;
						profileVelocity_axisUnitsPerSecond = 0.0;
						profileAcceleration_axisUnitsPerSecondSquared = 0.0;
					}
					else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
				}
				break;
			case Step::SEARCHING_LOW_LIMIT_FINE:
				setVelocity(homingVelocity_axisUnitsPerSecond / 10.0);
				if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
				break;
			case Step::FOUND_LOW_LIMIT_FINE:
				setVelocity(0.0);
				if (!isMoving()) homingStep = Step::RESETTING_POSITION_FEEDBACK;
				break;
			case Step::RESETTING_POSITION_FEEDBACK:
				setCurrentPosition(0.0);
				onHomingSuccess();
				break;
			}
			break;

		case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:

			if (homingDirection == HomingDirection::Type::NEGATIVE) {

				switch (homingStep) {
				case Step::NOT_STARTED:
					homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case Step::SEARCHING_LOW_LIMIT_COARSE:
					setVelocity(-homingVelocity_axisUnitsPerSecond);
					if (!previousHighLimitSignal && highLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousLowLimitSignal && lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_COARSE;
					break;
				case Step::FOUND_LOW_LIMIT_COARSE:
					setVelocity(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_LOW_LIMIT_FINE:
					setVelocity(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
					break;
				case Step::FOUND_LOW_LIMIT_FINE:
					setVelocity(0.0);
					if (!isMoving()) homingStep = Step::RESETTING_POSITION_FEEDBACK;
					break;
				case Step::RESETTING_POSITION_FEEDBACK:
					setCurrentPosition(0.0);
					homingStep = Step::SEARCHING_HIGH_LIMIT_COARSE;
					break;
				case Step::SEARCHING_HIGH_LIMIT_COARSE:
					setVelocity(homingVelocity_axisUnitsPerSecond);
					if (!previousLowLimitSignal && lowLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousHighLimitSignal && highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_COARSE;
					break;
				case Step::FOUND_HIGH_LIMIT_COARSE:
					setVelocity(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_HIGH_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_HIGH_LIMIT_FINE:
					setVelocity(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousHighLimitSignal && !highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_FINE;
					break;
				case Step::FOUND_HIGH_LIMIT_FINE:
					setVelocity(0.0);
					if (!isMoving()) homingStep = Step::SETTING_HIGH_LIMIT;
					break;
				case Step::SETTING_HIGH_LIMIT:
					setCurrentPositionAsPositiveLimit();
					homingStep = Step::FINISHED;
					onHomingSuccess();
					break;
				}
				break;

			}
			else if (homingDirection == HomingDirection::Type::POSITIVE) {


				switch (homingStep) {
				case Step::NOT_STARTED:
					homingStep = Step::SEARCHING_HIGH_LIMIT_COARSE;
					break;
				case Step::SEARCHING_HIGH_LIMIT_COARSE:
					setVelocity(homingVelocity_axisUnitsPerSecond);
					if (!previousLowLimitSignal && lowLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousHighLimitSignal && highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_COARSE;
					break;
				case Step::FOUND_HIGH_LIMIT_COARSE:
					setVelocity(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_HIGH_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_HIGH_LIMIT_FINE:
					setVelocity(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousHighLimitSignal && !highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_FINE;
					break;
				case Step::FOUND_HIGH_LIMIT_FINE:
					setVelocity(0.0);
					if (!isMoving()) homingStep = Step::SETTING_HIGH_LIMIT;
					break;
				case Step::SETTING_HIGH_LIMIT:
					setCurrentPosition(0.0);
					homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case Step::SEARCHING_LOW_LIMIT_COARSE:
					setVelocity(-homingVelocity_axisUnitsPerSecond);
					if (!previousHighLimitSignal && highLimitSignal) {
						homingError = Error::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousLowLimitSignal && lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_COARSE;
					break;
				case Step::FOUND_LOW_LIMIT_COARSE:
					setVelocity(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case Step::SEARCHING_LOW_LIMIT_FINE:
					setVelocity(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
					break;
				case Step::FOUND_LOW_LIMIT_FINE:
					setVelocity(0.0);
					if (!isMoving()) homingStep = Step::RESETTING_POSITION_FEEDBACK;
					break;
				case Step::RESETTING_POSITION_FEEDBACK:
					maxPositiveDeviation_axisUnits = std::abs(actualPosition_axisUnits);
					setCurrentPosition(0.0);
					homingStep = Step::FINISHED;
					onHomingSuccess();
					break;
				}
				break;

			}

		case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:

			if (homingDirection == HomingDirection::Type::POSITIVE) {

				switch (homingStep) {
				case Step::NOT_STARTED:
					homingStep = Step::SEARCHING_REFERENCE_FROM_BELOW_COARSE;
					break;
				case Step::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
					setVelocity(homingVelocity_axisUnitsPerSecond);
					if (!previousReferenceSignal && referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_BELOW_COARSE;
					break;
				case Step::FOUND_REFERENCE_FROM_BELOW_COARSE:
					setVelocity(0.0);
					if (!isMoving()) homingStep = Step::SEARCHING_REFERENCE_FROM_BELOW_FINE;
					break;
				case Step::SEARCHING_REFERENCE_FROM_BELOW_FINE:
					setVelocity(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_BELOW_FINE;
					break;
				case Step::FOUND_REFERENCE_FROM_BELOW_FINE:
					setVelocity(0.0);
					if (!isMoving()) {
						setCurrentPosition(0.0);
						homingStep = Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
					}
					break;
				case Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
					setVelocity(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_ABOVE_FINE;
					break;
				case Step::FOUND_REFERENCE_FROM_ABOVE_FINE:
					setVelocity(0.0);
					if (!isMoving()) {
						setCurrentPosition(actualPosition_axisUnits / 2.0);
						moveToPositionInTime(0.0, 0.0, 1.0);
						onHomingSuccess();
					}
					break;
				}

			}
			else if (homingDirection == HomingDirection::Type::NEGATIVE) {



				switch (homingStep) {
				case Step::NOT_STARTED:
					homingStep = Step::SEARCHING_REFERENCE_FROM_ABOVE_COARSE;
					break;
				case Step::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
					setVelocity(-homingVelocity_axisUnitsPerSecond);
					if (!previousReferenceSignal && referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_ABOVE_COARSE;
					break;
				case Step::FOUND_REFERENCE_FROM_ABOVE_COARSE:
					setVelocity(0.0);
					if (!isMoving()) homingStep = Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
					break;
				case Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
					setVelocity(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_ABOVE_FINE;
					break;
				case Step::FOUND_REFERENCE_FROM_ABOVE_FINE:
					setVelocity(0.0);
					if (!isMoving()) {
						setCurrentPosition(0.0);
						homingStep = Step::SEARCHING_REFERENCE_FROM_BELOW_FINE;
					}
					break;
				case Step::SEARCHING_REFERENCE_FROM_BELOW_FINE:
					setVelocity(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_BELOW_FINE;
					break;
				case Step::FOUND_REFERENCE_FROM_BELOW_FINE:
					setVelocity(0.0);
					if (!isMoving()) {
						setCurrentPosition(actualPosition_axisUnits / 2.0);
						moveToPositionInTime(0.0, 0.0, 1.0);
						onHomingSuccess();
					}
					break;
				}

			}
			break;

		default:
			onHomingError(); //homing should not be started for modes that don't support homing
			break;
	}
}


//==================================== SAVING AND LOADING =========================================


bool PositionControlledAxis::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;

	XMLElement* machineXML = xml->InsertNewChildElement("Machine");
	machineXML->SetAttribute("UnitType", getPositionUnitType(axisPositionUnitType)->saveName);
	machineXML->SetAttribute("Unit", getPositionUnit(axisPositionUnit)->saveName);
	machineXML->SetAttribute("PositionControl", getPositionControlType(positionControl)->saveName);

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("FeedbackUnitsPerMachineUnit", feedbackUnitsPerAxisUnits);
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", actuatorUnitsPerAxisUnits);
	unitConversionXML->SetAttribute("FeedbackAndActuatorConversionIdentical", feedbackAndActuatorConversionIdentical);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_axisUnitsPerSecond", velocityLimit_axisUnitsPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_axisUnitsPerSecondSquared", accelerationLimit_axisUnitsPerSecondSquared);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReferenceSignals");
	positionReferenceXML->SetAttribute("Type", getPositionReferenceSignal(positionReferenceSignal)->saveName);

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		positionReferenceXML->SetAttribute("HomingVelocity_axisUnitsPerSecond", homingVelocity_axisUnitsPerSecond);
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		positionReferenceXML->SetAttribute("HomingVelocity_axisUnitsPerSecond", homingVelocity_axisUnitsPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirection(homingDirection)->saveName);
		break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		break;
	}

	XMLElement* positionLimitsXML = xml->InsertNewChildElement("PositionLimits");
	positionLimitsXML->SetAttribute("MaxNegativeDeviation_axisUnits", maxNegativeDeviation_axisUnits);
	positionLimitsXML->SetAttribute("EnableNegativeLimit", enableNegativeLimit);
	positionLimitsXML->SetAttribute("MaxPositionDeviation_axisUnits", maxPositiveDeviation_axisUnits);
	positionLimitsXML->SetAttribute("EnablePositiveLimit", enablePositiveLimit);
	positionLimitsXML->SetAttribute("LimitToFeedbackWorkingRange", limitToFeedbackWorkingRange);

	XMLElement* defaultManualParametersXML = xml->InsertNewChildElement("DefaultManualParameters");
	defaultManualParametersXML->SetAttribute("Velocity_axisUnitsPerSecond", defaultManualVelocity_axisUnitsPerSecond);
	defaultManualParametersXML->SetAttribute("Acceleration_axisUnitsPerSecondSquared", defaultManualAcceleration_axisUnitsPerSecondSquared);

	return false;
}



bool PositionControlledAxis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* machineXML = xml->FirstChildElement("Machine");
	if (!machineXML) return Logger::warn("Could not load Machine Attributes");
	const char* machineUnitTypeString;
	if (machineXML->QueryStringAttribute("UnitType", &machineUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit Type");
	if (getPositionUnitType(machineUnitTypeString) == nullptr) return Logger::warn("Could not read Machine Unit Type");
	axisPositionUnitType = getPositionUnitType(machineUnitTypeString)->type;
	const char* machineUnitString;
	if (machineXML->QueryStringAttribute("Unit", &machineUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (getPositionUnit(machineUnitString) == nullptr) return Logger::warn("Could not read Machine Unit");
	axisPositionUnit = getPositionUnit(machineUnitString)->unit;
	const char* motionControlTypeString;
	if (machineXML->QueryStringAttribute("PositionControl", &motionControlTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Control Type");
	if (getPositionControlType(motionControlTypeString) == nullptr) return Logger::warn("Could not read Position Control Type");
	setPositionControlType(getPositionControlType(motionControlTypeString)->type);

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("FeedbackUnitsPerMachineUnit", &feedbackUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerMachineUnit", &actuatorUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Machine Unit");
	if (unitConversionXML->QueryBoolAttribute("FeedbackAndActuatorConversionIdentical", &feedbackAndActuatorConversionIdentical) != XML_SUCCESS) return Logger::warn("Could not load FeedbackAndActuatorConversionIdentical");

	XMLElement* positionLimitsXML = xml->FirstChildElement("PositionLimits");
	if (positionLimitsXML == nullptr) return Logger::warn("Could not load Position Limits Attribute");
	if (positionLimitsXML->QueryDoubleAttribute("MaxNegativeDeviation_axisUnits", &maxNegativeDeviation_axisUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation attribute");
	if (positionLimitsXML->QueryBoolAttribute("EnableNegativeLimit", &enableNegativeLimit) != XML_SUCCESS) return Logger::warn("could not load enable negative limit attribute");
	if (positionLimitsXML->QueryDoubleAttribute("MaxPositionDeviation_axisUnits", &maxPositiveDeviation_axisUnits) != XML_SUCCESS) return Logger::warn("could not load max positive deviation attribute");
	if (positionLimitsXML->QueryBoolAttribute("EnablePositiveLimit", &enablePositiveLimit) != XML_SUCCESS) return Logger::warn("Could not load enable positive limit attribute");
	if (positionLimitsXML->QueryBoolAttribute("LimitToFeedbackWorkingRange", &limitToFeedbackWorkingRange) != XML_SUCCESS) return Logger::warn("Could not load limit to feedback working range attribute");


	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReferenceSignals");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
	const char* positionLimitTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionLimitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (getPositionReferenceSignal(positionLimitTypeString) == nullptr) return Logger::warn("Could not read Position Reference Type");
	setPositionReferenceSignalType(getPositionReferenceSignal(positionLimitTypeString)->type);

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_axisUnitsPerSecond", &homingVelocity_axisUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN: {
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_axisUnitsPerSecond", &homingVelocity_axisUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		const char* homingDirectionString;
		if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
		if (getHomingDirection(homingDirectionString) == nullptr) return Logger::warn("Could not read homing direction");
		homingDirection = getHomingDirection(homingDirectionString)->type;
	}break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		break;
	}

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Machine Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_axisUnitsPerSecond", &velocityLimit_axisUnitsPerSecond)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_axisUnitsPerSecondSquared", &accelerationLimit_axisUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");

	XMLElement* defaultManualParametersXML = xml->FirstChildElement("DefaultManualParameters");
	if (!defaultManualParametersXML) return Logger::warn("Could not load default movement parameters");
	if (defaultManualParametersXML->QueryDoubleAttribute("Velocity_axisUnitsPerSecond", &defaultManualVelocity_axisUnitsPerSecond) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultManualParametersXML->QueryDoubleAttribute("Acceleration_axisUnitsPerSecondSquared", &defaultManualAcceleration_axisUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	manualControlAcceleration_axisUnitsPerSecond = defaultManualAcceleration_axisUnitsPerSecondSquared;
	targetVelocity_axisUnitsPerSecond = defaultManualVelocity_axisUnitsPerSecond;

	return true;
}
