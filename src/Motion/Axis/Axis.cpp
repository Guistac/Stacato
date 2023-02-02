#include <pch.h>

#include "Axis.h"
#include "Environnement/NodeGraph/DeviceNode.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

namespace Motion{

void Axis::initialize() {
	//inputs
	addNodePin(servoActuatorPin);
	addNodePin(gpioPin);
	addNodePin(lowLimitSignalPin);
	addNodePin(highLimitSignalPin);
	addNodePin(referenceSignalPin);
	addNodePin(surveillanceValidInputPin);
	addNodePin(surveillanceFeedbackDevicePin);
	addNodePin(externalSurveillanceFaultResetPin);
	addNodePin(feedbackDevicePin);
	
	//outputs
	addNodePin(surveillanceValidOutputPin);
	addNodePin(surveillanceFaultResetPin);
	axisPin->assignData(std::static_pointer_cast<Axis>(shared_from_this()));
	addNodePin(axisPin);
	addNodePin(positionPin);
	addNodePin(velocityPin);
	addNodePin(loadPin);
	
	//initialize parameters
	setPositionReferenceSignalType(positionReferenceSignal);
	setSurveillance(isSurveilled());
	
	b_isSurveilled->setEditCallback([this](std::shared_ptr<Parameter> editedParameter){
		setSurveillance(isSurveilled());
	});
}

std::string Axis::getStatusString(){
	std::string statusString;
	switch(state){
		case DeviceState::OFFLINE:
			if(!areAllPinsConnected()) {
				statusString = "Axis pins are not connected correctly.\n";
				return statusString;
			}
			if(!getServoActuatorDevice()->isOnline()) {
				statusString += "Servo Actuator \"" + getServoActuatorDevice()->getName() + "\" is Offline.\n";
			}
			for(auto gpioDevicePin : gpioPin->getConnectedPins()){
				auto gpioDevice = gpioDevicePin->getSharedPointer<GpioModule>();
				if(!gpioDevice->isOnline()) statusString += "Gpio Device \"" + gpioDevice->getName() + "\" is Offline.\n";
			}
			if(isSurveilled() && !getSurveillanceFeedbackDevice()->isOnline()) {
				statusString += "Surveillance Feedback Device \"" + getSurveillanceFeedbackDevice()->getName() + "\" is Offline.\n";
			}
			break;
		case DeviceState::NOT_READY:
			if(b_hasSurveillanceError) statusString += "Axis has Surveillance Error\n";
			if(!getServoActuatorDevice()->isReady()) return "Servo Actuator is not ready : " + getServoActuatorDevice()->getStatusString() + "\n";
			for(auto gpioDevicePin : gpioPin->getConnectedPins()){
				auto gpioDevice = gpioDevicePin->getSharedPointer<GpioModule>();
				if(!gpioDevice->isOnline()) statusString += "Gpio Device " + gpioDevice->getName() + " is Offline : " + gpioDevice->getStatusString() + "\n";
			}
			if(isSurveilled() && !getSurveillanceFeedbackDevice()->isEnabled()) return "Surveillance Feedback device is not ready : " + getSurveillanceFeedbackDevice()->getStatusString() + "\n";
		case DeviceState::READY:	return "Axis is ready\n";
		case DeviceState::ENABLED: 	return "Axis is enabled\n";
	}
	return statusString;
}

void Axis::updateAxisState(){
	
	DeviceState newAxisState = DeviceState::ENABLED;
	auto checkState = [&](DeviceState deviceState){ if(int(deviceState) < int(newAxisState)) newAxisState = deviceState; };
	
	auto servoActuatorDevice = getServoActuatorDevice();
	
	for(auto gpioDevicePin : gpioPin->getConnectedPins()){
		auto gpioDeviceState = gpioDevicePin->getSharedPointer<GpioModule>()->getState();
		checkState(gpioDeviceState);
	}
	checkState(servoActuatorDevice->getState());
	if(isSurveilled()) checkState(getSurveillanceFeedbackDevice()->getState());
	if(useFeedbackDevice_Param->value) checkState(getFeedbackDevice()->getState());
	
	//handle transition from enabled state
	if(state == DeviceState::ENABLED && newAxisState != DeviceState::ENABLED) disable();
	state = newAxisState;
	
	//update estop state
	b_emergencyStopActive = servoActuatorDevice->isEmergencyStopActive();
	
	//get actual realtime axis motion values
	if(useFeedbackDevice_Param->value){
		//————— ONEGIN extra feedback device
		//if we use an extra feedback device, the real position and velocity units come from this device
		auto feedbackDevice = getFeedbackDevice();
		*actualPositionValue = feedbackUnitsToAxisUnits(feedbackDevice->getPosition());
		*actualVelocityValue = feedbackUnitsToAxisUnits(feedbackDevice->getVelocity());
	}else{
		*actualPositionValue = servoActuatorUnitsToAxisUnits(servoActuatorDevice->getPosition());
		*actualVelocityValue = servoActuatorUnitsToAxisUnits(servoActuatorDevice->getVelocity());
	}
	*actualLoadValue = getServoActuatorDevice()->getEffort();
}

void Axis::reactToReferenceSignals(){
	//update and react to reference signals
	updateReferenceSignals();
	if(state == DeviceState::ENABLED && !isHoming()){
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

void Axis::inputProcess() {
	//check connection requirements and abort processing if the requirements are not met
	if(!areAllPinsConnected()) {
		state = DeviceState::OFFLINE;
		b_emergencyStopActive = false;
		b_hasSurveillanceError = false;
		if(isSurveilled()) *surveillanceValidOutputSignal = false;
		return;
	}
	
	updateAxisState();
	reactToReferenceSignals();
	if(isSurveilled()) updateSurveillance();
}

void Axis::updateSurveillance(){
	
	auto surveillanceFeedbackDevice = getSurveillanceFeedbackDevice();
	surveillanceValidInputPin->copyConnectedPinValue();
	surveillanceVelocity = surveillanceUnitsToAxisUnits(surveillanceFeedbackDevice->getVelocity());
	double requestedVelocity = motionProfile.getVelocity();
	surveillanceVelocityError = abs(requestedVelocity - surveillanceVelocity);
	
	bool previousExternalFaultResetSignal = *externalSurveillanceFaultResetSignal;
	if(externalSurveillanceFaultResetPin->isConnected()) externalSurveillanceFaultResetPin->copyConnectedPinValue();
	
	if(b_isClearingSurveillanceError){
		*surveillanceValidOutputSignal = true;
		*surveillanceFaultResetSignal = true;
	}
	else if(b_hasSurveillanceError){
		*surveillanceValidOutputSignal = false;
		*surveillanceFaultResetSignal = false;
		//react to rising edge of external fault reset signal
		if(*externalSurveillanceFaultResetSignal && !previousExternalFaultResetSignal) clearSurveillanceFault();
	}else{
		auto triggerSurveillanceError = [this](std::string reason){
			b_hasSurveillanceError = true;
			*surveillanceValidOutputSignal = false;
			Logger::critical("Axis {} Surveillance Error : {}", getName(), reason);
			disable();
		};
		if(!surveillanceFeedbackDevice->isEnabled()){
			std::string reason = "Surveillance Feedback device " + surveillanceFeedbackDevice->getName() + " is not enabled anymore";
			triggerSurveillanceError(reason);
		}
		else if(!*surveillanceValidInputSignal){
			triggerSurveillanceError("Surveillance Input Signal Not Valid");
		}
		else if(surveillanceVelocityError > maxVelocityDeviation->value){
			triggerSurveillanceError("Max Velocity Deviation Exceeded");
		}
		else{
			b_hasSurveillanceError = false;
			*surveillanceValidOutputSignal = true;
		}
		*surveillanceFaultResetSignal = false;
	}
	
}

void Axis::setMotionCommand(double position, double velocity, double acceleration){
	motionProfile.setPosition(position);
	motionProfile.setVelocity(velocity);
	motionProfile.setAcceleration(acceleration);
	controlMode = ControlMode::EXTERNAL;
}

void Axis::outputProcess(){
	//update timing
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//update profile generator
	if (state == DeviceState::ENABLED) {
		if (b_isHoming) homingControl();
		switch (controlMode) {
			case ControlMode::VELOCITY_TARGET:
				//don't respect position limits when homing
				if(b_isHoming) motionProfile.matchVelocity(profileTimeDelta_seconds,
														   manualVelocityTarget,
														   manualAcceleration);
				//but respect them when moving manually
				else motionProfile.matchVelocityAndRespectPositionLimits(profileTimeDelta_seconds,
																		 manualVelocityTarget,
																		 manualAcceleration,
																		 getLowPositionLimit(),
																		 getHighPositionLimit(),
																		 getAccelerationLimit());
				break;
			case ControlMode::FAST_STOP:
				motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, manualAcceleration);
				break;
			case ControlMode::POSITION_TARGET:{
				auto interpolationPoint = motionProfile.getInterpolationPoint(profileTime_seconds);
				motionProfile.setPosition(interpolationPoint.position);
				motionProfile.setVelocity(interpolationPoint.velocity);
				motionProfile.setAcceleration(interpolationPoint.acceleration);
				if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
			}break;
			case ControlMode::EXTERNAL:
				//here the motion profile is expected to be controlled by the setMotionCommand() method
				break;
		}
	}
	else  {
		//if the machine is disabled, we just update the motion profile using the actual feedback data
		motionProfile.setPosition(*actualPositionValue);
		motionProfile.setVelocity(*actualVelocityValue);
		motionProfile.setAcceleration(0.0);
		
		
		if(useFeedbackDevice_Param->value){
			//————— ONEGIN extra feedback device
			//while the axis is disabled we update the offset between the incremental actuator position and the absolute encoder position
			//this offset will be applied to generate correct position commands for the actuator while maintaining absolute position
			auto servoActuator = getServoActuatorDevice();
			auto feedbackDevice = getFeedbackDevice();
			double actuatorAxisPosition = servoActuatorUnitsToAxisUnits(servoActuator->getPosition());
			double feedbackAxisPosition = feedbackUnitsToAxisUnits(feedbackDevice->getPosition());
			actuatorToFeedbackPositionOffset = actuatorAxisPosition - feedbackAxisPosition;
		}
	}
	
	//send commands to the actuator
	if(useFeedbackDevice_Param->value){
		//————— ONEGIN
		//apply an offset to actuator position commands that is calculated when axis is disabled
		double actuatorPosition = axisUnitsToServoActuatorUnits(motionProfile.getPosition() + actuatorToFeedbackPositionOffset);
		double actuatorVelocity = axisUnitsToServoActuatorUnits(motionProfile.getVelocity());
		double actuatorAcceleration = axisUnitsToServoActuatorUnits(motionProfile.getAcceleration());
		getServoActuatorDevice()->setPositionTarget(actuatorPosition);
	}else{
		double actuatorPosition = axisUnitsToServoActuatorUnits(motionProfile.getPosition());
		double actuatorVelocity = axisUnitsToServoActuatorUnits(motionProfile.getVelocity());
		double actuatorAcceleration = axisUnitsToServoActuatorUnits(motionProfile.getAcceleration());
		getServoActuatorDevice()->setPositionTarget(actuatorPosition);
	}
	updateMetrics();
}

//=================================== STATE CONTROL ============================================

void Axis::enable() {
	std::thread machineEnabler([this]() {
		using namespace std::chrono;
		system_clock::time_point start = system_clock::now();
		
		auto servoActuator = getServoActuatorDevice();
		
		servoActuator->enable();
		while(system_clock::now() - start < milliseconds(500)){
			if(servoActuator->isEnabled()){
				state = DeviceState::ENABLED;
				onEnable();
				return;
			}
			std::this_thread::sleep_for(milliseconds(10));
		}
		
		servoActuator->disable();
		state = DeviceState::READY;
		Logger::warn("Could not enable Axis '{}', servo actuator did not enable on time", getName());
		
	});
	machineEnabler.detach();
}

void Axis::onEnable() {
	setVelocityTarget(0.0);
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	homingError = HomingError::NONE;
	Logger::info("Axis {} was enabled", getName());
}

void Axis::disable() {
	if(isServoActuatorDeviceConnected()) getServoActuatorDevice()->disable();
	onDisable();
}

void Axis::onDisable() {
	Logger::info("Axis was {} disabled", getName());
}

bool Axis::isReadyToEnable() {
	if(!areAllPinsConnected()) return false;
	if(isSurveilled()){
		//if the axis is surveilled
		//the only condition necessary to allow enabling
		//is that the gpio devices are enabled and the servo actuator be online
		//the servo actuator can be in sto state and we can still can request enabling of the axis
		for(auto gpioPin : gpioPin->getConnectedPins()){
			auto gpioDevice = gpioPin->getSharedPointer<GpioModule>();
			if(!gpioDevice->isEnabled()) return false;
		}
		if(!getServoActuatorDevice()->isOnline()) return false;
		return true;
	}else return areAllDevicesReady();
}


void Axis::clearSurveillanceFault(){
	std::thread surveillanceErrorClearer([this]() {
		using namespace std::chrono;
		system_clock::time_point start = system_clock::now();
		
		long long maxEnableTimeMillis = maxSurveillanceErrorClearTime->value * 1000.0;
		
		//settings these to signals high clears the sto status of the drive
		b_isClearingSurveillanceError = true;
		
		Logger::info("Clearing Surveillance Fault");
		
		while(system_clock::now() - start < milliseconds(maxEnableTimeMillis)){
			if(*surveillanceValidInputSignal){
				b_hasSurveillanceError = false;
				b_isClearingSurveillanceError = false;
				Logger::info("Surveillance Error Cleared");
				return;
			}
			std::this_thread::sleep_for(milliseconds(10));
		}
		
		b_hasSurveillanceError = true;
		b_isClearingSurveillanceError = false;
		
		Logger::warn("Failed to clear surveillance fault");
		
	});
	surveillanceErrorClearer.detach();
}

void Axis::triggerSurveillanceFault(){
	b_hasSurveillanceError = true;
}

//========================== DEVICES =============================

bool Axis::areAllPinsConnected(){
	if(needsReferenceDevice() && !isReferenceDeviceConnected()) return false;
	if(isSurveilled() && !isSurveillanceFeedbackDeviceConnected()) return false;
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
	if(useFeedbackDevice_Param->value && !feedbackDevicePin->isConnected()) return false;
	return true;
}

bool Axis::areAllDevicesReady(){
	if(needsReferenceDevice()){
		for(auto referenceDevicePin : gpioPin->getConnectedPins()){
			auto gpioDevice = referenceDevicePin->getSharedPointer<GpioModule>();
			if(!gpioDevice->isEnabled()) return false;
		}
	}
	if(isSurveilled() && !getSurveillanceFeedbackDevice()->isEnabled()) return false;
	if(!getServoActuatorDevice()->isReady()) return false;
}

bool Axis::needsReferenceDevice() {
	if(isSurveilled()) return true;
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			return true;
		default: return false;
	}
}

//============================= DEVICE AND SIGNAL LINKS ================================

void Axis::setMovementType(MovementType type){
	movementType = type;
	switch(type){
		case MovementType::ROTARY:
			if(positionUnit->unitType != Units::Type::ANGULAR_DISTANCE) {
				setPositionUnit(Units::AngularDistance::get().front());
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
		case MovementType::LINEAR:
			if(positionUnit->unitType != Units::Type::LINEAR_DISTANCE) {
				setPositionUnit(Units::LinearDistance::get().front());
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
	axisPin->updateConnectedPins();
}

void Axis::setPositionUnit(Unit u){
	positionUnit = u;
	sanitizeParameters();
	axisPin->updateConnectedPins();
}

void Axis::setPositionReferenceSignalType(PositionReferenceSignal type) {
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
	axisPin->updateConnectedPins();
}

void Axis::setSurveillance(bool isSurveilled){
	if(isSurveilled){
		surveillanceValidInputPin->setVisible(true);
		surveillanceFeedbackDevicePin->setVisible(true);
		surveillanceValidOutputPin->setVisible(true);
		surveillanceFaultResetPin->setVisible(true);
		externalSurveillanceFaultResetPin->setVisible(true);
	}else{
		surveillanceValidInputPin->disconnectAllLinks();
		surveillanceValidInputPin->setVisible(false);
		surveillanceFeedbackDevicePin->disconnectAllLinks();
		surveillanceFeedbackDevicePin->setVisible(false);
		surveillanceValidOutputPin->disconnectAllLinks();
		surveillanceValidOutputPin->setVisible(false);
		surveillanceFaultResetPin->disconnectAllLinks();
		surveillanceFaultResetPin->setVisible(false);
		externalSurveillanceFaultResetPin->disconnectAllLinks();
		externalSurveillanceFaultResetPin->setVisible(false);
	}
}



//============================== LIMITS, ORIGIN AND REFERENCES ================================

void Axis::updateReferenceSignals() {
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			previousLowLimitSignal = *lowLimitSignal;
			lowLimitSignalPin->copyConnectedPinValue();
			break;
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			previousLowLimitSignal = *lowLimitSignal;
			lowLimitSignalPin->copyConnectedPinValue();
			previousHighLimitSignal = *highLimitSignal;
			highLimitSignalPin->copyConnectedPinValue();
			break;
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			previousReferenceSignal = *referenceSignal;
			referenceSignalPin->copyConnectedPinValue();
			break;
		default: break;
	}
}

bool Axis::isMoving() {
	return motionProfile.getVelocity() != 0.0;
	//TODO: this needs a threshold setting
	//return getServoActuatorDevice()->isMoving();
}

double Axis::getLowPositionLimit() {
	return getLowPositionLimitWithoutClearance() + lowLimitClearance;
}

double Axis::getHighPositionLimit() {
	return getHighPositionLimitWithoutClearance() - highLimitClearance;
}

double Axis::getLowPositionLimitWithoutClearance() {
	double lowLimit = -std::numeric_limits<double>::infinity();
	if (b_enableLowLimit) lowLimit = lowPositionLimit;
	if (limitToFeedbackWorkingRange) lowLimit = std::max(lowLimit, getLowFeedbackPositionLimit());
	return lowLimit;
}

double Axis::getHighPositionLimitWithoutClearance() {
	double highLimit = std::numeric_limits<double>::infinity();
	if (b_enableHighLimit) highLimit = highPositionLimit;
	if (limitToFeedbackWorkingRange) highLimit = std::min(highLimit, getHighFeedbackPositionLimit());
	return highLimit;
}

double Axis::getRange() {
	return getHighPositionLimit() - getLowPositionLimit();
}

double Axis::getLowFeedbackPositionLimit() {
	if (isServoActuatorDeviceConnected()) return getServoActuatorDevice()->getPositionLowerWorkingRangeBound() / servoActuatorUnitsPerAxisUnits;
	else return -std::numeric_limits<double>::infinity();
}

double Axis::getHighFeedbackPositionLimit() {
	if (isServoActuatorDeviceConnected()) {
		auto actuator = getServoActuatorDevice();
		double maxServoPosition = actuator->getPositionUpperWorkingRangeBound();
		return maxServoPosition / servoActuatorUnitsPerAxisUnits;
	}
	else return std::numeric_limits<double>::infinity();
}

double Axis::getActualFollowingError(){
	return servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getFollowingError());
}

float Axis::getActualFollowingErrorNormalized(){
	return getServoActuatorDevice()->getFollowingErrorNormalized();
}

double Axis::getFollowingErrorLimit(){
	return servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getFollowingErrorLimit());
}

void Axis::setCurrentPosition(double distance) {
	auto servo = getServoActuatorDevice();
	servo->overridePosition(distance * servoActuatorUnitsPerAxisUnits);
	motionProfile.setPosition(distance);
}

void Axis::setCurrentPositionAsNegativeLimit() {
	lowPositionLimit = getServoActuatorDevice()->getPosition() / servoActuatorUnitsPerAxisUnits;
}

void Axis::setCurrentPositionAsPositiveLimit() {
	highPositionLimit = getServoActuatorDevice()->getPosition() / servoActuatorUnitsPerAxisUnits;
}

void Axis::scaleFeedbackToMatchPosition(double position_axisUnits) {
	double servoActuatorPosition_actuatorUnits = getServoActuatorDevice()->getPosition();
	//this recalculates unit scaling based on the distance from zero position
	//TODO: is this not working anymore ???
	servoActuatorUnitsPerAxisUnits = servoActuatorPosition_actuatorUnits / position_axisUnits;
	motionProfile.setPosition(position_axisUnits);
}

//================================= MANUAL CONTROL ===================================

void Axis::setVelocityTarget(double velocity_axisUnits) {
	manualVelocityTarget = velocity_axisUnits;
	controlMode = ControlMode::VELOCITY_TARGET;
}

void Axis::fastStop(){
	manualVelocityTarget = 0.0;
	controlMode = ControlMode::FAST_STOP;
}

void Axis::moveToPositionWithVelocity(double targetPosition, double targetVelocity) {
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

void Axis::moveToPositionInTime(double targetPosition, double targetTime) {
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

bool Axis::isHomeable() {
	switch (positionReferenceSignal) {
		case PositionReferenceSignal::NO_SIGNAL:
			return false;
		default:
			return true;
	}
}

void Axis::startHoming() {
	b_isHoming = true;
	homingStep = HomingStep::NOT_STARTED;
	homingError = HomingError::NONE;
}

void Axis::cancelHoming() {
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	controlMode = ControlMode::VELOCITY_TARGET;
	setVelocityTarget(0.0);
}

bool Axis::isHoming() {
	return b_isHoming;
}

bool Axis::didHomingSucceed() {
	return !isHoming() && homingStep == HomingStep::FINISHED;
}

bool Axis::didHomingFail() {
	return homingError != HomingError::NONE;
}

HomingStep Axis::getHomingStep(){
	return homingStep;
}

void Axis::onHomingSuccess() {
	b_isHoming = false;
	homingStep = HomingStep::FINISHED;
	Logger::info("Homing Axis {} Success !", getName());
}

void Axis::onHomingError() {
	b_isHoming = false;
	homingStep = HomingStep::NOT_STARTED;
	disable();
	Logger::info("Homing Axis {} Failure : {}", getName(), Enumerator::getDisplayString(homingError));
}

float Axis::getHomingProgress() {
	
	switch (positionReferenceSignal) {
			
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			
			switch (homingStep) {
				case HomingStep::NOT_STARTED: return 0.0;
				case HomingStep::SEARCHING_LOW_LIMIT_COARSE: return 0.1;
				case HomingStep::FOUND_LOW_LIMIT_COARSE: return 0.3;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE: return 0.5;
				case HomingStep::FOUND_LOW_LIMIT_FINE: return 0.7;
				case HomingStep::RESETTING_POSITION_FEEDBACK: return 0.9;
				case HomingStep::FINISHED: return 1.0;
				default: return 0.0;
			}
			break;
			
		case PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT:
			
			if (homingDirection == HomingDirection::NEGATIVE) {
				
				switch (homingStep) {
					case HomingStep::NOT_STARTED: return 0.0;
					case HomingStep::SEARCHING_LOW_LIMIT_COARSE: return 0.1;
					case HomingStep::FOUND_LOW_LIMIT_COARSE: return 0.2;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE: return 0.3;
					case HomingStep::FOUND_LOW_LIMIT_FINE: return 0.4;
					case HomingStep::RESETTING_POSITION_FEEDBACK: return 0.45;
					case HomingStep::SEARCHING_HIGH_LIMIT_COARSE: return 0.5;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE: return 0.6;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE: return 0.7;
					case HomingStep::FOUND_HIGH_LIMIT_FINE: return 0.8;
					case HomingStep::SETTING_HIGH_LIMIT: return 0.9;
					case HomingStep::FINISHED: return 1.0;
					default: return 0.0;
				}
				
			}
			else if (homingDirection == HomingDirection::POSITIVE) {
				
				switch (homingStep) {
					case HomingStep::NOT_STARTED: return 0.0;
					case HomingStep::SEARCHING_HIGH_LIMIT_COARSE: return 0.1;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE: return 0.2;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE: return 0.3;
					case HomingStep::FOUND_HIGH_LIMIT_FINE: return 0.4;
					case HomingStep::SETTING_HIGH_LIMIT: return 0.45;
					case HomingStep::SEARCHING_LOW_LIMIT_COARSE: return 0.5;
					case HomingStep::FOUND_LOW_LIMIT_COARSE: return 0.6;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE: return 0.7;
					case HomingStep::FOUND_LOW_LIMIT_FINE: return 0.8;
					case HomingStep::RESETTING_POSITION_FEEDBACK: return 0.9;
					case HomingStep::FINISHED: return 1.0;
					default: return 0.0;
				}
				
			}
			
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			
			if (homingDirection == HomingDirection::POSITIVE) {
				
				switch (homingStep) {
					case HomingStep::NOT_STARTED: return 0.0;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE: return 0.1;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE: return 0.2;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE: return 0.4;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE: return 0.5;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE: return 0.7;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE: return 0.9;
					case HomingStep::FINISHED: return 1.0;
					default: return 0.0;
				}
				
			}
			else if (homingDirection == HomingDirection::NEGATIVE) {
				
				switch (homingStep) {
					case HomingStep::NOT_STARTED: return 0.0;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE: return 0.1;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE: return 0.2;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE: return 0.4;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE: return 0.5;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE: return 0.7;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE: return 0.9;
					case HomingStep::FINISHED: return 1.0;
					default: return 0.0;
				}
				
			}
			break;
			
		default: break;
	}
}


void Axis::homingControl(){
	
	switch (positionReferenceSignal) {
			
		case PositionReferenceSignal::SIGNAL_AT_LOWER_LIMIT:
			
			
			switch (homingStep) {
				case HomingStep::NOT_STARTED:
					homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
					Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_COARSE:
					setVelocityTarget(-homingVelocityCoarse);
					//we don't check the signal rising edge but only the high condition
					//this way it triggers if we are already at the limit signal when homing was started
					if (*lowLimitSignal){
						homingStep = HomingStep::FOUND_LOW_LIMIT_COARSE;
						setVelocityTarget(0.0);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					}
					break;
				case HomingStep::FOUND_LOW_LIMIT_COARSE:
					if (!isMoving()) {
						setVelocityTarget(homingVelocityFine);
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					}
					break;
				case HomingStep::SEARCHING_LOW_LIMIT_FINE:
					//here we must check the falling edge
					//since we may have overshot the range of the limit signal
					if (previousLowLimitSignal && !*lowLimitSignal) {
						homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
						setVelocityTarget(0.0);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
					}
					break;
				case HomingStep::FOUND_LOW_LIMIT_FINE:
					if (!isMoving()) {
						auto servoActuator = getServoActuatorDevice();
						servoActuator->overridePosition(0.0);;
						homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						Logger::info("Homing Axis {} : Waiting For Encoder Position Override", getName());
					}
					break;
				case HomingStep::RESETTING_POSITION_FEEDBACK:
					if(!getServoActuatorDevice()->isBusyOverridingPosition()){
						motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
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
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
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
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
							setVelocityTarget(homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE:
						//here we have to check the falling edge since we might have overshot the signal
						if (previousLowLimitSignal && !*lowLimitSignal) {
							homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_FINE:
						if (!isMoving()) {
							getServoActuatorDevice()->overridePosition(0.0);
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							Logger::info("Homing Axis {} : Overriding Position Feedback", getName());
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						//if the servo actuator can hard reset its encoder, check if we are done resetting
						if(!getServoActuatorDevice()->isBusyOverridingPosition()) {
							motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_COARSE;
							setVelocityTarget(homingVelocityCoarse);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
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
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE:
						if (!isMoving()) {
							setVelocityTarget(-homingVelocityFine);
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
						//check the falling edge since we might have overshot the sensor
						if (previousHighLimitSignal && !*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::SETTING_HIGH_LIMIT;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SETTING_HIGH_LIMIT:
						setCurrentPositionAsPositiveLimit();
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
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
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
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_HIGH_LIMIT_FINE;
							setVelocityTarget(-homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_HIGH_LIMIT_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousHighLimitSignal && !*highLimitSignal) {
							homingStep = HomingStep::FOUND_HIGH_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_HIGH_LIMIT_FINE:
						if (!isMoving()) {
							homingStep = HomingStep::SETTING_HIGH_LIMIT;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SETTING_HIGH_LIMIT:
						setCurrentPosition(0.0); //set a zero reference now, we will use it later
						homingStep = HomingStep::SEARCHING_LOW_LIMIT_COARSE;
						setVelocityTarget(-homingVelocityCoarse);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
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
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_LOW_LIMIT_FINE;
							setVelocityTarget(homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_LOW_LIMIT_FINE:
						if (previousLowLimitSignal && !*lowLimitSignal){
							homingStep = HomingStep::FOUND_LOW_LIMIT_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_LOW_LIMIT_FINE:
						if (!isMoving()) {
							highPositionLimit = std::abs(*actualPositionValue);
							getServoActuatorDevice()->overridePosition(0.0);
							Logger::info("Homing Axis {} : Hard Reset Position Feedback", getName());
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						if(!getServoActuatorDevice()->isBusyOverridingPosition()){
							motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
							onHomingSuccess();
						}
						break;
					default:
						break;
				}
			}
			break;
			
		case PositionReferenceSignal::SIGNAL_AT_ORIGIN:
			
			
			if (homingDirection == HomingDirection::POSITIVE) {
				switch (homingStep) {
					case HomingStep::NOT_STARTED:
						homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE;
						setVelocityTarget(homingVelocityCoarse);
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_COARSE:
						//Don't check rising edge since the signal may have been triggered already when homing was started
						if (*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(-homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
						//check for falling edge since the signal may have been overshot
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
						if (!isMoving()) {
							
							switch(signalApproach->value){
								case SignalApproach::FIND_SIGNAL_EDGE:
								{
									auto servoActuator = getServoActuatorDevice();
									servoActuator->overridePosition(0.0);
									motionProfile.setPosition(0.0);
									homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
									Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
									Logger::info("Homing Axis {} : Waiting For Encoder Hard Reset", getName());
									homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
								}
									break;
									
								case SignalApproach::FIND_SIGNAL_CENTER:
									setCurrentPosition(0.0);
									homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
									setVelocityTarget(homingVelocityFine);
									Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
									break;
							}
							
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
						//check for falling edge since the signal might have been overshot
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
						if (!isMoving()) {
							//TODO: need to return to center and hard reset encoder if possible
							setCurrentPosition(*actualPositionValue / 2.0);
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
						Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_COARSE:
						//dont check the rising edge since we might already have triggered the signal at homing start
						if (*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_COARSE:
						if (!isMoving()) {
							homingStep = HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(homingVelocityFine);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_ABOVE_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_ABOVE_FINE:
						if (!isMoving()) {
							setCurrentPosition(0.0);
							
							
							switch(signalApproach->value){
								case SignalApproach::FIND_SIGNAL_EDGE:
									
								{
									
									
									
									if(useFeedbackDevice_Param->value){
										auto feedbackDevice = getFeedbackDevice();
										
										feedbackDevice->overridePosition(0.0);
										motionProfile.setPosition(0.0);
										homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
										Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
										Logger::info("Homing Axis {} : Waiting For hard Reset of {}", getName(), feedbackDevice->getName());
										
									}else{
										auto servoActuator = getServoActuatorDevice();
										//if the servo actuator can hard reset its encoder, do it and wait for the procedure to finish
										servoActuator->overridePosition(0.0);
										motionProfile.setPosition(0.0);
										homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
										Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
										Logger::info("Homing Axis {} : Waiting For Encoder Hard Reset", getName());
										
										homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
									}
									
									
									
									
								}
									
									break;
								case SignalApproach::FIND_SIGNAL_CENTER:
									homingStep = HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE;
									setVelocityTarget(-homingVelocityFine);
									Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
									break;
							}
							
						}
						break;
					case HomingStep::SEARCHING_REFERENCE_FROM_BELOW_FINE:
						//check the falling edge since we might have overshot the signal
						if (previousReferenceSignal && !*referenceSignal) {
							homingStep = HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE;
							setVelocityTarget(0.0);
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::FOUND_REFERENCE_FROM_BELOW_FINE:
						setVelocityTarget(0.0);
						if(!isMoving()){
							moveToPositionWithVelocity(*actualPositionValue / 2.0, homingVelocityCoarse);
							homingStep = HomingStep::MOVING_TO_REFERENCE_MIDDLE;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
						}
						break;
					case HomingStep::MOVING_TO_REFERENCE_MIDDLE:
						if(motionProfile.getInterpolationProgress(profileTime_seconds) >= 1.0){
							auto servoActuator = getServoActuatorDevice();
							//if the servo actuator can hard reset its encoder, do it and wait for the procedure to finish
							
							servoActuator->overridePosition(0.0);
							motionProfile.setPosition(0.0);
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
							Logger::info("Homing Axis {} : {}", getName(), Enumerator::getDisplayString(homingStep));
							Logger::info("Homing Axis {} : Waiting For Encoder Hard Reset", getName());
							
							
							homingStep = HomingStep::RESETTING_POSITION_FEEDBACK;
						}
						break;
					case HomingStep::RESETTING_POSITION_FEEDBACK:
						motionProfile.setPosition(0.0);
						
						
						
						if(useFeedbackDevice_Param->value){
							auto feedbackDevice = getFeedbackDevice();
							
							if(!feedbackDevice->isBusyOverridingPosition()){
								double feedbackPosition = feedbackUnitsToAxisUnits(feedbackDevice->getPosition());
								motionProfile.setPosition(feedbackPosition);
								onHomingSuccess();
							}
							
						}else{
							
							if(!getServoActuatorDevice()->isBusyOverridingPosition()) {
								motionProfile.setPosition(servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getPosition()));
								onHomingSuccess();
							}
						}
						
						
						break;
					default:
						break;
				}
			}
			break;
			
		default:
			homingError = HomingError::HOMING_NOT_SUPORTED;
			onHomingError(); //homing should not be started for modes that don't support homing
			break;
	}
	
}



void Axis::sanitizeParameters(){
	interpolationPositionTarget = std::min(interpolationPositionTarget, getHighPositionLimit());
	interpolationPositionTarget = std::max(interpolationPositionTarget, getLowPositionLimit());
	interpolationVelocityTarget = std::min(std::abs(interpolationVelocityTarget), getVelocityLimit());
	interpolationTimeTarget = std::abs(interpolationTimeTarget);
	
	servoActuatorUnitsPerAxisUnits = std::abs(servoActuatorUnitsPerAxisUnits);
	
	homingVelocityCoarse = std::abs(homingVelocityCoarse);
	homingVelocityFine = std::min(std::abs(homingVelocityFine), homingVelocityCoarse);
	
	velocityLimit = std::abs(velocityLimit);
	if(isServoActuatorDeviceConnected()) velocityLimit = std::min(velocityLimit, servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getVelocityLimit()));
	
	accelerationLimit = std::abs(accelerationLimit);
	if(isServoActuatorDeviceConnected()) accelerationLimit = std::min(accelerationLimit, servoActuatorUnitsToAxisUnits(getServoActuatorDevice()->getAccelerationLimit()));
	
	manualAcceleration = std::min(std::abs(manualAcceleration), accelerationLimit);
	
	lowPositionLimit = std::min(lowPositionLimit, highPositionLimit - highLimitClearance);
	highPositionLimit = std::max(highPositionLimit, lowPositionLimit + lowLimitClearance);
	lowLimitClearance = std::abs(lowLimitClearance);
	highLimitClearance = std::abs(highLimitClearance);
}

//==================================== SAVING AND LOADING =========================================



bool Axis::save(tinyxml2::XMLElement* xml) {
	
	using namespace tinyxml2;
	
	XMLElement* unitsXML = xml->InsertNewChildElement("Units");
	unitsXML->SetAttribute("UnitType", Enumerator::getSaveString(movementType));
	unitsXML->SetAttribute("Unit", positionUnit->saveString);
	
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
	signalApproach->save(positionReferenceXML);
	
	XMLElement* positionLimitsXML = xml->InsertNewChildElement("PositionLimits");
	positionLimitsXML->SetAttribute("LowLimit", lowPositionLimit);
	positionLimitsXML->SetAttribute("EnableLowLimit", b_enableLowLimit);
	positionLimitsXML->SetAttribute("LowLimitClearance", lowLimitClearance);
	positionLimitsXML->SetAttribute("HighLimit", highPositionLimit);
	positionLimitsXML->SetAttribute("EnableHighLimit", b_enableHighLimit);
	positionLimitsXML->SetAttribute("HighLimitClearance", highLimitClearance);
	positionLimitsXML->SetAttribute("LimitToFeedbackWorkingRange", limitToFeedbackWorkingRange);
	
	XMLElement* surveillanceXML = xml->InsertNewChildElement("Surveillance");
	b_isSurveilled->save(surveillanceXML);
	surveillancefeedbackUnitsPerAxisUnits->save(surveillanceXML);
	maxVelocityDeviation->save(surveillanceXML);
	maxSurveillanceErrorClearTime->save(surveillanceXML);
	
	XMLElement* feedbackXML = xml->InsertNewChildElement("Feedback");
	useFeedbackDevice_Param->save(feedbackXML);
	feedbackUnitsPerAxisUnits_Param->save(feedbackXML);
	
	return true;
}



bool Axis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* unitsXML = xml->FirstChildElement("Units");
	if (!unitsXML) return Logger::warn("Could not load Units Attributes");
	const char* axisUnitTypeString;
	if (unitsXML->QueryStringAttribute("UnitType", &axisUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit Type");
	if (!Enumerator::isValidSaveName<MovementType>(axisUnitTypeString)) return Logger::warn("Could not read Machine Unit Type");
	setMovementType(Enumerator::getEnumeratorFromSaveString<MovementType>(axisUnitTypeString));
	const char* axisUnitString;
	if (unitsXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	switch(movementType){
		case MovementType::ROTARY:
			if(!Units::AngularDistance::isValidSaveString(axisUnitString)) return Logger::warn("Could not read Machine Unit");
			break;
		case MovementType::LINEAR:
			if(!Units::LinearDistance::isValidSaveString(axisUnitString)) return Logger::warn("Could not read Machine Unit");
			break;
	}
	setPositionUnit(Units::fromSaveString(axisUnitString));
	
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
	signalApproach->load(positionReferenceXML);
	
	XMLElement* surveillanceXML = xml->FirstChildElement("Surveillance");
	if(surveillanceXML == nullptr){
		Logger::warn("Could not load Surveillance Attribute");
		return false;
	}
	if(!b_isSurveilled->load(surveillanceXML)) return false;
	if(!surveillancefeedbackUnitsPerAxisUnits->load(surveillanceXML)) return false;
	if(!maxVelocityDeviation->load(surveillanceXML)) return false;
	if(!maxSurveillanceErrorClearTime->load(surveillanceXML)) return false;
	
	XMLElement* feedbackXML = xml->FirstChildElement("Feedback");
	if(feedbackXML){
		if(!useFeedbackDevice_Param->load(feedbackXML)) return false;
		if(!feedbackUnitsPerAxisUnits_Param->load(feedbackXML)) return false;
	}else{
		Logger::warn("Could not load Surveillance Attribute");
		return false;
	}
	
	
	return true;
}





void Axis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//if (isServoActuatorDeviceConnected()) output.push_back(getServoActuatorDevice()->parentDevice);
	//if (needsReferenceDevice() && isReferenceDeviceConnected())output.push_back(getReferenceDevice()->parentDevice);
}


void Axis::updateMetrics() {
	positionHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getPosition()));
	actualPositionHistory.addPoint(glm::vec2(profileTime_seconds, *actualPositionValue));
	velocityHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getVelocity()));
	accelerationHistory.addPoint(glm::vec2(profileTime_seconds, motionProfile.getAcceleration()));
	loadHistory.addPoint(glm::vec2(profileTime_seconds, *actualLoadValue));
}

};
