#include <pch.h>

#include "VelocityControlledAxis.h"

#include "Motion/SubDevice.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"

void VelocityControlledAxis::initialize() {
	//inputs
	addNodePin(actuatorPin);
	addNodePin(gpioPin);
	addNodePin(lowLimitPin);
	addNodePin(lowSlowdownPin);
	addNodePin(highLimitPin);
	addNodePin(highSlowdownPin);
	
	//outputs
	std::shared_ptr<VelocityControlledAxis> thisAxis = std::static_pointer_cast<VelocityControlledAxis>(shared_from_this());
	velocityControlledAxisPin->assignData(thisAxis);
	addNodePin(velocityControlledAxisPin);
	addNodePin(velocityPin);
	addNodePin(loadPin);
}

std::string VelocityControlledAxis::getStatusString(){
	switch(state){
		case MotionState::OFFLINE:
			if(!isActuatorDeviceConnected()) return "Actuator device pin is not connected";
			else return "Actuator is Offline : " + getActuatorDevice()->getStatusString();
		case MotionState::NOT_READY:
			return "Actuator is not ready : " + getActuatorDevice()->getStatusString();
		case MotionState::READY:	return "Axis is ready to enable";
		case MotionState::ENABLED: 	return "Axis is enabled";
	}
}

bool VelocityControlledAxis::areAllPinsConnected(){
	if(!isActuatorDeviceConnected()) 	return false;
	if(!gpioPin->isConnected()) 		return false;
	if(!lowLimitPin->isConnected()) 	return false;
	if(!lowSlowdownPin->isConnected()) 	return false;
	if(!highLimitPin->isConnected()) 	return false;
	if(!highSlowdownPin->isConnected()) return false;
	return true;
}

bool VelocityControlledAxis::areAllDeviceReady(){
	for(auto referenceDevicePin : gpioPin->getConnectedPins()){
		auto gpioDevice = referenceDevicePin->getSharedPointer<GpioDevice>();
		if(!gpioDevice->isEnabled()) return false;
	}
	if(!getActuatorDevice()->isReady()) return false;
}

void VelocityControlledAxis::updateAxisState(){
	MotionState newAxisState = MotionState::ENABLED;
	auto checkState = [&](MotionState deviceState){ if(int(deviceState) < int(newAxisState)) newAxisState = deviceState; };
		
	auto actuatorDevice = getActuatorDevice();
	
	for(auto gpioDevicePin : gpioPin->getConnectedPins()){
		auto gpioDeviceState = gpioDevicePin->getSharedPointer<GpioDevice>()->getState();
		checkState(gpioDeviceState);
	}
	checkState(actuatorDevice->getState());
	
	//handle transition from enabled state
	if(state == MotionState::ENABLED && newAxisState != MotionState::ENABLED) disable();
	state = newAxisState;
	
	//update estop state
	b_emergencyStopActive = actuatorDevice->isEmergencyStopped();
	
	//get actual realtime axis motion values
	*actualVelocity = motionProfile.getVelocity();
	*actualLoad = actuatorDevice->getLoad();
}

void VelocityControlledAxis::updateReferenceSignals(){
	lowLimitPin->copyConnectedPinValue();
	lowSlowdownPin->copyConnectedPinValue();
	highLimitPin->copyConnectedPinValue();
	highSlowdownPin->copyConnectedPinValue();
}

//called by node connected to axis pin to send velocity to the axis
void VelocityControlledAxis::setVelocityCommand(double velocity, double acceleration){
	externalVelocityCommand = velocity;
	externalAccelerationCommand = acceleration;
	controlMode = ControlMode::EXTERNAL;
}

void VelocityControlledAxis::inputProcess() {
	//check connection requirements and abort processing if the requirements are not met
	if(!areAllPinsConnected()) {
		state = MotionState::OFFLINE;
		b_emergencyStopActive = false;
		return;
	}	
	updateAxisState();
	updateReferenceSignals();
}
	
void VelocityControlledAxis::outputProcess(){
	
	//TODO: abort if no pins connected
	
	//update timing
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileTimeDelta_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
    double velocityTarget = 0.0;
    double accelerationTarget = 0.0;
	switch(controlMode){
		case ControlMode::VELOCITY_TARGET:
			velocityTarget = manualVelocityTarget;
			if(abs(velocityTarget) > abs(motionProfile.getVelocity())) accelerationTarget = manualAcceleration->value;
			else if(abs(velocityTarget) < abs(motionProfile.getVelocity())) accelerationTarget = manualDeceleration->value;
			else accelerationTarget = 0.0;
			break;
		case ControlMode::FAST_STOP:
			velocityTarget = 0.0;
			accelerationTarget = decelerationLimit->value;
			break;
		case ControlMode::EXTERNAL:
			velocityTarget = externalVelocityCommand;
			accelerationTarget = externalAccelerationCommand;
			break;
		default:
			velocityTarget = 0.0;
			accelerationTarget = 0.0;
			break;
	}

    
	if((motionProfile.getVelocity() < 0.0 || velocityTarget < 0.0) && *lowLimitSignal){
		motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, decelerationLimit->value);
	}
	else if((motionProfile.getVelocity() > 0.0 || velocityTarget > 0.0) && *highLimitSignal){
		motionProfile.matchVelocity(profileTimeDelta_seconds, 0.0, decelerationLimit->value);
	}
	else if((motionProfile.getVelocity() < -slowdownVelocity->value || velocityTarget < -slowdownVelocity->value) && *lowSlowdownSignal){
		motionProfile.matchVelocity(profileTimeDelta_seconds, -slowdownVelocity->value, decelerationLimit->value);
	}
	else if((motionProfile.getVelocity() > slowdownVelocity->value || velocityTarget > slowdownVelocity->value) && *highSlowdownSignal){
		motionProfile.matchVelocity(profileTimeDelta_seconds, slowdownVelocity->value, decelerationLimit->value);
	}
	else{
        
        bool b_sameSign = velocityTarget * motionProfile.getVelocity() >= 0.0;
        
        //condition for acceleration abs(tar) > abs(act) && tar & act have same sign
        if(std::abs(velocityTarget) > std::abs(motionProfile.getVelocity()) && b_sameSign){
            //accelerate
            accelerationTarget = std::min(accelerationTarget, accelerationLimit->value);
            motionProfile.matchVelocity(profileTimeDelta_seconds, velocityTarget, accelerationTarget);
        }
        else if(velocityTarget == motionProfile.getVelocity()){
            //constant velocity
            motionProfile.matchVelocity(profileTime_seconds, velocityTarget, 0.0);
        }
        else{
            //decelerate
            accelerationTarget = std::min(accelerationTarget, decelerationLimit->value);
            motionProfile.matchVelocity(profileTimeDelta_seconds, velocityTarget, accelerationTarget);
        }
	}
		
	//send commands to the actuator
	double actuatorVelocity = axisUnitsToActuatorUnits(motionProfile.getVelocity());
	double actuatorAcceleration = axisUnitsToActuatorUnits(motionProfile.getAcceleration());
	getActuatorDevice()->setVelocityCommand(actuatorVelocity, actuatorAcceleration);
}

void VelocityControlledAxis::sendActuatorCommands() {
	*actualVelocity = motionProfile.getVelocity();
	double velocity_actuatorUnits = axisUnitsToActuatorUnits(motionProfile.getVelocity());
	double acceleration_actuatorUnits = axisUnitsToActuatorUnits(motionProfile.getAcceleration());
	getActuatorDevice()->setVelocityCommand(velocity_actuatorUnits, acceleration_actuatorUnits);
	actuatorPin->updateConnectedPins();
}

bool VelocityControlledAxis::isAtUpperLimit(){
	return *highLimitSignal && motionProfile.getVelocity() == 0.0;
}

bool VelocityControlledAxis::isAtLowerLimit(){
	return *lowLimitSignal && motionProfile.getVelocity() == 0.0;
}



void VelocityControlledAxis::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isActuatorDeviceConnected()) output.push_back(getActuatorDevice()->parentDevice);
}

bool VelocityControlledAxis::isMoving() {
	return motionProfile.getVelocity() != 0.0;
}

void VelocityControlledAxis::enable() {
	if(state != MotionState::READY) return;
	std::thread axisEnabler([this]() {
		using namespace std::chrono;
		system_clock::time_point enableTime = system_clock::now();
		
		std::shared_ptr<ActuatorDevice> actuator = getActuatorDevice();
		actuator->enable();
		while(system_clock::now() - enableTime < milliseconds(500)){
			if(actuator->isEnabled()){
				state = MotionState::ENABLED;
				onEnable();
				Logger::info("Velocity Controlled Axis '{}' Enabled", getName());
				return;
			}
			std::this_thread::sleep_for(milliseconds(10));
		}
		state = MotionState::NOT_READY;
		actuator->disable();
		Logger::warn("Could not enable velocity controlled axis '{}'", getName());
	});
	axisEnabler.detach();
}

void VelocityControlledAxis::disable() {
	getActuatorDevice()->disable();
	state = MotionState::NOT_READY;
    onDisable();
}

void VelocityControlledAxis::onEnable() {
    motionProfile.setPosition(0.0);
    motionProfile.setVelocity(0.0);
    motionProfile.setAcceleration(0.0);
}
void VelocityControlledAxis::onDisable() {
    motionProfile.setPosition(0.0);
    motionProfile.setVelocity(0.0);
    motionProfile.setAcceleration(0.0);
}

void VelocityControlledAxis::setVelocity(double velocity) {
	controlMode = ControlMode::VELOCITY_TARGET;
	manualVelocityTarget = velocity;
}

void VelocityControlledAxis::fastStop(){
	controlMode = ControlMode::FAST_STOP;
	manualVelocityTarget = 0.0;
}

void VelocityControlledAxis::setMovementType(MovementType type){
	movementType = type;
	switch(type){
		case MovementType::ROTARY:
			if(positionUnit->unitType != Units::Type::ANGULAR_DISTANCE){
				setPositionUnit(Units::AngularDistance::get().front());
			}
			break;
		case MovementType::LINEAR:
			if(positionUnit->unitType != Units::Type::LINEAR_DISTANCE){
				setPositionUnit(Units::LinearDistance::get().front());
			}
			break;
	}
}


void VelocityControlledAxis::setPositionUnit(Unit unit){
	positionUnit = unit;
}


void VelocityControlledAxis::sanitizeParameters(){
	velocityLimit->validateRange(0.0, actuatorUnitsToAxisUnits(getActuatorDevice()->getVelocityLimit()), true, true);
	accelerationLimit->validateRange(0.0, actuatorUnitsToAxisUnits(getActuatorDevice()->getAccelerationLimit()), true, true);
	decelerationLimit->validateRange(0.0, actuatorUnitsToAxisUnits(getActuatorDevice()->getDecelerationLimit()), true, true);
	manualAcceleration->validateRange(0.0, accelerationLimit->value, true, true);
	manualDeceleration->validateRange(0.0, decelerationLimit->value, true, true);
	slowdownVelocity->validateRange(0.0, velocityLimit->value, true, true);
}


bool VelocityControlledAxis::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->InsertNewChildElement("Axis");
	axisXML->SetAttribute("UnitType", Enumerator::getSaveString(movementType));
	axisXML->SetAttribute("Unit", positionUnit->saveString);
	
	actuatorUnitsPerAxisUnits->save(axisXML);

	XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("Limits");
	velocityLimit->save(kinematicLimitsXML);
	accelerationLimit->save(kinematicLimitsXML);
	decelerationLimit->save(kinematicLimitsXML);
	manualAcceleration->save(kinematicLimitsXML);
	manualDeceleration->save(kinematicLimitsXML);
	slowdownVelocity->save(kinematicLimitsXML);

	return true;
}



bool VelocityControlledAxis::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* axisXML = xml->FirstChildElement("Axis");
	if (!axisXML) return Logger::warn("Could not load Machine Attributes");
	const char* axisUnitTypeString;
	if (axisXML->QueryStringAttribute("UnitType", &axisUnitTypeString) != XML_SUCCESS) return Logger::warn("Could not load Axis Unit Type");
	if (!Enumerator::isValidSaveName<MovementType>(axisUnitTypeString)) return Logger::warn("Could not read Machine Unit Type");
	movementType = Enumerator::getEnumeratorFromSaveString<MovementType>(axisUnitTypeString);
	const char* axisUnitString;
	if (axisXML->QueryStringAttribute("Unit", &axisUnitString) != XML_SUCCESS) return Logger::warn("Could not load Machine Unit");
	switch(movementType){
		case MovementType::ROTARY:
			if(!Units::AngularDistance::isValidSaveString(axisUnitString)) return Logger::warn("Could not read Machine Unit");
			break;
		case MovementType::LINEAR:
			if(!Units::LinearDistance::isValidSaveString(axisUnitString)) return Logger::warn("Could not read Machine Unit");
			break;
	}
	positionUnit = Units::fromSaveString(axisUnitString);

	if(!actuatorUnitsPerAxisUnits->load(axisXML)) return false;

	XMLElement* kinematicLimitsXML = xml->FirstChildElement("Limits");
	if (!kinematicLimitsXML) return Logger::warn("Could not load Axis Kinematic Kimits");
	
	if(!velocityLimit->load(kinematicLimitsXML)) return false;
	if(!accelerationLimit->load(kinematicLimitsXML)) return false;
	if(!decelerationLimit->load(kinematicLimitsXML)) return false;
	if(!manualAcceleration->load(kinematicLimitsXML)) return false;
	if(!manualDeceleration->load(kinematicLimitsXML)) return false;
	if(!slowdownVelocity->load(kinematicLimitsXML)) return false;
	return true;
}
