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
	addNodePin(servoActuatorPin);
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
	setPositionReferenceSignalType(positionReferenceSignal);
}

void PositionControlledAxis::process() {

	//check connection requirements and abort processing if the requirements are not met
	if(!areAllPinsConnected()) return;

	//get devices
	std::shared_ptr<ServoActuatorDevice> servoActuatorDevice = getServoActuatorDevice();
	std::shared_ptr<GpioDevice> referenceDevice = getReferenceDevice();

	//update and react to reference signals
	if (needsReferenceDevice()) {
		updateReferenceSignals();
		if(isEnabled() && !isHoming()){
			switch(positionReferenceSignal){
				case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
					if(*highLimitSignal && motionProfile.getVelocity() > 0.0) {
						Logger::critical("Axis {} Disabled : Hit Upper Limit Signal", getName());
						disable();
					}
				case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
					if(*lowLimitSignal && motionProfile.getVelocity() < 0.0) {
						Logger::critical("Axis {} Disabled : Hit Lower Limit Signal", getName());
						disable();
					}
					break;
				default:
					break;
			}
		}
	}

	//handle device state transitions
	if (isEnabled()) {
		if (needsReferenceDevice() && !referenceDevice->isReady()) disable();
		else if (!servoActuatorDevice->isEnabled()) disable();
	}

	//get actual realtime axis motion values
	*actualPositionValue = servoActuatorUnitsToAxisUnits(servoActuatorDevice->getPosition());
	*actualVelocityValue = servoActuatorUnitsToAxisUnits(servoActuatorDevice->getVelocity());
	*actualLoadValue = getServoActuatorDevice()->getLoad();

	//update timing
	//TODO: Implement Universal Environnement Time
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
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
				//here we don't do anything, the connected pin updates the axis
				break;
		}
	}
	else  {
		//if the machine is disabled, we just update the motion profile using the actual feedback data
		motionProfile.setPosition(*actualPositionValue);
		motionProfile.setVelocity(*actualVelocityValue);
		motionProfile.setAcceleration(0.0);
	}
	
	//if the machine is controlled externally (by the axis pin) we don't sent actuator commands here
	//in this case the connected node is responsible for sending commands
	if (controlMode != ControlMode::EXTERNAL) sendActuatorCommands();

}

void PositionControlledAxis::sendActuatorCommands() {
	//here we expect that the motion profile has new values
	//we use those values to send commands to the actuators
	getServoActuatorDevice()->setPositionCommand(axisUnitsToServoActuatorUnits(motionProfile.getPosition()),
												 axisUnitsToServoActuatorUnits(motionProfile.getVelocity()));
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
	getServoActuatorDevice()->disable();
	onDisable();
}

void PositionControlledAxis::onDisable() {
	b_enabled = false;
	setVelocityTarget(0.0);
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	homingError = HomingError::NONE;
	Logger::info("Axis was {} disabled", getName());
}

bool PositionControlledAxis::isReady() {
	if(!areAllPinsConnected()) return false;
	if(!getServoActuatorDevice()->isReady()) return false;
	if(needsReferenceDevice() && !getReferenceDevice()->isReady()) return false;
	return true;
}

//========================== DEVICES =============================

bool PositionControlledAxis::areAllPinsConnected(){
	if(needsReferenceDevice() && !isReferenceDeviceConnected()) return false;
	if(!isServoActuatorDeviceConnected()) return false;
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
	sanitizeParameters();
}

void PositionControlledAxis::setPositionUnit(PositionUnit u){
	positionUnit = u;
	sanitizeParameters();
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
	sanitizeParameters();
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
	return getServoActuatorDevice()->isMoving();
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
	if (limitToFeedbackWorkingRange) lowLimit = std::max(lowLimit, getLowFeedbackPositionLimit());
	return lowLimit;
}

double PositionControlledAxis::getHighPositionLimitWithoutClearance() {
	double highLimit = std::numeric_limits<double>::infinity();
	if (b_enableHighLimit) highLimit = highPositionLimit;
	if (limitToFeedbackWorkingRange) highLimit = std::min(highLimit, getHighFeedbackPositionLimit());
	return highLimit;
}

double PositionControlledAxis::getRange() {
	return getHighPositionLimit() - getLowPositionLimit();
}

double PositionControlledAxis::getLowFeedbackPositionLimit() {
	if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getMinPosition() / servoActuatorUnitsPerAxisUnits;
	else return -std::numeric_limits<double>::infinity();
}

double PositionControlledAxis::getHighFeedbackPositionLimit() {
	if (isServoActuatorDeviceConnected()) {
		auto actuator = getServoActuatorDevice();
		double maxServoPosition = actuator->getMaxPosition();
		return maxServoPosition / servoActuatorUnitsPerAxisUnits;
	}
	else return std::numeric_limits<double>::infinity();
}

double PositionControlledAxis::getActualFollowingError(){
	return servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getFollowingError());
}

float PositionControlledAxis::getActualFollowingErrorNormalized(){
	return getServoActuatorDevice()->getFollowingErrorNormalized();
}


void PositionControlledAxis::setCurrentPosition(double distance) {
	getServoActuatorDevice()->setPosition(distance * servoActuatorUnitsPerAxisUnits);
	motionProfile.setPosition(distance);
}

void PositionControlledAxis::setCurrentPositionAsNegativeLimit() {
	lowPositionLimit = getServoActuatorDevice()->getPosition() / servoActuatorUnitsPerAxisUnits;
}

void PositionControlledAxis::setCurrentPositionAsPositiveLimit() {
	highPositionLimit = getServoActuatorDevice()->getPosition() / servoActuatorUnitsPerAxisUnits;
}

void PositionControlledAxis::scaleFeedbackToMatchPosition(double position_axisUnits) {
	double feedbackDevicePosition_feedbackUnits = getServoActuatorDevice()->getPosition();
	//this recalculates unit scaling based on the distance between on zero position
	servoActuatorUnitsPerAxisUnits = feedbackDevicePosition_feedbackUnits / position_axisUnits;
	motionProfile.setPosition(feedbackDevicePosition_feedbackUnits / servoActuatorUnitsPerAxisUnits);
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
	targetVelocity = std::min(std::abs(targetVelocity), getVelocityLimit());
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
													  getVelocityLimit());
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
					setVelocityTarget(-homingVelocityCoarse);
					//we don't check the signal rising edge but only the high condition
					//this way it triggers if we are already at the limit signal when homing was started
					if (*lowLimitSignal){
						homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
						setVelocityTarget(0.0);
					}
					break;
				case HomingStep::FOUND_LOW_LIMIT_COARSE:
					if (!isMoving()) {
						setVelocityTarget(homingVelocityFine);
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
					}
					/*
					//TODO: this is useful if the limits internaly disable the servo actuator
					if (!getServoActuatorDevice()->isEnabled()) {
						getServoActuatorDevice()->enable();
						motionProfile.setPosition(*actualPositionValue);
						motionProfile.setVelocity(0.0);
						motionProfile.setAcceleration(0.0);
						//Logger::warn("Trying To Enable Axis");
					}
					else homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
					 */
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE:
					//here we must check the falling edge
					//since we may have overshot the range of the limit signal
					if (previousLowLimitSignal && !*lowLimitSignal) {
						homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
						setVelocityTarget(0.0);
					}
					break;
				case HomingStep::FOUND_LOW_LIMIT_FINE:
					if (!isMoving()) {
						auto servoActuator = getServoActuatorDevice();
						//if the servo actuator can hard reset its encoder, do it and wait for the procedure to finish
						if(servoActuator->canHardReset()) servoActuator->hardReset();
						homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
					}
					break;
				case HomingStep::RESETTING_POSITION_FEEDBACK:
					if(getServoActuatorDevice()->canHardReset()){
						//if the servo actuator can hard reset its encoder, check if we are done resetting
						if(!getServoActuatorDevice()->isHardResetting()) {
							motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
							onHomingSuccess();
						}
					}else{
						//else we set a software offset in the encoder object
						setCurrentPosition(0.0);
						onHomingSuccess();
					}
					break;
				default: break;
				}
				break;

		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:

			if (homingDirection == HomingDirection::NEGATIVE) {

				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
						//we don't check the signal rising edge but only the high condition
						//this way it triggers if we are already at the limit signal when homing was started
						if (*highLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						else if (*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
							setVelocityTarget(homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE:
						//here we have to check the falling edge since we might have overshot the signal
						if (previousLowLimitSignal && !*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							//if the servo actuator can hard reset its encoder, do it and wait for the procedure to finish
							if(getServoActuatorDevice()->canHardReset()) getServoActuatorDevice()->hardReset();
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						//if the servo actuator can hard reset its encoder, check if we are done resetting
						if(getServoActuatorDevice()->canHardReset()){
							if(!getServoActuatorDevice()->isHardResetting()) {
								//reset the software offset of the encoder
								getServoActuatorDevice()->resetOffset();
								motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
								homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
								setVelocityTarget(homingVelocityCoarse);
							}
						}else{
							//else we set a software offset in the encoder object
							setCurrentPosition(0.0);
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
							setVelocityTarget(homingVelocityCoarse);
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
						//don't check the rising edge
						if (*lowLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						else if (*highLimitSignal){
							setVelocityTarget(0.0);
							homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE:
						if (!isMoving()) {
							setVelocityTarget(-homingVelocityFine);
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
						//check the falling edge since we might have overshot the sensor
						if (previousHighLimitSignal && !*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::SETTING_HIGH_LIMIT;
						}
						break;
					case HomingStep::SETTING_HIGH_LIMIT:
						setCurrentPositionAsPositiveLimit();
						homingStep = HomingStep::FINISHED;
						onHomingSuccess();
						break;
					default:
						break;
				}

			}
			else if (homingDirection == HomingDirection::POSITIVE) {

				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_COARSE:
						setVelocityTarget(homingVelocityCoarse);
						if (*lowLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						//dont check the rising edge since we might already have triggered the sensor at homing start
						else if (*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_COARSE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
							setVelocityTarget(-homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousHighLimitSignal && !*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::SETTING_HIGH_LIMIT;
						}
						break;
					case HomingStep::SETTING_HIGH_LIMIT:
						setCurrentPosition(0.0); //set a zero reference now, we will use it later
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
						//don't check the rising edge
						if (*highLimitSignal) {
							homingError = HomingError::TRIGGERED_WRONG_LIMIT_SIGNAL;
							onHomingError();
						}
						else if (*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
							setVelocityTarget(homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE:
						if (previousLowLimitSignal && !*lowLimitSignal){
							homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_FINE:
						if (!isMoving()) {
							highPositionLimit = std::abs(*actualPositionValue);
							//if we can hard reset the servo actuator encoder here, do it
							if(getServoActuatorDevice()->canHardReset()) {
								getServoActuatorDevice()->hardReset();
							}else{
								setCurrentPosition(0.0);
							}
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						if(getServoActuatorDevice()->canHardReset()){
							if(!getServoActuatorDevice()->isHardResetting()){
								//reset the software offset of the encoder
								getServoActuatorDevice()->resetOffset();
								motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
								onHomingSuccess();
							}
						}else{
							onHomingSuccess();
						}
						break;
					default:
						break;
				}

			}

		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:

			if (homingDirection == HomingDirection::POSITIVE) {

				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE;
						setVelocityTarget(homingVelocityCoarse);
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
						//Don't check rising edge since the signal may have been trigger already when homing was started
						if (*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(-homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
						//check for falling edge since the signal may have been overshot
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
						if (!isMoving()) {
							setCurrentPosition(0.0);
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
						//check for falling edge since the signal might have been overshot
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
						if (!isMoving()) {
							//TODO: need to return to center and hard reset encoder if possible
							setCurrentPosition(*actualPositionValue / 2.0);
							moveToPositionInTime(0.0, 0.0);
							onHomingSuccess();
						}
						break;
					default:
						break;
				}

			}
			else if (homingDirection == HomingDirection::NEGATIVE) {

				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
						//dont check the rising edge since we might already have triggered the signal at homing start
						if (*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
						if (!isMoving()) {
							setCurrentPosition(0.0);
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(-homingVelocityFine);
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(0.0);
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
						if (!isMoving()) {
							//TODO: need to return to center and hard reset encoder if possible
							setCurrentPosition(*actualPositionValue / 2.0);
							moveToPositionInTime(0.0, 0.0);
							onHomingSuccess();
						}
						break;
					default:
						break;
				}

			}
			break;

		default:
			onHomingError(); //homing should not be started for modes that don't support homing
			break;
	}
}


void PositionControlledAxis::sanitizeParameters(){
	interpolationPositionTarget = std::min(interpolationPositionTarget, getHighPositionLimit());
	interpolationPositionTarget = std::max(interpolationPositionTarget, getLowPositionLimit());
	interpolationVelocityTarget = std::min(std::abs(interpolationVelocityTarget), getVelocityLimit());
	interpolationTimeTarget = std::abs(interpolationTimeTarget);

	servoActuatorUnitsPerAxisUnits = std::abs(servoActuatorUnitsPerAxisUnits);
	
	homingVelocityCoarse = std::abs(homingVelocityCoarse);
	homingVelocityFine = std::min(std::abs(homingVelocityFine), homingVelocityCoarse);
	
	velocityLimit = std::abs(velocityLimit);
	//TODO: if(isServoActuatorDeviceConnected()) velocityLimit = std::min(velocityLimit, servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getVelocityLimit()));
	
	accelerationLimit = std::abs(accelerationLimit);
	//TODO: if(isServoActuatorDeviceConnected()) accelerationLimit = std::min(accelerationLimit, servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getAccelerationLimit()));
	
	manualAcceleration = std::min(std::abs(manualAcceleration), accelerationLimit);

	lowPositionLimit = std::min(lowPositionLimit, highPositionLimit - highLimitClearance);
	highPositionLimit = std::max(highPositionLimit, lowPositionLimit + lowLimitClearance);
	lowLimitClearance = std::abs(lowLimitClearance);
	highLimitClearance = std::abs(highLimitClearance);
}

//==================================== SAVING AND LOADING =========================================


bool PositionControlledAxis::save(tinyxml2::XMLElement* xml) {

	using namespace tinyxml2;

	XMLElement* unitsXML = xml->InsertNewChildElement("Units");
	unitsXML->SetAttribute("UnitType", Enumerator::getSaveString(positionUnitType));
	unitsXML->SetAttribute("Unit", Unit::getSaveString(positionUnit));

	XMLElement* unitConversionXML = xml->InsertNewChildElement("UnitConversion");
	unitConversionXML->SetAttribute("ActuatorUnitsPerMachineUnit", servoActuatorUnitsPerAxisUnits);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
	kinematicLimitsXML->SetAttribute("VelocityLimit", velocityLimit);
	kinematicLimitsXML->SetAttribute("AccelerationLimit", accelerationLimit);
	kinematicLimitsXML->SetAttribute("ManualAcceleration", manualAcceleration);

	XMLElement* positionReferenceXML = xml->InsertNewChildElement("PositionReferenceSignals");
	positionReferenceXML->SetAttribute("Type", Enumerator::getSaveString(positionReferenceSignal));
	positionReferenceXML->SetAttribute("HomingVelocityCoarse", homingVelocityCoarse);
	positionReferenceXML->SetAttribute("HomingVelocityFine", homingVelocityFine);
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

	XMLElement* unitConversionXML = xml->FirstChildElement("UnitConversion");
	if (!unitConversionXML) return Logger::warn("Could not load Unit Conversion");
	if (unitConversionXML->QueryDoubleAttribute("ActuatorUnitsPerMachineUnit", &servoActuatorUnitsPerAxisUnits) != XML_SUCCESS) return Logger::warn("Could not load Actuator Units Per Machine Unit");
	
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
	if (positionReferenceXML->QueryDoubleAttribute("HomingVelocityCoarse", &homingVelocityCoarse) != XML_SUCCESS) return Logger::warn("Could not load Coarse homing velocity");
	if (positionReferenceXML->QueryDoubleAttribute("HomingVelocityFine", &homingVelocityFine) != XML_SUCCESS) return Logger::warn("Could not load Fine homing velocity");
	const char* homingDirectionString;
	if (positionReferenceXML->QueryStringAttribute("HomingDirection", &homingDirectionString)) return Logger::warn("Could not load homing direction");
	if (!Enumerator::isValidSaveName<HomingDirection>(homingDirectionString)) return Logger::warn("Could not read homing direction");
	homingDirection = Enumerator::getEnumeratorFromSaveString<HomingDirection>(homingDirectionString);

	return true;
}





void PositionControlledAxis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isServoActuatorDeviceConnected()) output.push_back(getServoActuatorDevice()->parentDevice);
	if (needsReferenceDevice() && isReferenceDeviceConnected())output.push_back(getReferenceDevice()->parentDevice);
}


void PositionControlledAxis::updateMetrics() {
	positionHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getPosition()));
	actualPositionHistory.addPoint(glm::vec2(profileTime_seconds, *actualPositionValue));
	velocityHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getVelocity()));
	accelerationHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getAcceleration()));
	loadHistory.addPoint(glm::vec2(profileTime_seconds, *actualLoadValue));
}
