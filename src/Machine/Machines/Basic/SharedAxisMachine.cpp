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
	addAnimatable(axis1Animatable);
	addAnimatable(axis2Animatable);
	if(enableSynchronousControl->value) addAnimatable(synchronizedAnimatable);
	
	auto thisMachine = std::static_pointer_cast<SharedAxisMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
	
	
	velocityLimit->setEditCallback([this](std::shared_ptr<Parameter> p){
		double axisVelocityLimit = std::min(getAxis1()->getVelocityLimit(), getAxis2()->getVelocityLimit());
		velocityLimit->validateRange(0, axisVelocityLimit, true, true);
		updateAnimatableParameters();
	});
	accelerationLimit->setEditCallback([this](std::shared_ptr<Parameter> p){
		double axisAccelerationLimit = std::min(getAxis1()->getAccelerationLimit(), getAxis2()->getAccelerationLimit());
		accelerationLimit->validateRange(0, axisAccelerationLimit, true, true);
		updateAnimatableParameters();
	});
	enableSynchronousControl->setEditCallback([this](std::shared_ptr<Parameter> p){
		if(enableSynchronousControl->value) addAnimatable(synchronizedAnimatable);
		else removeAnimatable(synchronizedAnimatable);
	});
	
	
}

bool SharedAxisMachine::areAxesConnected() {
	return axis1Pin->isConnected() && axis2Pin->isConnected();
}

bool SharedAxisMachine::axesHaveSamePositionUnit(){
	return getAxis1()->getPositionUnit() == getAxis2()->getPositionUnit();
}

void SharedAxisMachine::updateAxisParameters(){
	if(!areAxesConnected()) return;
	if(getAxis1()->getPositionUnit() != getAxis2()->getPositionUnit()) return;
	
	positionUnit = getAxis1()->getPositionUnit();
	
	velocityLimit->setUnit(positionUnit);
	accelerationLimit->setUnit(positionUnit);
	axis1Offset->setUnit(positionUnit);
	axis2Offset->setUnit(positionUnit);
	minimumDistanceBetweenAxes->setUnit(positionUnit);

	velocityLimit->onEdit();
	accelerationLimit->onEdit();
	
	axis1Animatable->setName(std::string(getAxis1()->getName()));
	axis2Animatable->setName(std::string(getAxis2()->getName()));
}


void SharedAxisMachine::updateAnimatableParameters(){
	if(!areAxesConnected()) {
		axis1Animatable->setUnit(Units::None::None);
		axis1Animatable->velocityLimit = 0.0;
		axis2Animatable->setUnit(Units::None::None);
		axis2Animatable->velocityLimit = 0.0;
		synchronizedAnimatable->setUnit(Units::None::None);
		synchronizedAnimatable->velocityLimit = 0.0;
		return;
	}
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	axis1Animatable->setUnit(positionUnit);
	axis2Animatable->setUnit(positionUnit);
	synchronizedAnimatable->setUnit(positionUnit);
	
	axis1Animatable->lowerPositionLimit = axis1PositionToMachinePosition(axis1->getLowPositionLimit());
	axis2Animatable->lowerPositionLimit = axis2PositionToMachinePosition(axis2->getLowPositionLimit());
	synchronizedAnimatable->lowerPositionLimit = axis1isMaster->value ? axis1Animatable->lowerPositionLimit : axis2Animatable->lowerPositionLimit;
	
	axis1Animatable->upperPositionLimit = axis1PositionToMachinePosition(axis1->getHighPositionLimit());
	axis2Animatable->upperPositionLimit = axis2PositionToMachinePosition(axis2->getHighPositionLimit());
	synchronizedAnimatable->upperPositionLimit = axis1isMaster->value ? axis1Animatable->upperPositionLimit : axis2Animatable->upperPositionLimit;
	
	axis1Animatable->velocityLimit = velocityLimit->value;
	axis2Animatable->velocityLimit = velocityLimit->value;
	synchronizedAnimatable->velocityLimit = velocityLimit->value;
	
	axis1Animatable->accelerationLimit = accelerationLimit->value;
	axis2Animatable->accelerationLimit = accelerationLimit->value;
	synchronizedAnimatable->accelerationLimit = accelerationLimit->value;
}




std::shared_ptr<PositionControlledAxis> SharedAxisMachine::getAxis1() {
	return axis1Pin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}

std::shared_ptr<PositionControlledAxis> SharedAxisMachine::getAxis2() {
	return axis2Pin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}

void SharedAxisMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == axis1Pin || pin == axis2Pin) {
		updateAxisParameters();
		updateAnimatableParameters();
	}
}

void SharedAxisMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == axis1Pin || pin == axis2Pin) {
		updateAxisParameters();
		updateAnimatableParameters();
	}
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
			if(!axesHaveSamePositionUnit()) return std::string(getName()) + " is Offline : axe don't have the same position unit";
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
	if (!areAxesConnected() || !axesHaveSamePositionUnit()) {
		state = MotionState::OFFLINE;
		b_halted = false;
		b_emergencyStopActive = false;
		return;
	}
	
	//update machine state
	MotionState newState = MotionState::ENABLED;
	auto checkState = [&](MotionState checkedState){ if(int(checkedState) < int(newState)) newState = checkedState; };
	
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	checkState(axis1->getState());
	checkState(axis2->getState());
	
	//handle transition from enabled state
	if(state == MotionState::ENABLED && newState != MotionState::ENABLED) disable();
	state = newState;
	
	//update estop state
	b_emergencyStopActive = axis1->isEmergencyStopActive() || axis2->isEmergencyStopActive();
	
	//update real position, velocity, acceleration
	auto axis1RealPosition = AnimationValue::makePosition();
	axis1RealPosition->position = axis1PositionToMachinePosition(axis1->getActualPosition());
	axis1RealPosition->velocity = axis1ToMachineConversion(axis1->getActualVelocity());
	axis1RealPosition->acceleration = axis1ToMachineConversion(0.0);
	axis1Animatable->updateActualValue(axis1RealPosition);
	
	auto axis2RealPosition = AnimationValue::makePosition();
	axis2RealPosition->position = axis2PositionToMachinePosition(axis2->getActualPosition());
	axis2RealPosition->velocity = axis2ToMachineConversion(axis2->getActualVelocity());
	axis2RealPosition->acceleration = axis2ToMachineConversion(0.0);
	axis2Animatable->updateActualValue(axis2RealPosition);
	
	synchronizedAnimatable->updateActualValue(axis1isMaster->value ? axis1RealPosition : axis2RealPosition);
	
	*position1PinValue = axis1RealPosition->position;
	*velocity1PinValue = axis1RealPosition->velocity;
	*position2PinValue = axis2RealPosition->position;
	*velocity2PinValue = axis2RealPosition->velocity;
	
	//update position limits
	if(enableAntiCollision->value){
		if(axis1isAboveAxis2->value){
			//Axis 1 > Axis 2
			axis2Animatable->upperPositionLimit = axis1PositionToMachinePosition(axis1->getHighPositionLimit());
			axis2Animatable->lowerPositionLimit = axis2Animatable->getBrakingPosition() + std::abs(minimumDistanceBetweenAxes->value);
			axis2Animatable->upperPositionLimit = axis1Animatable->getBrakingPosition() - std::abs(minimumDistanceBetweenAxes->value);
			axis2Animatable->lowerPositionLimit = axis2PositionToMachinePosition(axis2->getLowPositionLimit());
		}else{
			//Axis 1 < Axis 2
			axis1Animatable->lowerPositionLimit = axis1PositionToMachinePosition(axis1->getLowPositionLimit());
			axis1Animatable->upperPositionLimit = axis2Animatable->getBrakingPosition() - std::abs(minimumDistanceBetweenAxes->value);
			axis2Animatable->lowerPositionLimit = axis1Animatable->getBrakingPosition() + std::abs(minimumDistanceBetweenAxes->value);
			axis2Animatable->upperPositionLimit = axis2PositionToMachinePosition(axis2->getHighPositionLimit());
		}
	}else{
		axis1Animatable->lowerPositionLimit = axis1PositionToMachinePosition(axis1->getLowPositionLimit());
		axis1Animatable->upperPositionLimit = axis1PositionToMachinePosition(axis1->getHighPositionLimit());
		axis2Animatable->lowerPositionLimit = axis2PositionToMachinePosition(axis2->getLowPositionLimit());
		axis2Animatable->upperPositionLimit = axis2PositionToMachinePosition(axis2->getHighPositionLimit());
	}
	
}

void SharedAxisMachine::outputProcess(){
	
	//handle dead mans switch
	if(!isMotionAllowed()){
		if(axis1Animatable->hasAnimation()) axis1Animatable->getAnimation()->pausePlayback();
		if(axis2Animatable->hasAnimation()) axis2Animatable->getAnimation()->pausePlayback();
		if(synchronizedAnimatable->hasAnimation()) synchronizedAnimatable->getAnimation()->pausePlayback();
		axis1Animatable->stopMovement();
		axis2Animatable->stopMovement();
		synchronizedAnimatable->stopMovement();
	}
	
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	if (!isEnabled() || isHoming()) {
		//if the machine is not enabled or is homing, the animatable doesn't do anything
		//we juste copy real values to the motion profile of the animatable
		axis1Animatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		axis2Animatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		synchronizedAnimatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
	}
	else{
		
		if(enableSynchronousControl->value){
						
			if(axis1Animatable->isControlledManuallyOrByAnimation() || axis2Animatable->isControlledManuallyOrByAnimation()){
				//individual animatables always have priority over the synchronized animatable
				//if any individual axis is controlled manually or by an animation, the synchronized animatable stops
				//any animation associated with the synchronized animatable also stops
				synchronizedAnimatable->stopMovement();
				synchronizedAnimatable->stopAnimation();
				synchronizedAnimatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
			}else{
				//in case the synchronized animatable is controlled
				//we force override the setpoint of the individual animatables
				synchronizedAnimatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				double masterTargetPosition = synchronizedAnimatable->getPositionSetpoint();
				double masterTargetVelocity = synchronizedAnimatable->getVelocitySetpoint();
				double masterTargetAcceleration = synchronizedAnimatable->getAccelerationSetpoint();
				if(axis1isMaster->value){
					double positionTargetDifference = masterTargetPosition - axis2Animatable->getPositionSetpoint();
					double slaveTargetPosition = masterTargetPosition - positionTargetDifference;
					axis1Animatable->forcePositionTarget(masterTargetPosition, masterTargetVelocity, masterTargetAcceleration);
					axis2Animatable->forcePositionTarget(slaveTargetPosition, masterTargetVelocity, masterTargetAcceleration);
				}else{
					double positionTargetDifference = masterTargetPosition - axis1Animatable->getPositionSetpoint();
					double slaveTargetPosition = masterTargetPosition - positionTargetDifference;
					axis2Animatable->forcePositionTarget(masterTargetPosition, masterTargetVelocity, masterTargetAcceleration);
					axis1Animatable->forcePositionTarget(slaveTargetPosition, masterTargetVelocity, masterTargetAcceleration);
				}
				
			}
	
		}
		
		axis1Animatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		axis2Animatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		
		auto axis1Target = axis1Animatable->getTargetValue()->toPosition();
		auto axis2Target = axis2Animatable->getTargetValue()->toPosition();
		getAxis1()->setMotionCommand(machinePositionToAxis1Position(axis1Target->position),
									 machineToAxis1Conversion(axis1Target->velocity),
									 machineToAxis1Conversion(axis1Target->acceleration));
		
		getAxis2()->setMotionCommand(machinePositionToAxis2Position(axis2Target->position),
									 machineToAxis2Conversion(axis2Target->velocity),
									 machineToAxis2Conversion(axis2Target->acceleration));
	}
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
	if(!areAxesConnected()) return false;
	else return getAxis1()->isHoming() || getAxis2()->isHoming();
}
void SharedAxisMachine::startHoming(){
	if(!areAxesConnected()) return false;
	if(!isEnabled()) return false;
	axis1Animatable->stopMovement();
	axis2Animatable->stopMovement();
	synchronizedAnimatable->stopMovement();
	getAxis1()->startHoming();
	getAxis2()->startHoming();
}
void SharedAxisMachine::stopHoming(){
	if(!areAxesConnected()) return false;
	getAxis1()->cancelHoming();
	getAxis2()->cancelHoming();
}
bool SharedAxisMachine::didHomingSucceed(){
	if(!areAxesConnected()) return false;
	return getAxis1()->didHomingSucceed() && getAxis2()->didHomingSucceed();
}
bool SharedAxisMachine::didHomingFail(){
	if(!areAxesConnected()) return false;
	return getAxis1()->didHomingFail() || getAxis2()->didHomingFail();
}
float SharedAxisMachine::getHomingProgress(){
	if(!areAxesConnected()) return false;
	return std::min(getAxis1()->getHomingProgress(), getAxis2()->getHomingProgress());
}
const char* SharedAxisMachine::getHomingStateString(){
	if(!areAxesConnected()) return "No Axes conneted...";
	if(isHoming()){
		if(getAxis1()->isHoming() && getAxis2()->isHoming()){
			return "Homing Both Axis...";
		}else if(getAxis1()->isHoming()){
			return Enumerator::getDisplayString(getAxis1()->getHomingStep());
		}else if(getAxis2()->isHoming()){
			return Enumerator::getDisplayString(getAxis2()->getHomingStep());
		}
	}else return "Not Homing";
}











//========= ANIMATABLE OWNER ==========

void SharedAxisMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	auto animatable = animation->getAnimatable();
	if(animatable != axis1Animatable || animatable != axis2Animatable || animatable != synchronizedAnimatable) return;
	
	switch(animation->getType()){
		case ManoeuvreType::KEY:
			animation->toKey()->captureTarget();
			break;
		case ManoeuvreType::TARGET:
			animation->toTarget()->captureTarget();
			animation->toTarget()->inAcceleration->overwrite(accelerationLimit->value);
			animation->toTarget()->outAcceleration->overwrite(accelerationLimit->value);
			animation->toTarget()->velocityConstraint->overwrite(velocityLimit->value);
			animation->toTarget()->timeConstraint->overwrite(0.0);
			animation->toTarget()->constraintType->overwrite(TargetAnimation::Constraint::TIME);
			break;
		case ManoeuvreType::SEQUENCE:
			animation->toSequence()->captureStart();
			animation->toSequence()->captureTarget();
			animation->toSequence()->duration->overwrite(0.0);
			animation->toSequence()->inAcceleration->overwrite(accelerationLimit->value);
			animation->toSequence()->outAcceleration->overwrite(accelerationLimit->value);
			break;
	}
}



void SharedAxisMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//if (isAxisConnected()) getAxis()->getDevices(output);
}



bool SharedAxisMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* generalXML = xml->InsertNewChildElement("GeneralSettings");
	enableAntiCollision->save(generalXML);
	minimumDistanceBetweenAxes->save(generalXML);
	enableSynchronousControl->save(generalXML);
	axis1isMaster->save(generalXML);
	
	XMLElement* axis1XML = xml->InsertNewChildElement("Axis1");
	invertAxis1->save(axis1XML);
	axis1Offset->save(axis1XML);
	
	XMLElement* axis2XML = xml->InsertNewChildElement("Axis2");
	invertAxis2->save(axis2XML);
	axis2Offset->save(axis2XML);
	
	XMLElement* motionLimitsXML = xml->InsertNewChildElement("MotionLimits");
	velocityLimit->save(motionLimitsXML);
	accelerationLimit->save(motionLimitsXML);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}


bool SharedAxisMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* generalXML;
	if(!loadXMLElement("GeneralSettings", xml, generalXML)) return false;
	if(!enableAntiCollision->load(generalXML)) return false;
	if(!minimumDistanceBetweenAxes->load(generalXML)) return false;
	if(!enableSynchronousControl->load(generalXML)) return false;
	enableSynchronousControl->onEdit();
	if(!axis1isMaster->load(generalXML)) return false;
	
	XMLElement* axis1XML;
	if(!loadXMLElement("Axis1", xml, axis1XML)) return false;
	if(!invertAxis1->load(axis1XML)) return false;
	if(!axis1Offset->load(axis1XML)) return false;
	
	XMLElement* axis2XML;
	if(!loadXMLElement("Axis2", xml, axis2XML)) return false;
	if(!invertAxis2->load(axis2XML)) return false;
	if(!axis2Offset->load(axis2XML)) return false;
	
	XMLElement* motionLimitsXML;
	if(!loadXMLElement("MotionLimits", xml, motionLimitsXML)) return false;
	if(!velocityLimit->load(motionLimitsXML)) return false;
	if(!accelerationLimit->load(motionLimitsXML)) return false;
	
	XMLElement* widgetXML = xml->FirstChildElement("ControWidget");
	if(widgetXML == nullptr) return Logger::warn("Could not find Control Widget Attribute");
	if(widgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS) return Logger::warn("Could not find machine control widget uid attribute");
	 
	return true;
}






void SharedAxisMachine::captureAxis1PositionToOffset(double machinePosition){
	//TODO: test
	axis1Offset->overwriteWithHistory(axis1Animatable->getPositionSetpoint() - machinePosition);
}

void SharedAxisMachine::captureAxis2PositionToOffset(double machinePosition){
	//TODO: test
	axis2Offset->overwriteWithHistory(axis2Animatable->getPositionSetpoint() - machinePosition);
}

double SharedAxisMachine::axis1PositionToMachinePosition(double axis1Position){
	//TODO: test
	if(invertAxis1->value) return -1.f * (axis1Position - axis1Offset->value);
	return axis1Position - axis1Offset->value;
}

double SharedAxisMachine::axis2PositionToMachinePosition(double axis2Position){
	//TODO: test
	if(invertAxis2->value) return -1.f * (axis2Position - axis2Offset->value);
	return axis2Position - axis2Offset->value;
}

double SharedAxisMachine::machinePositionToAxis1Position(double machineAxis1Position){
	//TODO: test
	if(invertAxis1->value) return (-1.f * machineAxis1Position) + axis1Offset->value;
	return machineAxis1Position + axis1Offset->value;
}

double SharedAxisMachine::machinePositionToAxis2Position(double machineAxis2Position){
	//TODO: test
	if(invertAxis2->value) return (-1.f * machineAxis2Position) + axis2Offset->value;
	return machineAxis2Position + axis2Offset->value;
}

double SharedAxisMachine::axis1ToMachineConversion(double axis1Value){
	if(invertAxis1->value) return axis1Value * -1.0;
	return axis1Value;
}

double SharedAxisMachine::axis2ToMachineConversion(double axis2Value){
	if(invertAxis2->value) return axis2Value * -1.0;
	return axis2Value;
}

double SharedAxisMachine::machineToAxis1Conversion(double machineAxis1Value){
	if(invertAxis1->value) return machineAxis1Value * -1.0;
	return machineAxis1Value;
}

double SharedAxisMachine::machineToAxis2Conversion(double machineAxis2Value){
	if(invertAxis2->value) return machineAxis2Value * -1.0;
	return machineAxis2Value;
}
