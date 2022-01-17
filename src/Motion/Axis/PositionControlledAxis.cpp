#include <pch.h>

#include "PositionControlledAxis.h"
#include "NodeGraph/Device.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "NodeGraph/Device.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

void PositionControlledAxis::assignIoData() {
	//inputs
	addIoData(actuatorDeviceLink);
	addIoData(servoActuatorDeviceLink);
	addIoData(positionFeedbackDeviceLink);
	addIoData(referenceDeviceLink);
	
	lowLimitSignalPin->assignData(lowLimitSignalPinValue);
	highLimitSignalPin->assignData(highLimitSignalPinValue);
	referenceSignalPin->assignData(referenceSignalPinValue);
	
	addIoData(lowLimitSignalPin);
	addIoData(highLimitSignalPin);
	addIoData(referenceSignalPin);
	//outputs
	//these pins are always present
	std::shared_ptr<PositionControlledAxis> thisAxis = std::dynamic_pointer_cast<PositionControlledAxis>(shared_from_this());
	positionControlledAxisPin->assignData(thisAxis);
	addIoData(positionControlledAxisPin);
	
	position->assignData(positionPinValue);
	addIoData(position);
	
	velocity->assignData(velocityPinValue);
	addIoData(velocity);
	setPositionControlType(positionControlType);
	setPositionReferenceSignalType(positionReferenceSignal);
	targetInterpolation = std::make_shared<Motion::Interpolation>();
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
	if (needsReference) {
		updateReferenceSignals();
		if (lowLimitSignal && profileVelocity_axisUnitsPerSecond < 0.0) fastStop();
		if (highLimitSignal && profileVelocity_axisUnitsPerSecond > 0.0) fastStop();
	}

	//handle device state transitions
	if (b_enabled) {
		if (needsActuator && !actuatorDevice->b_enabled) disable();
		if (needsFeedback && !positionFeedbackDevice->b_ready) disable();
		if (needsReference && !referenceDevice->b_ready) disable();
		if (needsServo && (!servoActuatorDevice->b_ready || !servoActuatorDevice->b_enabled)) {
			if (!isHoming())	disable();
		}
	}

	//get actual realtime axis motion values
	if (needsFeedback) {
		actualPosition_axisUnits = positionFeedbackDevice->getPosition() / feedbackUnitsPerAxisUnits;
		actualVelocity_axisUnitsPerSecond = positionFeedbackDevice->getVelocity() / feedbackUnitsPerAxisUnits;
	}
	else if (needsServo) {
		actualPosition_axisUnits = servoActuatorDevice->getPosition() / actuatorUnitsPerAxisUnits;
		actualVelocity_axisUnitsPerSecond = servoActuatorDevice->getVelocity() / actuatorUnitsPerAxisUnits;
	}
	
	*positionPinValue = actualPosition_axisUnits;
	*velocityPinValue = actualVelocity_axisUnitsPerSecond;
	
	switch (positionControlType) {
		case PositionControlType::SERVO:
			actualLoad_normalized = getServoActuatorDevice()->getLoad();
			break;
		case PositionControlType::CLOSED_LOOP:
			actualLoad_normalized = getActuatorDevice()->getLoad();
			break;
	}

	//update timing
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();

	//update profile generator
	if (isEnabled()) {

		//here the machine updates the motion profile of the axis and sends the commands to the actuators
		if (controlMode == ControlMode::MACHINE_CONTROL && isAxisPinConnected()) return;
		
		//else the axis controls itself and sends commands to the actuators
		if (b_isHoming) homingControl();
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET:
				velocityTargetControl();
				break;
			case ControlMode::POSITION_TARGET:
				positionTargetControl();
				break;
			case ControlMode::FAST_STOP:
				fastStopControl();
				break;
			default:
				break;
		}
	}
	else  {
		//if the machine is disabled, we just update the motion profile using the actual feedback data
		profilePosition_axisUnits = actualPosition_axisUnits;
		profileVelocity_axisUnitsPerSecond = actualVelocity_axisUnitsPerSecond;
	}
	sendActuatorCommands();
}

void PositionControlledAxis::sendActuatorCommands() {
	//here we expect that the motion profile has new values
	//we use those values to send commands to the actuators
	positionError_axisUnits = profilePosition_axisUnits - actualPosition_axisUnits / feedbackUnitsPerAxisUnits;
	switch (positionControlType) {
		case PositionControlType::SERVO:
			//for servo mode, we just request a new position from the actuator
			getServoActuatorDevice()->setCommand(profilePosition_axisUnits * actuatorUnitsPerAxisUnits);
			break;
		case PositionControlType::CLOSED_LOOP:
			//for closed loop, implement pid control with adjustable gains
			//getActuatorDevice()->setCommand(profileVelocity_axisUnitsPerSecond * actuatorUnitsPerAxisUnits);
			break;
	}
	updateMetrics();
}

void PositionControlledAxis::updateMetrics() {
	positionHistory.addPoint(glm::vec2(profileTime_seconds, profilePosition_axisUnits));
	actualPositionHistory.addPoint(glm::vec2(profileTime_seconds, actualPosition_axisUnits));
	positionErrorHistory.addPoint(glm::vec2(profileTime_seconds, positionError_axisUnits));
	velocityHistory.addPoint(glm::vec2(profileTime_seconds, profileVelocity_axisUnitsPerSecond));
	accelerationHistory.addPoint(glm::vec2(profileTime_seconds, profileAcceleration_axisUnitsPerSecondSquared));
	loadHistory.addPoint(glm::vec2(profileTime_seconds, actualLoad_normalized));
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
				if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) {
					Logger::warn("Could not enable Axis '{}', actuator did not enable on time", getName());
					return;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		else if (needsServoActuatorDevice()) {
			while (!getServoActuatorDevice()->isEnabled()) {
				if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) { 
					Logger::warn("Could not enable Axis '{}', servo actuator did not enable on time", getName()); 
					return;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(20));
			}
		}
		if (!isReady()) Logger::warn("Axis '{}' cannot be enabled", getName());
		else onEnable();
	});
	machineEnabler.detach();
}

void PositionControlledAxis::onEnable() {
	targetInterpolation->resetValues();
	manualVelocityTarget_axisUnitsPerSecond = 0.0;
	b_enabled = true;
	b_isHoming = false;
	Logger::info("Axis '{}' was enabled", getName());
}

void PositionControlledAxis::disable() {
	b_enabled = false;
	if (needsActuatorDevice()) getActuatorDevice()->disable();
	else if (needsServoActuatorDevice()) getServoActuatorDevice()->disable();
	targetInterpolation->resetValues();
	b_isHoming = false;
	manualVelocityTarget_axisUnitsPerSecond = 0.0;
	setVelocityTarget(0.0);
	Logger::info("Axis {} disabled", getName());
}

bool PositionControlledAxis::isReady() {
	if (needsActuatorDevice() && (!isActuatorDeviceConnected() || !getActuatorDevice()->isReady())) return false;
	else if (needsServoActuatorDevice() && (!isServoActuatorDeviceConnected() || !getServoActuatorDevice()->isReady())) return false;
	if (needsPositionFeedbackDevice() && (!isPositionFeedbackDeviceConnected() || !getPositionFeedbackDevice()->isReady())) return false;
	if (needsReferenceDevice() && (!isReferenceDeviceConnected() || !getReferenceDevice()->isReady())) return false;
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			if (!lowLimitSignalPin->isConnected()) return false;
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			if (!lowLimitSignalPin->isConnected()) return false;
			if (!highLimitSignalPin->isConnected()) return false;
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			if (!referenceSignalPin->isConnected()) return false;
			break;
		case PositionReferenceSignal::NO_SIGNAL:
			break;
	}
	return true;
}

//========================== DEVICES =============================

bool PositionControlledAxis::needsPositionFeedbackDevice() {
	switch (positionControlType) {
	case PositionControlType::CLOSED_LOOP:
		return true;
	case PositionControlType::SERVO:
		return false;
	default: return false;
	}
}

bool PositionControlledAxis::isPositionFeedbackDeviceConnected() {
	return positionFeedbackDeviceLink->isConnected();
}

std::shared_ptr<PositionFeedbackDevice> PositionControlledAxis::getPositionFeedbackDevice() {
	return positionFeedbackDeviceLink->getConnectedPins().front()->getSharedPointer<PositionFeedbackDevice>();
}

bool PositionControlledAxis::needsReferenceDevice() {
	switch (positionReferenceSignal) {
	case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
	case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
		return true;
	case PositionReferenceSignal::NO_SIGNAL:
		return false;
	default: return false;
	}
}

bool PositionControlledAxis::isReferenceDeviceConnected() {
	return referenceDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> PositionControlledAxis::getReferenceDevice() {
	return referenceDeviceLink->getConnectedPins().front()->getSharedPointer<GpioDevice>();
}

bool PositionControlledAxis::needsActuatorDevice() {
	switch (positionControlType) {
	case PositionControlType::CLOSED_LOOP:
		return true;
	case PositionControlType::SERVO:
		return false;
	default: return false;
	}
}

bool PositionControlledAxis::needsServoActuatorDevice() {
	switch (positionControlType) {
	case PositionControlType::CLOSED_LOOP:
		return false;
	case PositionControlType::SERVO:
		return true;
	default: return false;
	}
}

bool PositionControlledAxis::isServoActuatorDeviceConnected() {
	return servoActuatorDeviceLink->isConnected();
}

std::shared_ptr<ServoActuatorDevice> PositionControlledAxis::getServoActuatorDevice() {
	return servoActuatorDeviceLink->getConnectedPins().front()->getSharedPointer<ServoActuatorDevice>();
}



//============================= DEVICE AND SIGNAL LINKS ================================

void PositionControlledAxis::setPositionUnitType(PositionUnitType type){
	positionUnitType = type;
	switch(type){
		case PositionUnitType::ANGULAR:
			if(!isAngularPositionUnit(positionUnit)) {
				for(auto& type : Unit::getTypes<PositionUnit>()){
					if(isAngularPositionUnit(type.enumerator)){
						setPositionUnit(type.enumerator);
						break;
					}
				}
			}
			if(!isAngularPositionReferenceSignal(positionReferenceSignal)){
				for(auto& type : Enumerator::getTypes<PositionReferenceSignal>()){
					if(isAngularPositionReferenceSignal(type.enumerator)){
						setPositionReferenceSignalType(type.enumerator);
						break;
					}
				}
			}
			break;
		case PositionUnitType::LINEAR:
			if(!isLinearPositionUnit(positionUnit)){
				for(auto& type : Unit::getTypes<PositionUnit>()){
					if(isLinearPositionUnit(type.enumerator)){
						setPositionUnit(type.enumerator);
						break;
					}
				}
			}
			if(!isLinearPositionReferenceSignal(positionReferenceSignal)){
				for(auto& type : Enumerator::getTypes<PositionReferenceSignal>()){
					if(isLinearPositionReferenceSignal(type.enumerator)) {
						setPositionReferenceSignalType(type.enumerator);
						break;
					}
				}
			}
			break;
	}
}

void PositionControlledAxis::setPositionUnit(PositionUnit u){
	positionUnit = u;
}

void PositionControlledAxis::setPositionControlType(PositionControlType type) {
	switch (type) {
		case PositionControlType::CLOSED_LOOP:
			actuatorDeviceLink->setVisible(true);
			positionFeedbackDeviceLink->setVisible(true);
			servoActuatorDeviceLink->disconnectAllLinks();
			servoActuatorDeviceLink->setVisible(false);
			break;
		case PositionControlType::SERVO:
			actuatorDeviceLink->disconnectAllLinks();
			actuatorDeviceLink->setVisible(false);
			positionFeedbackDeviceLink->disconnectAllLinks();
			positionFeedbackDeviceLink->setVisible(false);
			servoActuatorDeviceLink->setVisible(true);
			feedbackUnitsPerAxisUnits = actuatorUnitsPerAxisUnits;
			break;
	}
	positionControlType = type;
}

void PositionControlledAxis::setPositionReferenceSignalType(PositionReferenceSignal type) {
	switch (type) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			referenceDeviceLink->setVisible(true);
			lowLimitSignalPin->setVisible(true);
			highLimitSignalPin->disconnectAllLinks();
			highLimitSignalPin->setVisible(false);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			maxNegativeDeviation_axisUnits = 0.0;
			enableNegativeLimit = true;
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			referenceDeviceLink->setVisible(true);
			lowLimitSignalPin->setVisible(true);
			highLimitSignalPin->setVisible(true);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			maxNegativeDeviation_axisUnits = 0.0;
			enableNegativeLimit = true;
			enablePositiveLimit = true;
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			referenceDeviceLink->setVisible(true);
			lowLimitSignalPin->disconnectAllLinks();
			lowLimitSignalPin->setVisible(false);
			highLimitSignalPin->disconnectAllLinks();
			highLimitSignalPin->setVisible(false);
			referenceSignalPin->setVisible(true);
			break;
		case PositionReferenceSignal::NO_SIGNAL:
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
	case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
		previousLowLimitSignal = lowLimitSignal;
		if(lowLimitSignalPin->isConnected()) lowLimitSignalPin->copyConnectedPinValue();
		lowLimitSignal = *lowLimitSignalPinValue;
		break;
	case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		previousLowLimitSignal = lowLimitSignal;
		if (lowLimitSignalPin->isConnected()) lowLimitSignalPin->copyConnectedPinValue();
		lowLimitSignal = *lowLimitSignalPinValue;
		previousHighLimitSignal = highLimitSignal;
		if (highLimitSignalPin->isConnected()) highLimitSignalPin->copyConnectedPinValue();
		highLimitSignal = *highLimitSignalPinValue;
		break;
	case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
		previousReferenceSignal = referenceSignal;
		if (referenceSignalPin->isConnected()) referenceSignalPin->copyConnectedPinValue();
		referenceSignal = *referenceSignalPinValue;
		break;
	default: break;
	}
}

bool PositionControlledAxis::isMoving() {
	switch (positionControlType) {
		case PositionControlType::SERVO:
			return getServoActuatorDevice()->isMoving();
			break;
		case PositionControlType::CLOSED_LOOP:
			return getPositionFeedbackDevice()->isMoving();
			break;
		default: 
			return false;
	}
}

double PositionControlledAxis::getLowPositionLimit() {
	return getLowPositionLimitWithoutClearance() + limitClearance_axisUnits;
}

double PositionControlledAxis::getHighPositionLimit() {
	return getHighPositionLimitWithoutClearance() - limitClearance_axisUnits;
}

double PositionControlledAxis::getLowPositionLimitWithoutClearance() {
	double lowLimit = -std::numeric_limits<double>::infinity();
	if (enableNegativeLimit) lowLimit = maxNegativeDeviation_axisUnits;
	if (limitToFeedbackWorkingRange) {
		double lowFeedbackLimit = getLowFeedbackPositionLimit();
		if (lowLimit < lowFeedbackLimit) lowLimit = lowFeedbackLimit;
	}
	return lowLimit;
}

double PositionControlledAxis::getHighPositionLimitWithoutClearance() {
	double highLimit = std::numeric_limits<double>::infinity();
	if (enablePositiveLimit) highLimit = maxPositiveDeviation_axisUnits;
	if (limitToFeedbackWorkingRange) {
		double highFeedbackLimit = getHighFeedbackPositionLimit();
		if (highLimit > highFeedbackLimit) highLimit = highFeedbackLimit;
	}
	return highLimit;
}

double PositionControlledAxis::getRange_axisUnits() {
	return getHighPositionLimit() - getLowPositionLimit();
}

double PositionControlledAxis::getLowFeedbackPositionLimit() {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMinPosition() / feedbackUnitsPerAxisUnits;
		else return -std::numeric_limits<double>::infinity();
	case PositionControlType::CLOSED_LOOP:
		if (isPositionFeedbackDeviceConnected()) return getPositionFeedbackDevice()->getMinPosition() / feedbackUnitsPerAxisUnits;
		else return -std::numeric_limits<double>::infinity();
	default: return 0.0;
	}
}

double PositionControlledAxis::getHighFeedbackPositionLimit() {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMaxPosition() / feedbackUnitsPerAxisUnits;
		else return std::numeric_limits<double>::infinity();
	case PositionControlType::CLOSED_LOOP:
		if (isPositionFeedbackDeviceConnected()) return getPositionFeedbackDevice()->getMaxPosition() / feedbackUnitsPerAxisUnits;
		else return std::numeric_limits<double>::infinity();
	default: return 0.0;
	}
}


void PositionControlledAxis::setCurrentPosition(double distance) {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		getServoActuatorDevice()->setPosition(distance * feedbackUnitsPerAxisUnits);
		break;
	case PositionControlType::CLOSED_LOOP:
		getPositionFeedbackDevice()->setPosition(distance * feedbackUnitsPerAxisUnits);
		break;
	}
	profilePosition_axisUnits = distance;
}

void PositionControlledAxis::setCurrentPositionAsNegativeLimit() {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		maxNegativeDeviation_axisUnits = getServoActuatorDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	case PositionControlType::CLOSED_LOOP:
		maxNegativeDeviation_axisUnits = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	}
}

void PositionControlledAxis::setCurrentPositionAsPositiveLimit() {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		maxPositiveDeviation_axisUnits = getServoActuatorDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	case PositionControlType::CLOSED_LOOP:
		maxPositiveDeviation_axisUnits = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	}
}

void PositionControlledAxis::scaleFeedbackToMatchPosition(double position_axisUnits) {
	double feedbackDevicePosition_feedbackUnits;
	switch (positionControlType) {
	case PositionControlType::SERVO:
		feedbackDevicePosition_feedbackUnits = getServoActuatorDevice()->getPosition();
		feedbackUnitsPerAxisUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
		actuatorUnitsPerAxisUnits = feedbackUnitsPerAxisUnits;
		break;
	case PositionControlType::CLOSED_LOOP:
		feedbackDevicePosition_feedbackUnits = getPositionFeedbackDevice()->getPosition();
		feedbackUnitsPerAxisUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
		break;
	}
	profilePosition_axisUnits = feedbackDevicePosition_feedbackUnits / feedbackUnitsPerAxisUnits;
}


float PositionControlledAxis::getActualPosition_normalized() {
	double low = getLowPositionLimit();
	double high = getHighPositionLimit();
	return (actualPosition_axisUnits - low) / (high - low);
}


//================================= VELOCITY TARGET CONTROL ===================================

void PositionControlledAxis::fastStop() {
	controlMode = ControlMode::FAST_STOP;
}

void PositionControlledAxis::fastStopControl() {
	if (profileVelocity_axisUnitsPerSecond != 0.0) {
		double deltaV_axisUnitsPerSecond = accelerationLimit_axisUnitsPerSecondSquared * profileTimeDelta_seconds;
		if (profileVelocity_axisUnitsPerSecond > 0.0) {
			profileVelocity_axisUnitsPerSecond -= deltaV_axisUnitsPerSecond;
			if (profileVelocity_axisUnitsPerSecond < 0.0) profileVelocity_axisUnitsPerSecond = 0.0;
		}
		else {
			profileVelocity_axisUnitsPerSecond += deltaV_axisUnitsPerSecond;
			if (profileVelocity_axisUnitsPerSecond > 0.0) profileVelocity_axisUnitsPerSecond = 0.0;
		}
		double deltaP_axisUnits = profileVelocity_axisUnitsPerSecond * profileTimeDelta_seconds;
		profilePosition_axisUnits += deltaP_axisUnits;
	}
	else {
		profileVelocity_axisUnitsPerSecond = 0.0;
		setVelocityTarget(0.0);
	}
}

void PositionControlledAxis::setVelocityTarget(double velocity_axisUnits) {
	manualVelocityTarget_axisUnitsPerSecond = velocity_axisUnits;
	if (controlMode == ControlMode::POSITION_TARGET) targetInterpolation->resetValues();
	controlMode = ControlMode::VELOCITY_TARGET;
}

void PositionControlledAxis::velocityTargetControl() {

	if (!isHoming()) {
		double brakingPositionAtMaxDeceleration = getFastStopBrakingPosition();
		if (brakingPositionAtMaxDeceleration < getLowPositionLimit()) {
			if (profileVelocity_axisUnitsPerSecond < 0.0) {
				fastStop();
				fastStopControl();
				return;
			}
			else if (manualVelocityTarget_axisUnitsPerSecond < 0.0) manualVelocityTarget_axisUnitsPerSecond = 0.0;
		}
		else if (brakingPositionAtMaxDeceleration > getHighPositionLimit()) {
			if (profileVelocity_axisUnitsPerSecond > 0.0) {
				fastStop();
				fastStopControl();
				return;
			}
			else if (manualVelocityTarget_axisUnitsPerSecond > 0.0) manualVelocityTarget_axisUnitsPerSecond = 0.0;
		}
	}


	if (profileVelocity_axisUnitsPerSecond != manualVelocityTarget_axisUnitsPerSecond) {
		double deltaV_axisUnitsPerSecond = manualControlAcceleration_axisUnitsPerSecond * profileTimeDelta_seconds;
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
	double deltaP_axisUnits = profileVelocity_axisUnitsPerSecond * profileTimeDelta_seconds;
	profilePosition_axisUnits += deltaP_axisUnits;
}



//================================= POSITION TARGET CONTROL ===================================

void PositionControlledAxis::moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits) {
	if (position_axisUnits > getHighPositionLimit()) position_axisUnits = getHighPositionLimit();
	else if (position_axisUnits < getLowPositionLimit()) position_axisUnits = getLowPositionLimit();
	auto startPoint = std::make_shared<Motion::ControlPoint>(profileTime_seconds, profilePosition_axisUnits, acceleration_axisUnits, profileVelocity_axisUnitsPerSecond);
	auto endPoint = std::make_shared<Motion::ControlPoint>(0.0, position_axisUnits, acceleration_axisUnits, 0.0);
	if (Motion::TrapezoidalInterpolation::getFastestVelocityConstrainedInterpolation(startPoint, endPoint, velocity_axisUnits, targetInterpolation)) {
		controlMode = ControlMode::POSITION_TARGET;
		manualVelocityTarget_axisUnitsPerSecond = 0.0;
	}
	else {
		setVelocityTarget(0.0);
	}
}

void PositionControlledAxis::moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits) {
	if (position_axisUnits > getHighPositionLimit()) position_axisUnits = getHighPositionLimit();
	else if (position_axisUnits < getLowPositionLimit()) position_axisUnits = getLowPositionLimit();
	auto startPoint = std::make_shared<Motion::ControlPoint>(profileTime_seconds, profilePosition_axisUnits, acceleration_axisUnits, profileVelocity_axisUnitsPerSecond);
	auto endPoint = std::make_shared<Motion::ControlPoint>(profileTime_seconds + movementTime_seconds, position_axisUnits, acceleration_axisUnits, 0.0);
	Motion::TrapezoidalInterpolation::getClosestTimeAndVelocityConstrainedInterpolation(startPoint, endPoint, velocityLimit_axisUnitsPerSecond, targetInterpolation);
}

void PositionControlledAxis::positionTargetControl() {
	if (targetInterpolation->isTimeInside(profileTime_seconds)) {
		Motion::CurvePoint curvePoint = targetInterpolation->getPointAtTime(profileTime_seconds);
		profilePosition_axisUnits = curvePoint.position;
		profileVelocity_axisUnitsPerSecond = curvePoint.velocity;
		profileAcceleration_axisUnitsPerSecondSquared = curvePoint.acceleration;
	}
	else if (targetInterpolation->getProgressAtTime(profileTime_seconds) >= 1.0) {
		profilePosition_axisUnits = targetInterpolation->outPosition;
		profileVelocity_axisUnitsPerSecond = 0.0;
		profileAcceleration_axisUnitsPerSecondSquared = 0.0;
		setVelocityTarget(0.0);
	}else{
		profileVelocity_axisUnitsPerSecond = 0.0;
		profileAcceleration_axisUnitsPerSecondSquared = 0.0;
		setVelocityTarget(0.0);
	}
}


//==================================== HOMING =====================================

bool PositionControlledAxis::isHomeable() {
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::NO_SIGNAL:
			return false;
		default: 
			return true;
	}
}

void PositionControlledAxis::startHoming() {
	b_isHoming = true;
	homingStep = HomingStep::NOT_STARTED;
	homingError = HomingError::NONE;
}

void PositionControlledAxis::cancelHoming() {
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	controlMode = ControlMode::VELOCITY_TARGET;
	setVelocityTarget(0.0);
}

bool PositionControlledAxis::isHoming() {
	return b_isHoming;
}

bool PositionControlledAxis::didHomingSucceed() {
	return !isHoming() && homingStep == HomingStep::FINISHED;
}

bool PositionControlledAxis::didHomingFail() {
	return homingError != HomingError::NONE;
}

void PositionControlledAxis::onHomingSuccess() {
	b_isHoming = false;
	homingStep = HomingStep::FINISHED;
}

void PositionControlledAxis::onHomingError() {
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	disable();
}

void PositionControlledAxis::homingControl() {

	switch (positionReferenceSignal) {

		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:

			switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond);
					if (/*!previousLowLimitSignal && */lowLimitSignal) homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
					break;
				case HomingStep::FOUND_LOW_LIMIT_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
							//Logger::warn("Trying To Enable Axis");
						}
						else homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
					break;
				case HomingStep::FOUND_LOW_LIMIT_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
					break;
				case HomingStep::RESETTING_POSITION_FEEDBACK:
					setCurrentPosition(0.0);
					onHomingSuccess();
					break;
				default: break;
				}
				break;

		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:

			if (homingDirection == HomingDirection::NEGATIVE) {

				switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond);
					if (/*!previousHighLimitSignal && */highLimitSignal) {
						homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousLowLimitSignal && lowLimitSignal) homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
					break;
				case HomingStep::FOUND_LOW_LIMIT_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
					break;
				case HomingStep::FOUND_LOW_LIMIT_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
					break;
				case HomingStep::RESETTING_POSITION_FEEDBACK:
					setCurrentPosition(0.0);
					homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
					break;
				case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond);
					if (/*!previousLowLimitSignal && */lowLimitSignal) {
						homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousHighLimitSignal && highLimitSignal) homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
					break;
				case HomingStep::FOUND_HIGH_LIMIT_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
					}
					break;
				case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousHighLimitSignal && !highLimitSignal) homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
					break;
				case HomingStep::FOUND_HIGH_LIMIT_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::SETTING_HIGH_LIMIT;
					break;
				case HomingStep::SETTING_HIGH_LIMIT:
					setCurrentPositionAsPositiveLimit();
					homingStep = HomingStep::FINISHED;
					onHomingSuccess();
					break;
					default: break;
				}
				break;

			}
			else if (homingDirection == HomingDirection::POSITIVE) {


				switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
					break;
				case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond);
					if (/*!previousLowLimitSignal && */lowLimitSignal) {
						homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousHighLimitSignal && highLimitSignal) homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
					break;
				case HomingStep::FOUND_HIGH_LIMIT_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
					}
					break;
				case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousHighLimitSignal && !highLimitSignal) homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
					break;
				case HomingStep::FOUND_HIGH_LIMIT_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::SETTING_HIGH_LIMIT;
					break;
				case HomingStep::SETTING_HIGH_LIMIT:
					setCurrentPosition(0.0);
					homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond);
					if (/*!previousHighLimitSignal && */highLimitSignal) {
						homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
						onHomingError();
					}
					else if (!previousLowLimitSignal && lowLimitSignal) homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
					break;
				case HomingStep::FOUND_LOW_LIMIT_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						if (needsServoActuatorDevice() && !getServoActuatorDevice()->isEnabled()) {
							getServoActuatorDevice()->enable();
							profilePosition_axisUnits = actualPosition_axisUnits;
							profileVelocity_axisUnitsPerSecond = 0.0;
							profileAcceleration_axisUnitsPerSecondSquared = 0.0;
						}
						else homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
					}
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousLowLimitSignal && !lowLimitSignal) homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
					break;
				case HomingStep::FOUND_LOW_LIMIT_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
					break;
				case HomingStep::RESETTING_POSITION_FEEDBACK:
					maxPositiveDeviation_axisUnits = std::abs(actualPosition_axisUnits);
					setCurrentPosition(0.0);
					homingStep = HomingStep::FINISHED;
					onHomingSuccess();
					break;
					default: break;
				}
				break;

			}

		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:

			if (homingDirection == HomingDirection::POSITIVE) {

				switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE;
					break;
				case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond);
					if (/*!previousReferenceSignal && */referenceSignal) homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE;
					break;
				case HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
					break;
				case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
					break;
				case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						setCurrentPosition(0.0);
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
					}
					break;
				case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
					break;
				case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						setCurrentPosition(actualPosition_axisUnits / 2.0);
						moveToPositionInTime(0.0, 0.0, 1.0);
						onHomingSuccess();
					}
					break;
					default: break;
				}

			}
			else if (homingDirection == HomingDirection::NEGATIVE) {



				switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE;
					break;
				case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond);
					if (/*!previousReferenceSignal && */referenceSignal) homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE;
					break;
				case HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE:
					setVelocityTarget(0.0);
					if (!isMoving()) homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
					break;
				case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
					setVelocityTarget(homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
					break;
				case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						setCurrentPosition(0.0);
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
					}
					break;
				case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
					setVelocityTarget(-homingVelocity_axisUnitsPerSecond / 10.0);
					if (previousReferenceSignal && !referenceSignal) homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
					break;
				case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
					setVelocityTarget(0.0);
					if (!isMoving()) {
						setCurrentPosition(actualPosition_axisUnits / 2.0);
						moveToPositionInTime(0.0, 0.0, 1.0);
						onHomingSuccess();
					}
					break;
					default: break;
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
	machineXML->SetAttribute("UnitType", Enumerator::getSaveString(positionUnitType));
	machineXML->SetAttribute("Unit", Unit::getSaveString(positionUnit));
	machineXML->SetAttribute("PositionControl", Enumerator::getSaveString(positionControlType));

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("FeedbackUnitsPerMachineUnit", feedbackUnitsPerAxisUnits);
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", actuatorUnitsPerAxisUnits);
	unitConversionXML->SetAttribute("FeedbackAndActuatorConversionIdentical", feedbackAndActuatorConversionIdentical);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit_axisUnitsPerSecond", velocityLimit_axisUnitsPerSecond);
	kinematicLimitsXML->SetAttribute("AccelerationLimit_axisUnitsPerSecondSquared", accelerationLimit_axisUnitsPerSecondSquared);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReferenceSignals");
	positionReferenceXML->SetAttribute("Type", Enumerator::getSaveString(positionReferenceSignal));

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
		positionReferenceXML->SetAttribute("HomingVelocity_axisUnitsPerSecond", homingVelocity_axisUnitsPerSecond);
		break;
	case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
		positionReferenceXML->SetAttribute("HomingVelocity_axisUnitsPerSecond", homingVelocity_axisUnitsPerSecond);
		positionReferenceXML->SetAttribute("HomingDirection", Enumerator::getSaveString(homingDirection));
		break;
	case PositionReferenceSignal::NO_SIGNAL:
		break;
	}

	XMLElement* positionLimitsXML = xml->InsertNewChildElement("PositionLimits");
	positionLimitsXML->SetAttribute("MaxNegativeDeviation_axisUnits", maxNegativeDeviation_axisUnits);
	positionLimitsXML->SetAttribute("EnableNegativeLimit", enableNegativeLimit);
	positionLimitsXML->SetAttribute("MaxPositionDeviation_axisUnits", maxPositiveDeviation_axisUnits);
	positionLimitsXML->SetAttribute("EnablePositiveLimit", enablePositiveLimit);
	positionLimitsXML->SetAttribute("LimitClearance_axisUnits", limitClearance_axisUnits);
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
	if (!Enumerator::isValidSaveName<PositionUnitType>(machineUnitTypeString)) return Logger::warn("Could not read Machine Unit Type");
	positionUnitType = Enumerator::getEnumeratorFromSaveString<PositionUnitType>(machineUnitTypeString);
	const char* axisUnitString;
	if (machineXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (!Unit::isValidSaveName<PositionUnit>(axisUnitString)) return Logger::warn("Could not read Machine Unit");
	positionUnit = Unit::getEnumeratorFromSaveString<PositionUnit>(axisUnitString);
	const char* motionControlTypeString;
	if (machineXML->QueryStringAttribute("PositionControl", &motionControlTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Control Type");
	if (!Enumerator::isValidSaveName<PositionControlType>(motionControlTypeString)) return Logger::warn("Could not read Position Control Type");
	setPositionControlType(Enumerator::getEnumeratorFromSaveString<PositionControlType>(motionControlTypeString));

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
	if (positionLimitsXML->QueryDoubleAttribute("LimitClearance_axisUnits", &limitClearance_axisUnits) != XML_SUCCESS) Logger::warn("Could not load limit clearnce attribute");
	if (positionLimitsXML->QueryBoolAttribute("LimitToFeedbackWorkingRange", &limitToFeedbackWorkingRange) != XML_SUCCESS) return Logger::warn("Could not load limit to feedback working range attribute");


	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReferenceSignals");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
	const char* positionLimitTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionLimitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (!Enumerator::isValidSaveName<PositionReferenceSignal>(positionLimitTypeString)) return Logger::warn("Could not read Position Reference Type");
	setPositionReferenceSignalType(Enumerator::getEnumeratorFromSaveString<PositionReferenceSignal>(positionLimitTypeString));

	switch (positionReferenceSignal) {
	case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_axisUnitsPerSecond", &homingVelocity_axisUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		break;
	case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
	case PositionReferenceSignal::SIGNAL_AT_ORIGIN: {
		if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity_axisUnitsPerSecond", &homingVelocity_axisUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
		const char* homingDirectionString;
		if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
		if (!Enumerator::isValidSaveName<HomingDirection>(homingDirectionString)) return Logger::warn("Could not read homing direction");
		homingDirection = Enumerator::getEnumeratorFromSaveString<HomingDirection>(homingDirectionString);
	}break;
	case PositionReferenceSignal::NO_SIGNAL:
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





void PositionControlledAxis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (needsActuatorDevice() && isActuatorDeviceConnected()) output.push_back(getActuatorDevice()->parentDevice);
	if (needsServoActuatorDevice() && isServoActuatorDeviceConnected()) output.push_back(getServoActuatorDevice()->parentDevice);
	if (needsPositionFeedbackDevice() && isPositionFeedbackDeviceConnected()) output.push_back(getPositionFeedbackDevice()->parentDevice);
	if (needsReferenceDevice() && isReferenceDeviceConnected())output.push_back(getReferenceDevice()->parentDevice);
}


void PositionControlledAxis::onDisable() {}

double PositionControlledAxis::getFastStopBrakingPosition() {
	double decelerationSigned = std::abs(accelerationLimit_axisUnitsPerSecondSquared);
	if (profileVelocity_axisUnitsPerSecond < 0.0) decelerationSigned *= -1.0;
	double decelerationPositionDelta = std::pow(profileVelocity_axisUnitsPerSecond, 2.0) / (2.0 * decelerationSigned);
	double brakingPosition_positionUnits = profilePosition_axisUnits + decelerationPositionDelta;
	return brakingPosition_positionUnits + profileVelocity_axisUnitsPerSecond * profileTimeDelta_seconds;
}
