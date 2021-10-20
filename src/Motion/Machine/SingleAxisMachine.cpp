#include <pch.h>

#include "SingleAxisMachine.h"
#include "NodeGraph/Device.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

void SingleAxisMachine::process() {

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
		actualPosition_machineUnits = positionFeedbackDevice->getPosition() / feedbackUnitsPerMachineUnits;
		actualVelocity_machineUnitsPerSecond = positionFeedbackDevice->getVelocity() / feedbackUnitsPerMachineUnits;
	}
	else if (needsServo) {
		actualPosition_machineUnits = servoActuatorDevice->getPosition() / actuatorUnitsPerMachineUnits;
		actualVelocity_machineUnitsPerSecond = servoActuatorDevice->getVelocity() / actuatorUnitsPerMachineUnits;
	}

	//update timing
	//TODO: the machine should get timing information from the actuator object or servo actuator object

	//if a machine is connected and machine control mode is enabled, we let the machine do its processing
	//else the axis controls itself

	currentProfilePointTime_seconds = EtherCatFieldbus::getReferenceClock_seconds();
	currentProfilePointDeltaT_seconds = currentProfilePointTime_seconds - previousProfilePointTime_seconds;
	previousProfilePointTime_seconds = currentProfilePointTime_seconds;

	//update profile generator
	if (b_enabled) {
		if (b_isHoming) homingControl();
		switch (controlMode) {
		case ControlMode::VELOCITY_TARGET: velocityTargetControl(); break;
		case ControlMode::POSITION_TARGET: positionTargetControl(); break;
		case ControlMode::FOLLOW_CURVE: followCurveControl(); break;
		}
	}
	else if (needsFeedback || needsServo) {
		//if the axis is disabled, copy position feedback data to profile generator
		profilePosition_machineUnits = actualPosition_machineUnits;
		profileVelocity_machineUnitsPerSecond = actualVelocity_machineUnitsPerSecond;
	}

	switch (positionControl) {
	case PositionControl::Type::SERVO:
		servoActuatorDevice->setCommand(profilePosition_machineUnits * actuatorUnitsPerMachineUnits);
		break;
	case PositionControl::Type::CLOSED_LOOP:
		Logger::critical("Closed loop control is not supported yet");
		actuatorDevice->setCommand(profileVelocity_machineUnitsPerSecond * actuatorUnitsPerMachineUnits);
		//TODO:
		//for closed loop, implement pid control with adjustable gains
		break;
	}

	position->set(actualPosition_machineUnits);
	velocity->set(actualVelocity_machineUnitsPerSecond);

	//prepare metrics data
	double positionError = profilePosition_machineUnits - actualPosition_machineUnits / feedbackUnitsPerMachineUnits;
	double actualLoad;
	if (needsActuator) actualLoad = actuatorDevice->getLoad();
	else if (needsServo) actualLoad = servoActuatorDevice->getLoad();
	positionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profilePosition_machineUnits));
	actualPositionHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualPosition_machineUnits));
	positionErrorHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, positionError));
	velocityHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileVelocity_machineUnitsPerSecond));
	accelerationHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, profileAcceleration_machineUnitsPerSecondSquared));
	loadHistory.addPoint(glm::vec2(currentProfilePointTime_seconds, actualLoad));
}

//=================================== STATE CONTROL ============================================

void SingleAxisMachine::enable() {
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

void SingleAxisMachine::onEnable() {
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_machineUnitsPerSecond = 0.0;
	profileVelocity_machineUnitsPerSecond = 0.0;
	profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	b_enabled = true;
	Logger::info("Machine '{}' was enabled", getName());
}

void SingleAxisMachine::disable() {
	b_enabled = false;
	if (needsActuatorDevice()) getActuatorDevice()->disable();
	else if (needsServoActuatorDevice()) getServoActuatorDevice()->disable();
	targetCurveProfile = MotionCurve::CurveProfile();
	manualVelocityTarget_machineUnitsPerSecond = 0.0;
	profileVelocity_machineUnitsPerSecond = 0.0;
	profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	Logger::info("Machine {} disabled", getName());
}

bool SingleAxisMachine::isEnabled() {
	return b_enabled;
}

bool SingleAxisMachine::isReady() {
	//checks if all connected devices are connected and ready
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

//================================= MANUAL CONTROLS ===================================

void SingleAxisMachine::setVelocity(double velocity_machineUnits) {
	manualVelocityTarget_machineUnitsPerSecond = velocity_machineUnits;
	if (controlMode == ControlMode::POSITION_TARGET) {
		targetCurveProfile = MotionCurve::CurveProfile();
	}
	controlMode = ControlMode::VELOCITY_TARGET;
}

void SingleAxisMachine::velocityTargetControl() {
	if (profileVelocity_machineUnitsPerSecond != manualVelocityTarget_machineUnitsPerSecond) {
		double deltaV_machineUnitsPerSecond = manualControlAcceleration_machineUnitsPerSecond * currentProfilePointDeltaT_seconds;
		if (profileVelocity_machineUnitsPerSecond < manualVelocityTarget_machineUnitsPerSecond) {
			profileVelocity_machineUnitsPerSecond += deltaV_machineUnitsPerSecond;
			profileAcceleration_machineUnitsPerSecondSquared = manualControlAcceleration_machineUnitsPerSecond;
			if (profileVelocity_machineUnitsPerSecond > manualVelocityTarget_machineUnitsPerSecond) profileVelocity_machineUnitsPerSecond = manualVelocityTarget_machineUnitsPerSecond;
		}
		else {
			profileVelocity_machineUnitsPerSecond -= deltaV_machineUnitsPerSecond;
			profileAcceleration_machineUnitsPerSecondSquared = -manualControlAcceleration_machineUnitsPerSecond;
			if (profileVelocity_machineUnitsPerSecond < manualVelocityTarget_machineUnitsPerSecond) profileVelocity_machineUnitsPerSecond = manualVelocityTarget_machineUnitsPerSecond;
		}
	}
	else profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	double deltaP_machineUnits = profileVelocity_machineUnitsPerSecond * currentProfilePointDeltaT_seconds;
	profilePosition_machineUnits += deltaP_machineUnits;
}

void SingleAxisMachine::moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits) {
	if (position_machineUnits > maxPositiveDeviation_machineUnits && enablePositiveLimit) position_machineUnits = maxPositiveDeviation_machineUnits;
	else if (position_machineUnits < maxNegativeDeviation_machineUnits && enableNegativeLimit) position_machineUnits = maxNegativeDeviation_machineUnits;
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(0.0, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getFastestVelocityConstrainedProfile(startPoint, endPoint, velocity_machineUnits, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void SingleAxisMachine::moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits) {
	if (position_machineUnits > maxPositiveDeviation_machineUnits && enablePositiveLimit) position_machineUnits = maxPositiveDeviation_machineUnits;
	else if (position_machineUnits < maxNegativeDeviation_machineUnits && enableNegativeLimit) position_machineUnits = maxNegativeDeviation_machineUnits;
	MotionCurve::CurvePoint startPoint(currentProfilePointTime_seconds, profilePosition_machineUnits, acceleration_machineUnits, profileVelocity_machineUnitsPerSecond);
	MotionCurve::CurvePoint endPoint(currentProfilePointTime_seconds + movementTime_seconds, position_machineUnits, acceleration_machineUnits, 0.0);
	if (MotionCurve::getTimeConstrainedProfile(startPoint, endPoint, velocityLimit_machineUnitsPerSecond, targetCurveProfile)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_machineUnitsPerSecond = 0.0;
	}
	else {
		setVelocity(0.0);
	}
}

void SingleAxisMachine::positionTargetControl() {
	if (MotionCurve::isInsideCurveTime(currentProfilePointTime_seconds, targetCurveProfile)) {
		MotionCurve::CurvePoint curvePoint = MotionCurve::getCurvePointAtTime(currentProfilePointTime_seconds, targetCurveProfile);
		profilePosition_machineUnits = curvePoint.position;
		profileVelocity_machineUnitsPerSecond = curvePoint.velocity;
		profileAcceleration_machineUnitsPerSecondSquared = curvePoint.acceleration;
	}
	else {
		profileVelocity_machineUnitsPerSecond = 0.0;
		profileAcceleration_machineUnitsPerSecondSquared = 0.0;
	}
}

//========================== DEVICES =============================

bool SingleAxisMachine::needsPositionFeedbackDevice() {
	switch (positionControl) {
	case PositionControl::Type::CLOSED_LOOP:
		return true;
	case PositionControl::Type::SERVO:
		return false;
	}
}

bool SingleAxisMachine::isPositionFeedbackDeviceConnected() {
	return positionFeedbackDeviceLink->isConnected();
}

std::shared_ptr<PositionFeedbackDevice> SingleAxisMachine::getPositionFeedbackDevice() {
	return positionFeedbackDeviceLink->getConnectedPins().front()->getPositionFeedbackDevice();
}



bool SingleAxisMachine::needsReferenceDevice() {
	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		return true;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		return false;
	}
}

bool SingleAxisMachine::isReferenceDeviceConnected() {
	return referenceDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> SingleAxisMachine::getReferenceDevice() {
	return referenceDeviceLink->getConnectedPins().front()->getGpioDevice();
}

bool SingleAxisMachine::needsActuatorDevice() {
	switch (positionControl) {
	case PositionControl::Type::CLOSED_LOOP:
		return true;
	case PositionControl::Type::SERVO:
		return false;
	}
}

bool SingleAxisMachine::isActuatorDeviceConnected() {
	return actuatorDeviceLink->isConnected();
}
std::shared_ptr<ActuatorDevice> SingleAxisMachine::getActuatorDevice() {
	return actuatorDeviceLink->getConnectedPins().front()->getActuatorDevice();
}

bool SingleAxisMachine::needsServoActuatorDevice() {
	switch (positionControl) {
	case PositionControl::Type::CLOSED_LOOP:
		return false;
	case PositionControl::Type::SERVO:
		return true;
	}
}

bool SingleAxisMachine::isServoActuatorDeviceConnected() {
	return servoActuatorDeviceLink->isConnected();
}

std::shared_ptr<ServoActuatorDevice> SingleAxisMachine::getServoActuatorDevice() {
	return servoActuatorDeviceLink->getConnectedPins().front()->getServoActuatorDevice();
}



//============================== LIMITS ORIGINS AND REFERENCES ================================


void SingleAxisMachine::updateReferenceSignals() {
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

bool SingleAxisMachine::isMoving() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		return getServoActuatorDevice()->isMoving();
		break;
	case PositionControl::Type::CLOSED_LOOP:
		getPositionFeedbackDevice()->isMoving();
		break;
	}
}

double SingleAxisMachine::getAxisPositionProgress() {
	double lowLimit = getLowAxisPositionLimit();
	double highLimit = getHighAxisPositionLimit();
	double current = actualPosition_machineUnits;
	return (current - lowLimit) / (highLimit - lowLimit);
}


double SingleAxisMachine::getLowAxisPositionLimit() {
	double lowLimit = -std::numeric_limits<double>::infinity();
	if (enableNegativeLimit) lowLimit = maxNegativeDeviation_machineUnits;
	if (limitToFeedbackWorkingRange) {
		double lowFeedbackLimit = getLowFeedbackPositionLimit();
		if (lowLimit < lowFeedbackLimit) lowLimit = lowFeedbackLimit;
	}
	return lowLimit;
}

double SingleAxisMachine::getHighAxisPositionLimit() {
	double highLimit = std::numeric_limits<double>::infinity();
	if (enablePositiveLimit) highLimit = maxPositiveDeviation_machineUnits;
	if (limitToFeedbackWorkingRange) {
		double highFeedbackLimit = getHighFeedbackPositionLimit();
		if (highLimit > highFeedbackLimit) highLimit = highFeedbackLimit;
	}
	return highLimit;
}

double SingleAxisMachine::getLowFeedbackPositionLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMinPosition() / feedbackUnitsPerMachineUnits;
		else return -std::numeric_limits<double>::infinity();
	case PositionControl::Type::CLOSED_LOOP:
		if (isPositionFeedbackDeviceConnected()) return getPositionFeedbackDevice()->getMinPosition() / feedbackUnitsPerMachineUnits;
		else return -std::numeric_limits<double>::infinity();
	}
}

double SingleAxisMachine::getHighFeedbackPositionLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMaxPosition() / feedbackUnitsPerMachineUnits;
		else return std::numeric_limits<double>::infinity();
	case PositionControl::Type::CLOSED_LOOP:
		if (isPositionFeedbackDeviceConnected()) return getPositionFeedbackDevice()->getMaxPosition() / feedbackUnitsPerMachineUnits;
		else return std::numeric_limits<double>::infinity();
	}
}


void SingleAxisMachine::setCurrentAxisPosition(double distance) {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		getServoActuatorDevice()->setPosition(distance * feedbackUnitsPerMachineUnits);
		break;
	case PositionControl::Type::CLOSED_LOOP:
		getPositionFeedbackDevice()->setPosition(distance * feedbackUnitsPerMachineUnits);
		break;
	}
	profilePosition_machineUnits = distance;
}

void SingleAxisMachine::setCurrentPositionAsNegativeLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		maxNegativeDeviation_machineUnits = getServoActuatorDevice()->getPosition() / feedbackUnitsPerMachineUnits;
		break;
	case PositionControl::Type::CLOSED_LOOP:
		maxNegativeDeviation_machineUnits = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerMachineUnits;
		break;
	}
}

void SingleAxisMachine::setCurrentPositionAsPositiveLimit() {
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		maxPositiveDeviation_machineUnits = getServoActuatorDevice()->getPosition() / feedbackUnitsPerMachineUnits;
		break;
	case PositionControl::Type::CLOSED_LOOP:
		maxPositiveDeviation_machineUnits = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerMachineUnits;
		break;
	}
}

void SingleAxisMachine::scaleFeedbackToMatchPosition(double position_axisUnits) {
	double feedbackDevicePosition_feedbackUnits;
	switch (positionControl) {
	case PositionControl::Type::SERVO:
		feedbackDevicePosition_feedbackUnits = getServoActuatorDevice()->getPosition();
		feedbackUnitsPerMachineUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
		actuatorUnitsPerMachineUnits = feedbackUnitsPerMachineUnits;
		break;
	case PositionControl::Type::CLOSED_LOOP:
		feedbackDevicePosition_feedbackUnits = getPositionFeedbackDevice()->getPosition();
		feedbackUnitsPerMachineUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
		break;
	}
	profilePosition_machineUnits = feedbackDevicePosition_feedbackUnits / feedbackUnitsPerMachineUnits;
}

//=================================================================


void SingleAxisMachine::followCurveControl() {}


void SingleAxisMachine::startHoming() {
	b_isHoming = true;
	homingStep = Homing::Step::NOT_STARTED;
	homingError = Homing::Error::NONE;
}

void SingleAxisMachine::cancelHoming() {
	b_isHoming = false;
	homingStep = Homing::Step::NOT_STARTED;
	controlMode = ControlMode::VELOCITY_TARGET;
	setVelocity(0.0);
}

bool SingleAxisMachine::isHoming() {
	return b_isHoming;
}

bool SingleAxisMachine::didHomingSucceed() {
	return !isHoming() && homingStep == Homing::Step::FINISHED;
}

bool SingleAxisMachine::didHomingFail() {
	return homingError != Homing::Error::NONE;
}

void SingleAxisMachine::onHomingSuccess() {
	b_isHoming = false;
	homingStep = Homing::Step::FINISHED;
}

void SingleAxisMachine::onHomingError() {
	b_isHoming = false;
	homingStep = Homing::Step::NOT_STARTED;
	disable();
}

void SingleAxisMachine::homingControl() {

	using namespace Homing;

	switch (positionReferenceSignal) {



	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:

		switch (homingStep) {
		case Step::NOT_STARTED:
			homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
			break;
		case Step::SEARCHING_LOW_LIMIT_COARSE:
			setVelocity(-homingVelocity_machineUnitsPerSecond);
			if (!previousLowLimitSignal && lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_COARSE;
			break;
		case Step::FOUND_LOW_LIMIT_COARSE:
			setVelocity(0.0);
			if (!isMoving()) {
				if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
					getServoActuatorDevice()->enable();
					profilePosition_machineUnits = actualPosition_machineUnits;
					profileVelocity_machineUnitsPerSecond = 0.0;
					profileAcceleration_machineUnitsPerSecondSquared = 0.0;
				}
				else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
			}
			break;
		case Step::SEARCHING_LOW_LIMIT_FINE:
			setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
			if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
			break;
		case Step::FOUND_LOW_LIMIT_FINE:
			setVelocity(0.0);
			if (!isMoving()) homingStep = Step::RESETTING_POSITION_FEEDBACK;
			break;
		case Step::RESETTING_POSITION_FEEDBACK:
			setCurrentAxisPosition(0.0);
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
				setVelocity(-homingVelocity_machineUnitsPerSecond);
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
						profilePosition_machineUnits = actualPosition_machineUnits;
						profileVelocity_machineUnitsPerSecond = 0.0;
						profileAcceleration_machineUnitsPerSecondSquared = 0.0;
					}
					else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
				}
				break;
			case Step::SEARCHING_LOW_LIMIT_FINE:
				setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
				break;
			case Step::FOUND_LOW_LIMIT_FINE:
				setVelocity(0.0);
				if (!isMoving()) homingStep = Step::RESETTING_POSITION_FEEDBACK;
				break;
			case Step::RESETTING_POSITION_FEEDBACK:
				setCurrentAxisPosition(0.0);
				homingStep = Step::SEARCHING_HIGH_LIMIT_COARSE;
				break;
			case Step::SEARCHING_HIGH_LIMIT_COARSE:
				setVelocity(homingVelocity_machineUnitsPerSecond);
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
						profilePosition_machineUnits = actualPosition_machineUnits;
						profileVelocity_machineUnitsPerSecond = 0.0;
						profileAcceleration_machineUnitsPerSecondSquared = 0.0;
					}
					else homingStep = Step::SEARCHING_HIGH_LIMIT_FINE;
				}
				break;
			case Step::SEARCHING_HIGH_LIMIT_FINE:
				setVelocity(-homingVelocity_machineUnitsPerSecond / 10.0);
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
				setVelocity(homingVelocity_machineUnitsPerSecond);
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
						profilePosition_machineUnits = actualPosition_machineUnits;
						profileVelocity_machineUnitsPerSecond = 0.0;
						profileAcceleration_machineUnitsPerSecondSquared = 0.0;
					}
					else homingStep = Step::SEARCHING_HIGH_LIMIT_FINE;
				}
				break;
			case Step::SEARCHING_HIGH_LIMIT_FINE:
				setVelocity(-homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousHighLimitSignal && !highLimitSignal) homingStep = Step::FOUND_HIGH_LIMIT_FINE;
				break;
			case Step::FOUND_HIGH_LIMIT_FINE:
				setVelocity(0.0);
				if (!isMoving()) homingStep = Step::SETTING_HIGH_LIMIT;
				break;
			case Step::SETTING_HIGH_LIMIT:
				setCurrentAxisPosition(0.0);
				homingStep = Step::SEARCHING_LOW_LIMIT_COARSE;
				break;
			case Step::SEARCHING_LOW_LIMIT_COARSE:
				setVelocity(-homingVelocity_machineUnitsPerSecond);
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
						profilePosition_machineUnits = actualPosition_machineUnits;
						profileVelocity_machineUnitsPerSecond = 0.0;
						profileAcceleration_machineUnitsPerSecondSquared = 0.0;
					}
					else homingStep = Step::SEARCHING_LOW_LIMIT_FINE;
				}
				break;
			case Step::SEARCHING_LOW_LIMIT_FINE:
				setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousLowLimitSignal && !lowLimitSignal) homingStep = Step::FOUND_LOW_LIMIT_FINE;
				break;
			case Step::FOUND_LOW_LIMIT_FINE:
				setVelocity(0.0);
				if (!isMoving()) homingStep = Step::RESETTING_POSITION_FEEDBACK;
				break;
			case Step::RESETTING_POSITION_FEEDBACK:
				maxPositiveDeviation_machineUnits = std::abs(actualPosition_machineUnits);
				setCurrentAxisPosition(0.0);
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
				setVelocity(homingVelocity_machineUnitsPerSecond);
				if (!previousReferenceSignal && referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_BELOW_COARSE;
				break;
			case Step::FOUND_REFERENCE_FROM_BELOW_COARSE:
				setVelocity(0.0);
				if (!isMoving()) homingStep = Step::SEARCHING_REFERENCE_FROM_BELOW_FINE;
				break;
			case Step::SEARCHING_REFERENCE_FROM_BELOW_FINE:
				setVelocity(-homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_BELOW_FINE;
				break;
			case Step::FOUND_REFERENCE_FROM_BELOW_FINE:
				setVelocity(0.0);
				if (!isMoving()) {
					setCurrentAxisPosition(0.0);
					homingStep = Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
				}
				break;
			case Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
				setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_ABOVE_FINE;
				break;
			case Step::FOUND_REFERENCE_FROM_ABOVE_FINE:
				setVelocity(0.0);
				if (!isMoving()) {
					setCurrentAxisPosition(actualPosition_machineUnits / 2.0);
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
				setVelocity(-homingVelocity_machineUnitsPerSecond);
				if (!previousReferenceSignal && referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_ABOVE_COARSE;
				break;
			case Step::FOUND_REFERENCE_FROM_ABOVE_COARSE:
				setVelocity(0.0);
				if (!isMoving()) homingStep = Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
				break;
			case Step::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
				setVelocity(homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_ABOVE_FINE;
				break;
			case Step::FOUND_REFERENCE_FROM_ABOVE_FINE:
				setVelocity(0.0);
				if (!isMoving()) {
					setCurrentAxisPosition(0.0);
					homingStep = Step::SEARCHING_REFERENCE_FROM_BELOW_FINE;
				}
				break;
			case Step::SEARCHING_REFERENCE_FROM_BELOW_FINE:
				setVelocity(-homingVelocity_machineUnitsPerSecond / 10.0);
				if (previousReferenceSignal && !referenceSignal) homingStep = Step::FOUND_REFERENCE_FROM_BELOW_FINE;
				break;
			case Step::FOUND_REFERENCE_FROM_BELOW_FINE:
				setVelocity(0.0);
				if (!isMoving()) {
					setCurrentAxisPosition(actualPosition_machineUnits / 2.0);
					moveToPositionInTime(0.0, 0.0, 1.0);
					onHomingSuccess();
				}
				break;
			}


		}
		//touch off reference from one side, touch off reference from other side, go to middle of two touches, set encoder to zero






	case PositionReferenceSignal::Type::NO_SIGNAL:
		break;
	}
}


//==================================== SAVING AND LOADING =========================================


bool SingleAxisMachine::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;

	XMLElement* machineXML = xml->InsertNewChildElement("Machine");
	machineXML->SetAttribute("UnitType", getPositionUnitType(machinePositionUnitType)->saveName);
	machineXML->SetAttribute("Unit", getPositionUnit(machinePositionUnit)->saveName);
	machineXML->SetAttribute("PositionControl", getPositionControlType(positionControl)->saveName);

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("FeedbackUnitsPerMachineUnit", feedbackUnitsPerMachineUnits);
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", actuatorUnitsPerMachineUnits);
	unitConversionXML->SetAttribute("FeedbackAndActuatorConversionIdentical", feedbackAndActuatorConversionIdentical);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_machineUnitsPerSecond", velocityLimit_machineUnitsPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_machineUnitsPerSecondSquared", accelerationLimit_machineUnitsPerSecondSquared);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReferenceSignals");
	positionReferenceXML->SetAttribute("Type", getPositionReferenceSignal(positionReferenceSignal)->saveName);

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		positionReferenceXML->SetAttribute("HomingVelocity_machineUnitsPerSecond", homingVelocity_machineUnitsPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", getHomingDirection(homingDirection)->saveName);
		break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		break;
	}

	XMLElement* positionLimitsXML = xml->InsertNewChildElement("PositionLimits");
	positionLimitsXML->SetAttribute("MaxNegativeDeviation_machineUnits", maxNegativeDeviation_machineUnits);
	positionLimitsXML->SetAttribute("EnableNegativeLimit", enableNegativeLimit);
	positionLimitsXML->SetAttribute("MaxPositionDeviation_machineUnits", maxPositiveDeviation_machineUnits);
	positionLimitsXML->SetAttribute("EnablePositiveLimit", enablePositiveLimit);
	positionLimitsXML->SetAttribute("LimitToFeedbackWorkingRange", limitToFeedbackWorkingRange);

	XMLElement* defaultManualParametersXML = xml->InsertNewChildElement("DefaultManualParameters");
	defaultManualParametersXML->SetAttribute("Velocity_machineUnitsPerSecond", defaultManualVelocity_machineUnitsPerSecond);
	defaultManualParametersXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", defaultManualAcceleration_machineUnitsPerSecondSquared);

	return false;
}



bool SingleAxisMachine::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* machineXML = xml->FirstChildElement("Machine");
	if (!machineXML) return Logger::warn("Could not load Machine Attributes");
	const char* machineUnitTypeString;
	if (machineXML->QueryStringAttribute("UnitType", &machineUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit Type");
	if (getPositionUnitType(machineUnitTypeString) == nullptr) return Logger::warn("Could not read Machine Unit Type");
	machinePositionUnitType = getPositionUnitType(machineUnitTypeString)->type;
	const char* machineUnitString;
	if (machineXML->QueryStringAttribute("Unit", &machineUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (getPositionUnit(machineUnitString) == nullptr) return Logger::warn("Could not read Machine Unit");
	machinePositionUnit = getPositionUnit(machineUnitString)->unit;
	const char* motionControlTypeString;
	if (machineXML->QueryStringAttribute("PositionControl", &motionControlTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Control Type");
	if (getPositionControlType(motionControlTypeString) == nullptr) return Logger::warn("Could not read Position Control Type");
	setPositionControlType(getPositionControlType(motionControlTypeString)->type);

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("FeedbackUnitsPerMachineUnit", &feedbackUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerMachineUnit", &actuatorUnitsPerMachineUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Machine Unit");
	if (unitConversionXML->QueryBoolAttribute("FeedbackAndActuatorConversionIdentical", &feedbackAndActuatorConversionIdentical) != XML_SUCCESS) return Logger::warn("Could not load FeedbackAndActuatorConversionIdentical");

	XMLElement* positionLimitsXML = xml->FirstChildElement("PositionLimits");
	if (positionLimitsXML == nullptr) return Logger::warn("Could not load Position Limits Attribute");
	if (positionLimitsXML->QueryDoubleAttribute("MaxNegativeDeviation_machineUnits", &maxNegativeDeviation_machineUnits) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation attribute");
	if (positionLimitsXML->QueryBoolAttribute("EnableNegativeLimit", &enableNegativeLimit) != XML_SUCCESS) return Logger::warn("could not load enable negative limit attribute");
	if (positionLimitsXML->QueryDoubleAttribute("MaxPositionDeviation_machineUnits", &maxPositiveDeviation_machineUnits) != XML_SUCCESS) return Logger::warn("could not load max positive deviation attribute");
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
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN: {
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_machineUnitsPerSecond", &homingVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
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
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit_machineUnitsPerSecond", &velocityLimit_machineUnitsPerSecond)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit_machineUnitsPerSecondSquared", &accelerationLimit_machineUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");

	XMLElement* defaultManualParametersXML = xml->FirstChildElement("DefaultManualParameters");
	if (!defaultManualParametersXML) return Logger::warn("Could not load default movement parameters");
	if (defaultManualParametersXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &defaultManualVelocity_machineUnitsPerSecond) != XML_SUCCESS) Logger::warn("Could not load default movement velocity");
	if (defaultManualParametersXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &defaultManualAcceleration_machineUnitsPerSecondSquared) != XML_SUCCESS) Logger::warn("Could not load default movement acceleration");
	manualControlAcceleration_machineUnitsPerSecond = defaultManualAcceleration_machineUnitsPerSecondSquared;
	targetVelocity_machineUnitsPerSecond = defaultManualVelocity_machineUnitsPerSecond;

	return true;
}



void SingleAxisMachine::setPositionControlType(PositionControl::Type type) {
	//remove all pins
	//we then add them all again to assure the same pin order each time
	removeIoData(positionFeedbackDeviceLink);
	removeIoData(servoActuatorDeviceLink);
	removeIoData(actuatorDeviceLink);
	removeIoData(referenceDeviceLink);
	removeIoData(lowLimitSignalPin);
	removeIoData(highLimitSignalPin);
	removeIoData(referenceSignalPin);
	switch (type) {
	case PositionControl::Type::CLOSED_LOOP:
		addIoData(actuatorDeviceLink);
		addIoData(positionFeedbackDeviceLink);
		break;
	case PositionControl::Type::SERVO:
		addIoData(servoActuatorDeviceLink);
		feedbackUnitsPerMachineUnits = actuatorUnitsPerMachineUnits;
		break;
	}
	positionControl = type;
	setPositionReferenceSignalType(positionReferenceSignal);
}

void SingleAxisMachine::setPositionReferenceSignalType(PositionReferenceSignal::Type type) {
	removeIoData(referenceDeviceLink);
	removeIoData(lowLimitSignalPin);
	removeIoData(highLimitSignalPin);
	removeIoData(referenceSignalPin);
	switch (type) {
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_LIMIT:
		addIoData(referenceDeviceLink);
		addIoData(lowLimitSignalPin);
		maxNegativeDeviation_machineUnits = 0.0;
		enableNegativeLimit = true;
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		addIoData(referenceDeviceLink);
		addIoData(highLimitSignalPin);
		addIoData(lowLimitSignalPin);
		maxNegativeDeviation_machineUnits = 0.0;
		enableNegativeLimit = true;
		enablePositiveLimit = true;
		break;
	case PositionReferenceSignal::Type::SIGNAL_AT_ORIGIN:
		addIoData(referenceDeviceLink);
		addIoData(referenceSignalPin);
		break;
	case PositionReferenceSignal::Type::NO_SIGNAL:
		break;
	}
	positionReferenceSignal = type;
}