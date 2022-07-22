#include <pch.h>

#include "PositionControlledMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Animation/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/Safety/DeadMansSwitch.h"

void PositionControlledMachine::initialize() {
	//inputs
	addNodePin(positionControlledAxisPin);
		
	//outputs
	positionPin->assignData(positionPinValue);
	addNodePin(positionPin);
	velocityPin->assignData(velocityPinValue);
	addNodePin(velocityPin);

	//machine parameters
	addAnimatable(animatablePosition);
	
	auto thisMachine = std::static_pointer_cast<PositionControlledMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
}

bool PositionControlledMachine::isAxisConnected() {
	return positionControlledAxisPin->isConnected();
}

std::shared_ptr<PositionControlledAxis> PositionControlledMachine::getAxis() {
	return positionControlledAxisPin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}

void PositionControlledMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) updateAnimatableParameters();
}

void PositionControlledMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) updateAnimatableParameters();
}

void PositionControlledMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) updateAnimatableParameters();
}

bool PositionControlledMachine::isHardwareReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	if (!axis->isReady()) return false;
	return true;
}

bool PositionControlledMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void PositionControlledMachine::enableHardware() {
	if (isReady()) {
		std::thread machineEnabler([this]() {
			using namespace std::chrono;
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			axis->enable();
			time_point enableRequestTime = system_clock::now();
			while (duration(system_clock::now() - enableRequestTime) < milliseconds(500)) {
				std::this_thread::sleep_for(milliseconds(10));
				if (axis->getState() == MotionState::ENABLED) {
					state = MotionState::ENABLED;
					//b_enabled = true;
					onEnableHardware();
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void PositionControlledMachine::disableHardware() {
	//b_enabled = false;
	state = MotionState::READY;
	if (isAxisConnected()) getAxis()->disable();
	onDisableHardware();
}

void PositionControlledMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
}

void PositionControlledMachine::onDisableHardware() {
	Logger::info("Disabled Machine {}", getName());
}

bool PositionControlledMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledMachine::onEnableSimulation() {
}

void PositionControlledMachine::onDisableSimulation() {
}

std::string PositionControlledMachine::getStatusString(){
	if(b_halted){
		std::string output = "Machine is Halted";
		return output;
	}
	switch(state){
		case MotionState::OFFLINE:
			if(!isAxisConnected()) return std::string(getName()) + " is Offline : no axis connected";
			else return "Axis is Offline : " + getAxis()->getStatusString();
		case MotionState::NOT_READY:
			return "Axis is not ready : " + getAxis()->getStatusString();
		case MotionState::READY: return std::string(getName()) + " is Not Enabled";
		case MotionState::ENABLED: return std::string(getName()) + " is Enabled";
	}
}

void PositionControlledMachine::inputProcess() {
	if (!isAxisConnected()) {
		state = MotionState::OFFLINE;
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	
	//update machine state
	if (isEnabled() && axis->getState() != MotionState::ENABLED) disable();
	else state = axis->getState();
	
	//update estop state
	b_emergencyStopActive = axis->isEmergencyStopActive() && axis->getState() != MotionState::OFFLINE;
	
	//update halt state
	b_halted = isEnabled() && !isMotionAllowed();
	
	//update animatable state
	if(state == MotionState::OFFLINE) animatablePosition->state = Animatable::State::OFFLINE;
	else if(state == MotionState::ENABLED && !b_halted) animatablePosition->state = Animatable::State::READY;
	else animatablePosition->state = Animatable::State::NOT_READY;
	
	auto actualPosition = AnimationValue::makePosition();
	actualPosition->position = axisPositionToMachinePosition(axis->getActualPosition());
	actualPosition->velocity = axisVelocityToMachineVelocity(axis->getActualVelocity());
	actualPosition->acceleration = axisAccelerationToMachineAcceleration(0.0);
	animatablePosition->updateActualValue(actualPosition);
	
	//Get Realtime values from axis (for position and velocity pins only)
	*positionPinValue = actualPosition->position;
	*velocityPinValue = actualPosition->velocity;
}

void PositionControlledMachine::outputProcess(){
	
	if(!isMotionAllowed()){
		if(animatablePosition->hasAnimation()){
			animatablePosition->getAnimation()->pausePlayback();
		}
		animatablePosition->stopMovement();
	}
	
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	if (!isEnabled() || isHoming()) {
		//if the axis is not enabled or is homing, the animatable doesn't do anything
		animatablePosition->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
	}
	else{
		//else we update the animatable, get its target and send it to the axis
		animatablePosition->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		auto target = animatablePosition->getTargetValue()->toPosition();
		
		double axisPositionTarget = machinePositionToAxisPosition(target->position);
		double axisVelocityTarget = machineVelocityToAxisVelocity(target->velocity);
		double axisAccelerationTarget = machineAccelerationToAxisAcceleration(target->acceleration);
		
		getAxis()->setMotionCommand(axisPositionTarget,
									axisVelocityTarget,
									axisAccelerationTarget);
	}
}

void PositionControlledMachine::simulateInputProcess() {
	//update machine state
	if(isEnabled()) state = MotionState::ENABLED;
	else if(isAxisConnected()) state = MotionState::READY;
	else state = MotionState::OFFLINE;
	
	//update halt and estop state
	b_emergencyStopActive = false;
	b_halted = false;
		
	//update animatable state
	if(state == MotionState::OFFLINE) animatablePosition->state = Animatable::State::OFFLINE;
	else if(isEnabled() && !b_halted) animatablePosition->state = Animatable::State::READY;
	else animatablePosition->state = Animatable::State::NOT_READY;
}

void PositionControlledMachine::simulateOutputProcess(){
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	 
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	animatablePosition->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
	auto target = animatablePosition->getTargetValue()->toPosition();
	animatablePosition->updateActualValue(target);
	*positionPinValue = target->position;
	*velocityPinValue = target->velocity;
}

bool PositionControlledMachine::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

bool PositionControlledMachine::isHoming(){
	return isAxisConnected() && getAxis()->isHoming();
}
void PositionControlledMachine::startHoming(){
	animatablePosition->stopAnimation();
	getAxis()->startHoming();
}
void PositionControlledMachine::stopHoming(){
	getAxis()->cancelHoming();
}
bool PositionControlledMachine::didHomingSucceed(){
	return isAxisConnected() && getAxis()->didHomingSucceed();
}
bool PositionControlledMachine::didHomingFail(){
	return isAxisConnected() && getAxis()->didHomingFail();
}
float PositionControlledMachine::getHomingProgress(){
	return getAxis()->getHomingProgress();
}
const char* PositionControlledMachine::getHomingStateString(){
	if(!isAxisConnected()) return "No axis is connected to machine";
	return Enumerator::getDisplayString(getAxis()->getHomingStep());
}







void PositionControlledMachine::updateAnimatableParameters(){
	if(!isAxisConnected()) {
		animatablePosition->setUnit(Units::None::None);
		animatablePosition->lowerPositionLimit = 0.0;
		animatablePosition->upperPositionLimit = 0.0;
		animatablePosition->velocityLimit = 0.0;
		animatablePosition->accelerationLimit = 0.0;
		animatablePosition->rapidVelocity = 0.0;
		animatablePosition->rapidAcceleration = 0.0;
		return;
	}
	auto axis = getAxis();
	animatablePosition->setUnit(axis->getPositionUnit());
	animatablePosition->lowerPositionLimit = axisPositionToMachinePosition(axis->getLowPositionLimit());
	animatablePosition->upperPositionLimit = axisPositionToMachinePosition(axis->getHighPositionLimit());
	animatablePosition->velocityLimit = axisVelocityToMachineVelocity(axis->getVelocityLimit());
	animatablePosition->accelerationLimit = axisAccelerationToMachineAcceleration(axis->getAccelerationLimit());
}








//========= ANIMATABLE OWNER ==========

void PositionControlledMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	/*
	if(animation->getAnimatable() != animatablePosition) return;
	switch(animation->getType()){
		case ManoeuvreType::KEY:
			animation->toKey()->captureTarget();
			break;
		case ManoeuvreType::TARGET:
			animation->toTarget()->captureTarget();
			animation->toTarget()->inAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			animation->toTarget()->outAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			animation->toTarget()->velocityConstraint->overwrite(rapidVelocity_machineUnitsPerSecond);
			animation->toTarget()->timeConstraint->overwrite(0.0);
			animation->toTarget()->constraintType->overwrite(TargetAnimation::Constraint::TIME);
			break;
		case ManoeuvreType::SEQUENCE:
			animation->toSequence()->captureStart();
			animation->toSequence()->captureTarget();
			animation->toSequence()->duration->overwrite(0.0);
			animation->toSequence()->inAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			animation->toSequence()->outAcceleration->overwrite(rapidAcceleration_machineUnitsPerSecond);
			break;
	}
	*/
}



void PositionControlledMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}



bool PositionControlledMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", animatablePosition->rapidVelocity);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", animatablePosition->rapidAcceleration);
	 
	XMLElement* machineZeroXML = xml->InsertNewChildElement("MachineZero");
	machineZeroXML->SetAttribute("MachineZero_AxisUnits", machineZero_axisUnits);
	machineZeroXML->SetAttribute("InvertAxisDirection", b_invertDirection);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}


bool PositionControlledMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if (rapidsXML == nullptr) return Logger::warn("Could not find Rapids attribute");
	if (rapidsXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &animatablePosition->rapidVelocity) != XML_SUCCESS) return Logger::warn("Could find rapid velocity attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &animatablePosition->rapidAcceleration) != XML_SUCCESS) return Logger::warn("Could find rapid acceleration attribute");
	 
	XMLElement* machineZeroXML = xml->FirstChildElement("MachineZero");
	if(machineZeroXML == nullptr) return Logger::warn("Could not find Machine Zero Attribute");
	if(machineZeroXML->QueryDoubleAttribute("MachineZero_AxisUnits", &machineZero_axisUnits) != XML_SUCCESS) return Logger::warn("Could not find machine zero value attribute");
	if(machineZeroXML->QueryBoolAttribute("InvertAxisDirection", &b_invertDirection) != XML_SUCCESS) return Logger::warn("Could not find invert axis direction attribute");
	
	XMLElement* widgetXML = xml->FirstChildElement("ControWidget");
	if(widgetXML == nullptr) return Logger::warn("Could not find Control Widget Attribute");
	if(widgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS) return Logger::warn("Could not find machine control widget uid attribute");
	 
	return true;
}




void PositionControlledMachine::captureMachineZero(){
	//machineZero_axisUnits = motionProfile.getPosition();
}




double PositionControlledMachine::axisPositionToMachinePosition(double axisPosition){
	if(b_invertDirection) return -1.0f * (axisPosition - machineZero_axisUnits);
	return axisPosition - machineZero_axisUnits;
}

double PositionControlledMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(b_invertDirection) return axisVelocity * -1.0;
	return axisVelocity;
}

double PositionControlledMachine::axisAccelerationToMachineAcceleration(double axisAcceleration){
	if(b_invertDirection) return axisAcceleration * -1.0;
	return axisAcceleration;
}

double PositionControlledMachine::machinePositionToAxisPosition(double machinePosition){
	if(b_invertDirection) return (-1.0f * machinePosition) + machineZero_axisUnits;
	return machinePosition + machineZero_axisUnits;
}

double PositionControlledMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(b_invertDirection) return machineVelocity * -1.0;
	return machineVelocity;
}

double PositionControlledMachine::machineAccelerationToAxisAcceleration(double machineAcceleration){
	if(b_invertDirection) return machineAcceleration * -1.0;
	return machineAcceleration;
}
