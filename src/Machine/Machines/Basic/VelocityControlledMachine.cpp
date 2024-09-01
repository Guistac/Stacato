#include <pch.h>

#include "VelocityControlledMachine.h"

#include "Motion/Axis/AxisNode.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Animation/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Motion/Safety/DeadMansSwitch.h"

void VelocityControlledMachine::initialize() {
	//inputs
	addNodePin(axisPin);
		
	//outputs
	velocityPin->assignData(velocityPinValue);
	addNodePin(velocityPin);

	//machine parameters
	addAnimatable(animatableVelocity);
	
	auto thisMachine = std::static_pointer_cast<VelocityControlledMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine);
	
	invertAxis->setEditCallback([this](std::shared_ptr<Parameter>){ updateAnimatableParameters(); });
	
}

bool VelocityControlledMachine::isAxisConnected() {
	return axisPin->isConnected();
}

std::shared_ptr<AxisInterface> VelocityControlledMachine::getAxisInterface() {
	if(!axisPin->isConnected()) return nullptr;
	return axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
}

void VelocityControlledMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == axisPin) updateAnimatableParameters();
}

void VelocityControlledMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == axisPin) updateAnimatableParameters();
}

void VelocityControlledMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == axisPin) updateAnimatableParameters();
}

bool VelocityControlledMachine::isHardwareReady() {
	if (!isAxisConnected()) return false;
	auto axis = getAxisInterface();
	if(!axis->isReady()) return false;
	return true;
}

bool VelocityControlledMachine::isMoving() {
	if (isAxisConnected()) {
		auto axis = getAxisInterface();
		return axis->getState() == DeviceState::ENABLED && axis->getVelocityActual() != 0.0;
	}
	return false;
}

void VelocityControlledMachine::enableHardware() {
	if (isReady()) b_enableRequest = true;
}

void VelocityControlledMachine::disableHardware() {
	animatableVelocity->stopMovement();
	animatableVelocity->stopAnimation();
	b_disableRequest = true;
}

void VelocityControlledMachine::onEnableHardware() {}
void VelocityControlledMachine::onDisableHardware() {}

bool VelocityControlledMachine::isSimulationReady(){ return isAxisConnected();}
void VelocityControlledMachine::onEnableSimulation(){}
void VelocityControlledMachine::onDisableSimulation(){}

std::string VelocityControlledMachine::getStatusString(){
	std::string status;
	switch(state){
		case DeviceState::OFFLINE:
			status = "Machine is Offline : ";
			if(!isAxisConnected()) status += "No Axis is Connected";
			else status += "\n" + getAxisInterface()->getStatusString();
			return status;
		case DeviceState::NOT_READY:
			status = "Machine is not ready : " + getAxisInterface()->getStatusString();
			return status;
		case DeviceState::READY:
			status = "Machine is ready to enable.";
			return status;
		case DeviceState::ENABLED:
			status = "Machine is enabled.";
			if(b_halted){
				if(!isSimulating()){
					if(!isMotionAllowed()){
						for(auto connectedDeadMansSwitchPin : deadMansSwitchPin->getConnectedPins()){
							auto deadMansSwitch = connectedDeadMansSwitchPin->getSharedPointer<DeadMansSwitch>();
							status += "\nMovement is prohibited by Dead Mans Switch \"" + std::string(deadMansSwitch->getName()) + "\"";
						}
					}
				}
			}
			return status;
		default:
			return "";
	}
}

void VelocityControlledMachine::inputProcess() {
	if (!isAxisConnected()) {
		state = DeviceState::OFFLINE;
		return;
	}
	auto axis = getAxisInterface();
	state = axis->getState();
	
	//update machine state
	if (isEnabled() && !axis->isEnabled()) b_disableRequest = true;
	else state = axis->getState();
	
	//update estop state
	b_emergencyStopActive = axis->isEmergencyStopActive() && axis->getState() != DeviceState::OFFLINE;
	
	//update halt state
	b_halted = isEnabled() && !isMotionAllowed();
	
	//update animatable state
	if(state == DeviceState::OFFLINE) animatableVelocity->state = Animatable::State::OFFLINE;
	else if(state == DeviceState::ENABLED){
		if(b_halted) animatableVelocity->state = Animatable::State::HALTED;
		else animatableVelocity->state = Animatable::State::READY;
	}
	else animatableVelocity->state = Animatable::State::NOT_READY;
	
	auto actualVelocity = AnimationValue::makeVelocity();
	actualVelocity->velocity = axisVelocityToMachineVelocity(axis->getVelocityActual());
	actualVelocity->acceleration = axisAccelerationToMachineAcceleration(0.0);
	animatableVelocity->updateActualValue(actualVelocity);
	
	//Get Realtime values from axis (for position and velocity pins only)
	*velocityPinValue = actualVelocity->velocity;
}

void VelocityControlledMachine::outputProcess(){
	
	if(b_enableRequest){
		b_enableRequest = false;
		if(isAxisConnected()) getAxisInterface()->enable();
	}
	
	if(b_disableRequest){
		b_disableRequest = false;
		Logger::info("machine disable request process");
		if(isAxisConnected()) getAxisInterface()->disable();
	}
	
	if(b_emergencyStopActive){
		animatableVelocity->stopMovement();
		animatableVelocity->stopAnimation();
	}
	
	if(!isMotionAllowed()){
		if(animatableVelocity->hasAnimation()){
			animatableVelocity->getAnimation()->pausePlayback();
		}
		animatableVelocity->stopMovement();
	}
	
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	
	if (!isEnabled()) {
		//if the axis is not enabled or is homing, the animatable doesn't do anything
		animatableVelocity->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
	}
	else{
		//else we update the animatable, get its target and send it to the axis
		animatableVelocity->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		auto target = animatableVelocity->getTargetValue()->toVelocity();
		
		double axisVelocityTarget = machineVelocityToAxisVelocity(target->velocity);
		double axisAccelerationTarget = machineAccelerationToAxisAcceleration(target->acceleration);
		
		getAxisInterface()->setVelocityTarget(axisVelocityTarget, axisAccelerationTarget);
	}
	
}

void VelocityControlledMachine::simulateInputProcess(){}
void VelocityControlledMachine::simulateOutputProcess(){}






void VelocityControlledMachine::updateAnimatableParameters(){
	if(!isAxisConnected()) {
		animatableVelocity->setUnit(Units::None::None);
		animatableVelocity->velocityLimit = 0.0;
		animatableVelocity->accelerationLimit = 0.0;
		return;
	}
	auto axis = getAxisInterface();
	
	animatableVelocity->setUnit(axis->getPositionUnit());
	animatableVelocity->velocityLimit = std::abs(axis->getVelocityLimit());
	animatableVelocity->accelerationLimit = std::abs(axis->getAccelerationLimit());
}








//========= ANIMATABLE OWNER ==========

void VelocityControlledMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	if(animation->getAnimatable() != animatableVelocity) return;
	switch(animation->getType()){
		case ManoeuvreType::KEY:
			animation->toKey()->captureTarget();
			break;
		case ManoeuvreType::TARGET:
			animation->toTarget()->captureTarget();
			animation->toTarget()->inAcceleration->overwrite(animatableVelocity->accelerationLimit);
			animation->toTarget()->outAcceleration->overwrite(animatableVelocity->accelerationLimit);
			animation->toTarget()->velocityConstraint->overwrite(0.0);
			animation->toTarget()->timeConstraint->overwrite(0.0);
			animation->toTarget()->constraintType->overwrite(TargetAnimation::Constraint::TIME);
			break;
		case ManoeuvreType::SEQUENCE:
			animation->toSequence()->captureStart();
			animation->toSequence()->captureTarget();
			animation->toSequence()->duration->overwrite(0.0);
			animation->toSequence()->inAcceleration->overwrite(animatableVelocity->accelerationLimit);
			animation->toSequence()->outAcceleration->overwrite(animatableVelocity->accelerationLimit);
			break;
	}
	
}



void VelocityControlledMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//if (isAxisConnected()) getAxisInterface()->getDevices(output);
}



bool VelocityControlledMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	 
	XMLElement* limitsXML = xml->InsertNewChildElement("Limits");
	invertAxis->save(limitsXML);
	
	XMLElement* userSetupXML = xml->InsertNewChildElement("UserSetup");
	invertControlGui->save(userSetupXML);
	
	XMLElement* animatableXML = xml->InsertNewChildElement("Animatable");
	animatableVelocity->save(animatableXML);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}

bool VelocityControlledMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	 
	XMLElement* limitsXML;
	if(!loadXMLElement("Limits", xml, limitsXML)) return false;
	if(!invertAxis->load(limitsXML)) return false;
	
	XMLElement* userSetupXML;
	if(!loadXMLElement("UserSetup", xml, userSetupXML)) return false;
	if(!invertControlGui->load(userSetupXML)) return false;
	
	if(XMLElement* animatableXML = xml->FirstChildElement("Animatable")){
		animatableVelocity->load(animatableXML);
	}
	 
	XMLElement* widgetXML = xml->FirstChildElement("ControWidget");
	if(widgetXML == nullptr) return Logger::warn("Could not find Control Widget Attribute");
	if(widgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS) return Logger::warn("Could not find machine control widget uid attribute");
	
	return true;
}



double VelocityControlledMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(invertAxis->value) return axisVelocity * -1.0;
	return axisVelocity;
}

double VelocityControlledMachine::axisAccelerationToMachineAcceleration(double axisAcceleration){
	if(invertAxis->value) return axisAcceleration * -1.0;
	return axisAcceleration;
}

double VelocityControlledMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(invertAxis->value) return machineVelocity * -1.0;
	return machineVelocity;
}

double VelocityControlledMachine::machineAccelerationToAxisAcceleration(double machineAcceleration){
	if(invertAxis->value) return machineAcceleration * -1.0;
	return machineAcceleration;
}
