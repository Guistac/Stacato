#include <pch.h>

#include "PositionControlledAxis.h"
#include "NodeGraph/Device.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "NodeGraph/Device.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

void PositionControlledAxis::initialize() {
	//inputs
	addNodePin(actuatorPin);
	addNodePin(servoActuatorPin);
	addNodePin(positionFeedbackPin);
	addNodePin(gpioPin);
	addNodePin(lowLimitSignalPin);
	addNodePin(highLimitSignalPin);
	addNodePin(referenceSignalPin);
	
	//outputs
	axisPin->assignData(std::dynamic_pointer_cast<PositionControlledAxis>(shared_from_this()));
	addNodePin(axisPin);
	addNodePin(positionPin);
	addNodePin(velocityPin);
	addNodePin(loadPin);
	
	//initialize parameters
	setPositionControlType(positionControlType);
	setPositionReferenceSignalType(positionReferenceSignal);
}

void PositionControlledAxis::process() {

	//check connection requirements and abort processing if the requirements are not met
	if(!areAllPinsConnected()) return;

	//get devices
	//std::shared_ptr<ActuatorDevice> actuatorDevice = getActuatorDevice();
	std::shared_ptr<ServoActuatorDevice> servoActuatorDevice = getServoActuatorDevice();
	std::shared_ptr<GpioDevice> referenceDevice = getReferenceDevice();
	std::shared_ptr<PositionFeedbackDevice> feedbackDevice = getServoActuatorDevice();

	//get reference signals
	if (referenceDevice) {
		updateReferenceSignals();
		if(*lowLimitSignal && motionProfile.getVelocity() < 0.0) fastStop();
		else if(*highLimitSignal && motionProfile.getVelocity() > 0.0) fastStop();
	}

	//handle device state transitions
	if (b_enabled) {
		//if (actuatorDevice && !actuatorDevice->b_enabled) disable();
		if (feedbackDevice && !feedbackDevice->b_ready) disable();
		if (referenceDevice && !referenceDevice->b_ready) disable();
		if (servoActuatorDevice && (!servoActuatorDevice->b_ready || !servoActuatorDevice->isEnabled())) {
			if (!isHoming()) disable();
		}
	}

	//get actual realtime axis motion values
	if (feedbackDevice) {
		*actualPositionValue = feedbackUnitsToAxisUnits(feedbackDevice->getPosition());
		*actualVelocityValue = feedbackUnitsToAxisUnits(feedbackDevice->getVelocity());
	}
	else if (servoActuatorDevice) {
		*actualPositionValue = actuatorUnitsToAxisUnits(servoActuatorDevice->getPosition());
		*actualVelocityValue = actuatorUnitsToAxisUnits(servoActuatorDevice->getVelocity());
	}
	
	//get actual load value
	switch (positionControlType) {
		case PositionControlType::SERVO:
			*actualLoadValue = getServoActuatorDevice()->getLoad();
			break;
		case PositionControlType::CLOSED_LOOP:
			*actualLoadValue = getActuatorDevice()->getLoad();
			break;
	}

	//update timing
	//TODO: Implement Universal Environnement Time
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();

	//here the machine updates the motion profile of the axis and sends the commands to the actuators
	if (isAxisPinConnected()) return;
	
	//update profile generator
	if (isEnabled()) {
		if (b_isHoming) homingControl();
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET:
				motionProfile.matchVelocity(profileTimeDelta_seconds, manualVelocityTarget, manualAcceleration);
				break;
			case ControlMode::FAST_STOP:
				motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, manualAcceleration);
				break;
			case ControlMode::POSITION_TARGET:
				motionProfile.updateInterpolation(profileTime_seconds);
				if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
				break;
			case ControlMode::EXTERNAL:
				//do nothing here, the connected node handles updating the motion profile
				break;
		}
	}
	else  {
		//if the machine is disabled, we just update the motion profile using the actual feedback data
		motionProfile.setPosition(*actualPositionValue);
		motionProfile.setVelocity(*actualVelocityValue);
		motionProfile.setAcceleration(0.0);
	}
	//convert and send the motion profile values to the actuator
	sendActuatorCommands();
}

void PositionControlledAxis::sendActuatorCommands() {
	//here we expect that the motion profile has new values
	//we use those values to send commands to the actuators
	//TODO: fix position error
	//positionError_axisUnits = motionProfile.getPosition() - actualPosition_axisUnits / feedbackUnitsPerAxisUnits;
	switch (positionControlType) {
		case PositionControlType::SERVO:
			//for servo mode, we just request a new position from the actuator
			getServoActuatorDevice()->setPositionCommand(axisUnitsToActuatorUnits(motionProfile.getPosition()),
														 axisUnitsToActuatorUnits(motionProfile.getVelocity()));
			break;
		case PositionControlType::CLOSED_LOOP:
			//TODO: implement this
			//for closed loop, implement pid control with adjustable gains
			//getActuatorDevice()->setCommand(profileVelocity_axisUnitsPerSecond * actuatorUnitsPerAxisUnits);
			getActuatorDevice()->setVelocityCommand(0.0);
			break;
	}
	//TODO: check if this works and actually updates the connected node pin
	servoActuatorPin->updateConnectedPins();
	updateMetrics();
}

void PositionControlledAxis::setMotionCommand(double position, double velocity){
	motionProfile.setPosition(position);
	motionProfile.setVelocity(velocity);
	controlMode = ControlMode::EXTERNAL;
	sendActuatorCommands();
}

//=================================== STATE CONTROL ============================================

void PositionControlledAxis::enable() {
	std::thread machineEnabler([this]() {
		using namespace std::chrono;
		system_clock::time_point start = system_clock::now();
		
		auto servoActuator = getServoActuatorDevice();
		servoActuator->enable();
		
		while(system_clock::now() - start < milliseconds(500)){
			
			if(servoActuator->isEnabled()){
				b_enabled = true;
				onEnable();
				Logger::info("Axis {} Enabled", getName());
				return;
			}
			std::this_thread::sleep_for(milliseconds(10));
		}
		
		servoActuator->disable();
		b_enabled = false;
		Logger::warn("Could not enable Axis '{}', servo actuator did not enable on time", getName());

	});
	machineEnabler.detach();
}

void PositionControlledAxis::onEnable() {
	b_enabled = true;
	setVelocityTarget(0.0);
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	homingError = HomingError::NONE;
	Logger::info("Axis '{}' was enabled", getName());
}

void PositionControlledAxis::disable() {
	if (needsActuatorDevice()) getActuatorDevice()->disable();
	else if (needsServoActuatorDevice()) getServoActuatorDevice()->disable();
	onDisable();
}

void PositionControlledAxis::onDisable() {
	b_enabled = false;
	setVelocityTarget(0.0);
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	homingError = HomingError::NONE;
	Logger::info("Axis {} disabled", getName());
}

bool PositionControlledAxis::isReady() {
	if(!areAllPinsConnected()) return false;
	if(needsActuatorDevice() && !getActuatorDevice()->isReady()) return false;
	if(needsServoActuatorDevice() && !getServoActuatorDevice()->isReady()) return false;
	if(needsPositionFeedbackDevice() && !getPositionFeedbackDevice()->isReady()) return false;
	if(needsReferenceDevice() && !getReferenceDevice()->isReady()) return false;
	return true;
}

//========================== DEVICES =============================

bool PositionControlledAxis::areAllPinsConnected(){
	if(needsPositionFeedbackDevice() && !isPositionFeedbackDeviceConnected()) return false;
	if(needsReferenceDevice() && !isReferenceDeviceConnected()) return false;
	if(needsActuatorDevice() && !isActuatorDeviceConnected()) return false;
	if(needsServoActuatorDevice() && !isServoActuatorDeviceConnected()) return false;
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

bool PositionControlledAxis::needsPositionFeedbackDevice() {
	switch (positionControlType) {
	case PositionControlType::CLOSED_LOOP:
		return true;
	case PositionControlType::SERVO:
		return false;
	default: return false;
	}
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
			actuatorPin->setVisible(true);
			positionFeedbackPin->setVisible(true);
			servoActuatorPin->disconnectAllLinks();
			servoActuatorPin->setVisible(false);
			break;
		case PositionControlType::SERVO:
			actuatorPin->disconnectAllLinks();
			actuatorPin->setVisible(false);
			positionFeedbackPin->disconnectAllLinks();
			positionFeedbackPin->setVisible(false);
			servoActuatorPin->setVisible(true);
			feedbackUnitsPerAxisUnits = actuatorUnitsPerAxisUnits;
			break;
	}
	positionControlType = type;
}

void PositionControlledAxis::setPositionReferenceSignalType(PositionReferenceSignal type) {
	switch (type) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			gpioPin->setVisible(true);
			lowLimitSignalPin->setVisible(true);
			highLimitSignalPin->disconnectAllLinks();
			highLimitSignalPin->setVisible(false);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			lowPositionLimit = 0.0;
			b_enableLowLimit = true;
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			gpioPin->setVisible(true);
			lowLimitSignalPin->setVisible(true);
			highLimitSignalPin->setVisible(true);
			referenceSignalPin->disconnectAllLinks();
			referenceSignalPin->setVisible(false);
			lowPositionLimit = 0.0;
			b_enableLowLimit = true;
			b_enableHighLimit = true;
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			gpioPin->setVisible(true);
			lowLimitSignalPin->disconnectAllLinks();
			lowLimitSignalPin->setVisible(false);
			highLimitSignalPin->disconnectAllLinks();
			highLimitSignalPin->setVisible(false);
			referenceSignalPin->setVisible(true);
			break;
		case PositionReferenceSignal::NO_SIGNAL:
			gpioPin->disconnectAllLinks();
			gpioPin->setVisible(false);
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
			previousLowLimitSignal = *lowLimitSignal;
			if(lowLimitSignalPin->isConnected()) lowLimitSignalPin->copyConnectedPinValue();
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			previousLowLimitSignal = *lowLimitSignal;
			if (lowLimitSignalPin->isConnected()) lowLimitSignalPin->copyConnectedPinValue();
			previousHighLimitSignal = *highLimitSignal;
			if (highLimitSignalPin->isConnected()) highLimitSignalPin->copyConnectedPinValue();
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			previousReferenceSignal = *referenceSignal;
			if (referenceSignalPin->isConnected()) referenceSignalPin->copyConnectedPinValue();
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
	return getLowPositionLimitWithoutClearance() + lowLimitClearance;
}

double PositionControlledAxis::getHighPositionLimit() {
	return getHighPositionLimitWithoutClearance() - highLimitClearance;
}

double PositionControlledAxis::getLowPositionLimitWithoutClearance() {
	double lowLimit = -std::numeric_limits<double>::infinity();
	if (b_enableLowLimit) lowLimit = lowPositionLimit;
	if (limitToFeedbackWorkingRange) {
		double lowFeedbackLimit = getLowFeedbackPositionLimit();
		if (lowLimit < lowFeedbackLimit) lowLimit = lowFeedbackLimit;
	}
	return lowLimit;
}

double PositionControlledAxis::getHighPositionLimitWithoutClearance() {
	double highLimit = std::numeric_limits<double>::infinity();
	if (b_enableHighLimit) highLimit = highPositionLimit;
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
	motionProfile.setPosition(distance);
}

void PositionControlledAxis::setCurrentPositionAsNegativeLimit() {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		lowPositionLimit = getServoActuatorDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	case PositionControlType::CLOSED_LOOP:
		lowPositionLimit = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	}
}

void PositionControlledAxis::setCurrentPositionAsPositiveLimit() {
	switch (positionControlType) {
	case PositionControlType::SERVO:
		highPositionLimit = getServoActuatorDevice()->getPosition() / feedbackUnitsPerAxisUnits;
		break;
	case PositionControlType::CLOSED_LOOP:
		highPositionLimit = getPositionFeedbackDevice()->getPosition() / feedbackUnitsPerAxisUnits;
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
	motionProfile.setPosition(feedbackDevicePosition_feedbackUnits / feedbackUnitsPerAxisUnits);
}

//================================= MANUAL CONTROL ===================================

void PositionControlledAxis::setVelocityTarget(double velocity_axisUnits) {
	manualVelocityTarget = velocity_axisUnits;
	controlMode = ControlMode::VELOCITY_TARGET;
}

void PositionControlledAxis::fastStop(){
	manualVelocityTarget = 0.0;
	controlMode = ControlMode::FAST_STOP;
}

void PositionControlledAxis::moveToPositionWithVelocity(double targetPosition, double targetVelocity) {
	targetPosition = std::min(targetPosition, getHighPositionLimit());
	targetPosition = std::max(targetPosition, getLowPositionLimit());
	targetVelocity = std::min(std::abs(targetVelocity), getVelocityLimit_axisUnitsPerSecond());
	bool success = motionProfile.moveToPositionWithVelocity(profileTime_seconds,
															targetPosition,
															targetVelocity,
															manualAcceleration);
	if(success) controlMode = ControlMode::POSITION_TARGET;
	else setVelocityTarget(0.0);
}

void PositionControlledAxis::moveToPositionInTime(double targetPosition, double targetTime) {
	targetPosition = std::min(targetPosition, getHighPositionLimit());
	targetPosition = std::max(targetPosition, getLowPositionLimit());
	bool success = motionProfile.moveToPositionInTime(profileTime_seconds,
													  targetPosition,
													  targetTime,
													  manualAcceleration,
													  getVelocityLimit_axisUnitsPerSecond());
	if(success) controlMode = ControlMode::POSITION_TARGET;
	else setVelocityTarget(0.0);
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
							motionProfile.setPosition(*actualPositionValue);
							motionProfile.setVelocity(0.0);
							motionProfile.setAcceleration(0.0);
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
							motionProfile.setPosition(*actualPositionValue);
							motionProfile.setVelocity(0.0);
							motionProfile.setAcceleration(0.0);
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
							motionProfile.setPosition(*actualPositionValue);
							motionProfile.setVelocity(0.0);
							motionProfile.setAcceleration(0.0);
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
							motionProfile.setPosition(*actualPositionValue);
							motionProfile.setVelocity(0.0);
							motionProfile.setAcceleration(0.0);
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
							motionProfile.setPosition(*actualPositionValue);
							motionProfile.setVelocity(0.0);
							motionProfile.setAcceleration(0.0);
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
					highPositionLimit = std::abs(*actualPositionValue);
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
						setCurrentPosition(*actualPositionValue / 2.0);
						moveToPositionInTime(0.0, 0.0);
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
						setCurrentPosition(*actualPositionValue / 2.0);
						moveToPositionInTime(0.0, 0.0);
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


void PositionControlledAxis::sanitizeParameters(){
	
	clampValue(interpolationVelocityTarget, 0.0, velocityLimit);
	clampValue(interpolationPositionTarget, getLowPositionLimit(), getHighPositionLimit());
	
	/*
	//Unit Conversions
	double feedbackUnitsPerAxisUnits = 0.0;
	double actuatorUnitsPerAxisUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal positionReferenceSignal = PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	HomingDirection homingDirection = HomingDirection::NEGATIVE;
	double homingVelocity_axisUnitsPerSecond = 0.0;
	
	//limits
	double velocityLimit = 0.0;
	double accelerationLimit = 0.0;
	double manualAcceleration = 0.0;
	double highPositionLimit = 0.0;
	double lowPositionLimit = 0.0;
	double limitClearance_axisUnits = 0.0;
	bool limitToFeedbackWorkingRange = true;
	bool enableNegativeLimit = true;
	bool enablePositiveLimit = true;
	
	//pid controlled settings
	double proportionalGain = 0.0;
	double integralGain = 0.0;
	double derivativeGain = 0.0;
	double maxPositionError_axisUnits = 0.0;
	*/
	
	
}

//==================================== SAVING AND LOADING =========================================


bool PositionControlledAxis::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;

	XMLElement* unitsXML = xml->InsertNewChildElement("Units");
	unitsXML->SetAttribute("UnitType", Enumerator::getSaveString(positionUnitType));
	unitsXML->SetAttribute("Unit", Unit::getSaveString(positionUnit));
	unitsXML->SetAttribute("PositionControl", Enumerator::getSaveString(positionControlType));

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("FeedbackUnitsPerMachineUnit", feedbackUnitsPerAxisUnits);
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", actuatorUnitsPerAxisUnits);
	unitConversionXML->SetAttribute("FeedbackAndActuatorConversionIdentical", feedbackAndActuatorConversionIdentical);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit", velocityLimit);
	kinematicLimitsXML->SetAttribute("AccelerationLimit", accelerationLimit);
	kinematicLimitsXML->SetAttribute("ManualAcceleration", manualAcceleration);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReferenceSignals");
	positionReferenceXML->SetAttribute("Type", Enumerator::getSaveString(positionReferenceSignal));
	positionReferenceXML->SetAttribute("HomingVelocity", homingVelocity_axisUnitsPerSecond);
	positionReferenceXML->SetAttribute("HomingDirection", Enumerator::getSaveString(homingDirection));

	XMLElement* positionLimitsXML = xml->InsertNewChildElement("PositionLimits");
	positionLimitsXML->SetAttribute("LowLimit", lowPositionLimit);
	positionLimitsXML->SetAttribute("EnableLowLimit", b_enableLowLimit);
	positionLimitsXML->SetAttribute("LowLimitClearance", lowLimitClearance);
	positionLimitsXML->SetAttribute("HighLimit", highPositionLimit);
	positionLimitsXML->SetAttribute("EnableHighLimit", b_enableHighLimit);
	positionLimitsXML->SetAttribute("HighLimitClearance", highLimitClearance);
	positionLimitsXML->SetAttribute("LimitToFeedbackWorkingRange", limitToFeedbackWorkingRange);

	return false;
}



bool PositionControlledAxis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* unitsXML = xml->FirstChildElement("Units");
	if (!unitsXML) return Logger::warn("Could not load Units Attributes");
	const char* axisUnitTypeString;
	if (unitsXML->QueryStringAttribute("UnitType", &axisUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit Type");
	if (!Enumerator::isValidSaveName<PositionUnitType>(axisUnitTypeString)) return Logger::warn("Could not read Machine Unit Type");
	positionUnitType = Enumerator::getEnumeratorFromSaveString<PositionUnitType>(axisUnitTypeString);
	const char* axisUnitString;
	if (unitsXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	if (!Unit::isValidSaveName<PositionUnit>(axisUnitString)) return Logger::warn("Could not read Machine Unit");
	positionUnit = Unit::getEnumeratorFromSaveString<PositionUnit>(axisUnitString);
	const char* motionControlTypeString;
	if (unitsXML->QueryStringAttribute("PositionControl", &motionControlTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Control Type");
	if (!Enumerator::isValidSaveName<PositionControlType>(motionControlTypeString)) return Logger::warn("Could not read Position Control Type");
	setPositionControlType(Enumerator::getEnumeratorFromSaveString<PositionControlType>(motionControlTypeString));

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("FeedbackUnitsPerMachineUnit", &feedbackUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Feedback Units Per Machine Unit");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerMachineUnit", &actuatorUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Machine Unit");
	if (unitConversionXML->QueryBoolAttribute("FeedbackAndActuatorConversionIdentical", &feedbackAndActuatorConversionIdentical) != XML_SUCCESS) return Logger::warn("Could not load FeedbackAndActuatorConversionIdentical");
	
	XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Machine Kinematic Kimits");
	if (kinematicLimitsXML->QueryDoubleAttribute("VelocityLimit", &velocityLimit)) Logger::warn("Could not load velocity limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("AccelerationLimit", &accelerationLimit) != XML_SUCCESS) Logger::warn("Could not load acceleration limit");
	if (kinematicLimitsXML->QueryDoubleAttribute("ManualAcceleration", &manualAcceleration) != XML_SUCCESS) Logger::warn("Could not load manual Acceleration");

	XMLElement* positionLimitsXML = xml->FirstChildElement("PositionLimits");
	if (positionLimitsXML == nullptr) return Logger::warn("Could not load Position Limits Attribute");
	if (positionLimitsXML->QueryDoubleAttribute("LowLimit", &lowPositionLimit) != XML_SUCCESS) return Logger::warn("Could not load max negative deviation attribute");
	if (positionLimitsXML->QueryBoolAttribute("EnableLowLimit", &b_enableLowLimit) != XML_SUCCESS) return Logger::warn("could not load enable negative limit attribute");
	if (positionLimitsXML->QueryDoubleAttribute("LowLimitClearance", &lowLimitClearance) != XML_SUCCESS) Logger::warn("Could not load low limit clearnce attribute");
	if (positionLimitsXML->QueryDoubleAttribute("HighLimit", &highPositionLimit) != XML_SUCCESS) return Logger::warn("could not load max positive deviation attribute");
	if (positionLimitsXML->QueryBoolAttribute("EnableHighLimit", &b_enableHighLimit) != XML_SUCCESS) return Logger::warn("Could not load enable positive limit attribute");
	if (positionLimitsXML->QueryDoubleAttribute("HighLimitClearance", &highLimitClearance) != XML_SUCCESS) Logger::warn("Could not load high limit clearnce attribute");
	if (positionLimitsXML->QueryBoolAttribute("LimitToFeedbackWorkingRange", &limitToFeedbackWorkingRange) != XML_SUCCESS) return Logger::warn("Could not load limit to feedback working range attribute");

	XMLElement* positionReferenceXML = xml->FirstChildElement("PositionReferenceSignals");
	if (!positionReferenceXML) return Logger::warn("Could not load Machine Position Reference");
	const char* positionLimitTypeString;
	if (positionReferenceXML->QueryStringAttribute("Type", &positionLimitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Position Reference Type");
	if (!Enumerator::isValidSaveName<PositionReferenceSignal>(positionLimitTypeString)) return Logger::warn("Could not read Position Reference Type");
	setPositionReferenceSignalType(Enumerator::getEnumeratorFromSaveString<PositionReferenceSignal>(positionLimitTypeString));
	if (positionReferenceXML->QueryDoubleAttribute("HomingVelocity", &homingVelocity_axisUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not load homing velocity");
	const char* homingDirectionString;
	if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
	if (!Enumerator::isValidSaveName<HomingDirection>(homingDirectionString)) return Logger::warn("Could not read homing direction");
	homingDirection = Enumerator::getEnumeratorFromSaveString<HomingDirection>(homingDirectionString);

	return true;
}





void PositionControlledAxis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (needsActuatorDevice() && isActuatorDeviceConnected()) output.push_back(getActuatorDevice()->parentDevice);
	if (needsServoActuatorDevice() && isServoActuatorDeviceConnected()) output.push_back(getServoActuatorDevice()->parentDevice);
	if (needsPositionFeedbackDevice() && isPositionFeedbackDeviceConnected()) output.push_back(getPositionFeedbackDevice()->parentDevice);
	if (needsReferenceDevice() && isReferenceDeviceConnected())output.push_back(getReferenceDevice()->parentDevice);
}


void PositionControlledAxis::updateMetrics() {
	positionHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getPosition()));
	actualPositionHistory.addPoint(glm::vec2(profileTime_seconds, *actualPositionValue));
	positionErrorHistory.addPoint(glm::vec2(profileTime_seconds, positionError));
	velocityHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getVelocity()));
	accelerationHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getAcceleration()));
	loadHistory.addPoint(glm::vec2(profileTime_seconds, *actualLoadValue));
}
