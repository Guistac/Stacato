#include <pch.h>

#include "SharedAxisMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Animation/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/Safety/DeadMansSwitch.h"

void SharedAxisMachine::initialize() {
	//inputs
	addNodePin(axis1Pin);
	addNodePin(axis2Pin);
		
	//outputs
	
	addNodePin(position1Pin);
	addNodePin(velocity1Pin);
	addNodePin(position2Pin);
	addNodePin(velocity2Pin);

	//machine parameters
	addAnimatable(axis1Position);
	addAnimatable(axis2Position);
	
	auto thisMachine = std::static_pointer_cast<SharedAxisMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
}

bool SharedAxisMachine::areAxesConnected() {
	return axis1Pin->isConnected() && axis2Pin->isConnected();
}

std::shared_ptr<PositionControlledAxis> SharedAxisMachine::getAxis1() {
	return axis1Pin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}

std::shared_ptr<PositionControlledAxis> SharedAxisMachine::getAxis2() {
	return axis2Pin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}

void SharedAxisMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == axis1Pin || pin == axis2Pin) updateAnimatableParameters();
}

void SharedAxisMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == axis1Pin || pin == axis2Pin) updateAnimatableParameters();
}

void SharedAxisMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == axis1Pin || pin == axis2Pin) updateAnimatableParameters();
}

bool SharedAxisMachine::isHardwareReady() {
	if (!areAxesConnected()) return false;
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	if(axis1->getState() != MotionState::READY) return false;
	if(axis2->getState() != MotionState::READY) return false;
	return true;
}

bool SharedAxisMachine::isMoving() {
	if (areAxesConnected()) return getAxis1()->isMoving() || getAxis2()->isMoving();
	return false;
}

void SharedAxisMachine::enableHardware() {
	if (isReady()) {
		std::thread machineEnabler([this]() {
			using namespace std::chrono;
			auto axis1 = getAxis1();
			auto axis2 = getAxis2();
			axis1->enable();
			axis2->enable();
			time_point enableRequestTime = system_clock::now();
			while (duration(system_clock::now() - enableRequestTime) < milliseconds(500)) {
				std::this_thread::sleep_for(milliseconds(10));
				if (axis1->getState() == MotionState::ENABLED && axis2->getState() == MotionState::ENABLED) {
					state = MotionState::ENABLED;
					onEnableHardware();
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void SharedAxisMachine::disableHardware() {
	state = MotionState::READY;
	if (areAxesConnected()) {
		getAxis1()->disable();
		getAxis2()->disable();
	}
	onDisableHardware();
}

void SharedAxisMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
}

void SharedAxisMachine::onDisableHardware() {
	Logger::info("Disabled Machine {}", getName());
}

bool SharedAxisMachine::isSimulationReady(){
	return areAxesConnected();
}

void SharedAxisMachine::onEnableSimulation() {
}

void SharedAxisMachine::onDisableSimulation() {
}

std::string SharedAxisMachine::getStatusString(){
	std::string output;
	if(b_halted){
		output = "Machine is Halted";
		return output;
	}
	switch(state){
		case MotionState::OFFLINE:
			if(!areAxesConnected()) return std::string(getName()) + " is Offline : axes not connected";
			if(getAxis1()->getState() == MotionState::OFFLINE) output += "Axis 1 {} is Offline : " + getAxis1()->getStatusString() + "\n";
			if(getAxis2()->getState() == MotionState::OFFLINE) output += "Axis 2 {} is Offline : " + getAxis2()->getStatusString() + "\n";
			return output;
		case MotionState::NOT_READY:
			output = std::string(getName()) + " is not ready\n";
			if(getAxis1()->getState() == MotionState::NOT_READY) output += "Axis 1 {} is not ready : " + getAxis1()->getStatusString() + "\n";
			if(getAxis2()->getState() == MotionState::NOT_READY) output += "Axis 2 {} is not ready : " + getAxis2()->getStatusString() + "\n";
			return output;
		case MotionState::READY: return std::string(getName()) + " is Not Enabled";
		case MotionState::ENABLED: return std::string(getName()) + " is Enabled";
	}
}

void SharedAxisMachine::inputProcess() {
	if (!areAxesConnected()) {
		state = MotionState::OFFLINE;
		b_halted = false;
		b_emergencyStopActive = false;
		return;
	}
	
	/*
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
	 */
}

void SharedAxisMachine::outputProcess(){
	
	/*
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
	 */
}

void SharedAxisMachine::simulateInputProcess() {
	/*
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
	 */
}

void SharedAxisMachine::simulateOutputProcess(){
	/*
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	 
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	animatablePosition->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
	auto target = animatablePosition->getTargetValue()->toPosition();
	animatablePosition->updateActualValue(target);
	*positionPinValue = target->position;
	*velocityPinValue = target->velocity;
	 */
}

bool SharedAxisMachine::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

bool SharedAxisMachine::isHoming(){
	//return isAxisConnected() && getAxis()->isHoming();
}
void SharedAxisMachine::startHoming(){
	/*
	animatablePosition->stopAnimation();
	getAxis()->startHoming();
	 */
}
void SharedAxisMachine::stopHoming(){
	//getAxis()->cancelHoming();
}
bool SharedAxisMachine::didHomingSucceed(){
	//return isAxisConnected() && getAxis()->didHomingSucceed();
}
bool SharedAxisMachine::didHomingFail(){
	//return isAxisConnected() && getAxis()->didHomingFail();
}
float SharedAxisMachine::getHomingProgress(){
	//return getAxis()->getHomingProgress();
}
const char* SharedAxisMachine::getHomingStateString(){
	/*
	if(!isAxisConnected()) return "No axis is connected to machine";
	return Enumerator::getDisplayString(getAxis()->getHomingStep());
	 */
}







void SharedAxisMachine::updateAnimatableParameters(){
	/*
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
	 */
}








//========= ANIMATABLE OWNER ==========

void SharedAxisMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
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



void SharedAxisMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//if (isAxisConnected()) getAxis()->getDevices(output);
}



bool SharedAxisMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	/*
	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", animatablePosition->rapidVelocity);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", animatablePosition->rapidAcceleration);
	 
	XMLElement* machineZeroXML = xml->InsertNewChildElement("MachineZero");
	machineZeroXML->SetAttribute("MachineZero_AxisUnits", machineZero_axisUnits);
	machineZeroXML->SetAttribute("InvertAxisDirection", b_invertDirection);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	*/
	return true;
}


bool SharedAxisMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	/*

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
	 */
	return true;
}




void SharedAxisMachine::captureMachineZero(){
	//machineZero_axisUnits = motionProfile.getPosition();
}




double SharedAxisMachine::axisPositionToMachinePosition(double axisPosition){
	if(b_invertDirection) return -1.0f * (axisPosition - machineZero_axisUnits);
	return axisPosition - machineZero_axisUnits;
}

double SharedAxisMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(b_invertDirection) return axisVelocity * -1.0;
	return axisVelocity;
}

double SharedAxisMachine::axisAccelerationToMachineAcceleration(double axisAcceleration){
	if(b_invertDirection) return axisAcceleration * -1.0;
	return axisAcceleration;
}

double SharedAxisMachine::machinePositionToAxisPosition(double machinePosition){
	if(b_invertDirection) return (-1.0f * machinePosition) + machineZero_axisUnits;
	return machinePosition + machineZero_axisUnits;
}

double SharedAxisMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(b_invertDirection) return machineVelocity * -1.0;
	return machineVelocity;
}

double SharedAxisMachine::machineAccelerationToAxisAcceleration(double machineAcceleration){
	if(b_invertDirection) return machineAcceleration * -1.0;
	return machineAcceleration;
}
