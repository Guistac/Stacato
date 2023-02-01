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
	controlWidget = std::make_shared<ControlWidget>(thisMachine);
	
	
	lowerPositionLimit->setEditCallback([this](std::shared_ptr<Parameter>){
		if(lowerPositionLimit->value < getMinPosition()) lowerPositionLimit->overwrite(getMinPosition());
		else if(lowerPositionLimit->value > 0.0) lowerPositionLimit->overwrite(0.0);
		updateAnimatableParameters();
	});
	
	upperPositionLimit->setEditCallback([this](std::shared_ptr<Parameter>){
		if(upperPositionLimit->value > getMaxPosition()) upperPositionLimit->overwrite(getMaxPosition());
		else if(upperPositionLimit->value < 0.0) upperPositionLimit->overwrite(0.0);
		updateAnimatableParameters();
	});
	
	invertAxis->setEditCallback([this](std::shared_ptr<Parameter>){
		upperPositionLimit->onEdit();
		lowerPositionLimit->onEdit();
		updateAnimatableParameters();
	});
	
	axisOffset->setEditCallback([this](std::shared_ptr<Parameter>){
		auto axis = getAxis();
		if(axisOffset->value < axis->getLowPositionLimit()) axisOffset->overwrite(axis->getLowPositionLimit());
		if(axisOffset->value > axis->getHighPositionLimit()) axisOffset->overwrite(axis->getHighPositionLimit());
		upperPositionLimit->onEdit();
		lowerPositionLimit->onEdit();
		updateAnimatableParameters();
	});
	
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
	if(axis->getState() != DeviceState::READY) return false;
	return true;
}

bool PositionControlledMachine::isMoving() {
    if (isAxisConnected()) {
        auto axis = getAxis();
        return axis->isMoving() && axis->getState() == DeviceState::ENABLED;
    }
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
				if (axis->getState() == DeviceState::ENABLED) {
					state = DeviceState::ENABLED;
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
	state = DeviceState::READY;
	if (isAxisConnected()) getAxis()->disable();
	onDisableHardware();
}

void PositionControlledMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
	animatablePosition->stopMovement();
	animatablePosition->stopAnimation();
}

void PositionControlledMachine::onDisableHardware() {
	Logger::info("Disabled Machine {}", getName());
	animatablePosition->stopMovement();
	animatablePosition->stopAnimation();
}

bool PositionControlledMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledMachine::onEnableSimulation() {
}

void PositionControlledMachine::onDisableSimulation() {
}

std::string PositionControlledMachine::getStatusString(){
	std::string status;
	switch(state){
		case DeviceState::OFFLINE:
			status = "Machine is Offline : ";
			if(!isAxisConnected()) status += "No Axis is Connected";
			else status += "\n" + getAxis()->getStatusString();
			return status;
		case DeviceState::NOT_READY:
			status = "Machine is not ready : " + getAxis()->getStatusString();
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
				for(auto constraint : animatablePosition->getConstraints()){
					if(constraint->getType() == AnimationConstraint::Type::HALT && constraint->isEnabled()){
						status += "\nMovement is halted by constraint \"" + constraint->getName() + "\"";
					}
				}
			}
			return status;
	}
}

void PositionControlledMachine::inputProcess() {
	if (!isAxisConnected()) {
		state = DeviceState::OFFLINE;
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	
	//update machine state
	if (isEnabled() && axis->getState() != DeviceState::ENABLED) disable();
	else state = axis->getState();
	
	//update estop state
	b_emergencyStopActive = axis->isEmergencyStopActive() && axis->getState() != DeviceState::OFFLINE;
	
	//update halt state
	b_halted = isEnabled() && !isMotionAllowed();
	
	//update animatable state
	if(state == DeviceState::OFFLINE) animatablePosition->state = Animatable::State::OFFLINE;
    else if(state == DeviceState::ENABLED){
        if(b_halted) animatablePosition->state = Animatable::State::HALTED;
        else animatablePosition->state = Animatable::State::READY;
    }
	else animatablePosition->state = Animatable::State::NOT_READY;
	
	auto actualPosition = AnimationValue::makePosition();
	actualPosition->position = axisPositionToMachinePosition(axis->getActualPosition());
	actualPosition->velocity = axisVelocityToMachineVelocity(axis->getActualVelocity());
	actualPosition->acceleration = axisAccelerationToMachineAcceleration(0.0);
	animatablePosition->updateActualValue(actualPosition);
	
	//animatablePosition->upperPositionLimit = axisPositionToMachinePosition(axis->getHighPositionLimit());
	//animatablePosition->lowerPositionLimit = axisPositionToMachinePosition(axis->getLowPositionLimit());
	//if(invertAxis->value) std::swap(animatablePosition->upperPositionLimit, animatablePosition->lowerPositionLimit);
	
	//Get Realtime values from axis (for position and velocity pins only)
	*positionPinValue = actualPosition->position;
	*velocityPinValue = actualPosition->velocity;
}

void PositionControlledMachine::outputProcess(){
	
	if(b_emergencyStopActive){
		animatablePosition->stopMovement();
		animatablePosition->stopAnimation();
	}
	
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
	if(state == DeviceState::ENABLED) { /* do nothing*/ }
	else if(isAxisConnected()) state = DeviceState::READY;
	else state = DeviceState::OFFLINE;
	
	//update halt and estop state
	b_emergencyStopActive = false;
	b_halted = false;
		
	//update animatable state
	if(state == DeviceState::OFFLINE) animatablePosition->state = Animatable::State::OFFLINE;
	else if(state == DeviceState::ENABLED && !b_halted) animatablePosition->state = Animatable::State::READY;
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



double PositionControlledMachine::getMinPosition(){
	if(invertAxis->value) return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
}

double PositionControlledMachine::getMaxPosition(){
	if(invertAxis->value) return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
}

double PositionControlledMachine::getLowerPositionLimit(){
	return lowerPositionLimit->value;
}

double PositionControlledMachine::getUpperPositionLimit(){
	return upperPositionLimit->value;
}


void PositionControlledMachine::captureZero(){
	if(invertAxis->value) axisOffset->overwriteWithHistory(getAxis()->getHighPositionLimit() - animatablePosition->motionProfile.getPosition());
	else axisOffset->overwriteWithHistory(getAxis()->getLowPositionLimit() + animatablePosition->motionProfile.getPosition());
	animatablePosition->motionProfile.setPosition(0.0);
	animatablePosition->setManualVelocityTarget(0.0);
}

void PositionControlledMachine::resetZero(){
	if(invertAxis->value) axisOffset->overwriteWithHistory(getAxis()->getHighPositionLimit());
	else axisOffset->overwriteWithHistory(getAxis()->getLowPositionLimit());
	animatablePosition->setManualVelocityTarget(0.0);
}

void PositionControlledMachine::captureLowerLimit(){
	lowerPositionLimit->overwriteWithHistory(animatablePosition->motionProfile.getPosition());
}

void PositionControlledMachine::resetLowerLimit(){
	lowerPositionLimit->overwriteWithHistory(getMinPosition());
}

void PositionControlledMachine::captureUpperLimit(){
	upperPositionLimit->overwriteWithHistory(animatablePosition->motionProfile.getPosition());
}

void PositionControlledMachine::resetUpperLimit(){
	upperPositionLimit->overwriteWithHistory(getMaxPosition());
}






bool PositionControlledMachine::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

bool PositionControlledMachine::isHoming(){
	return getAxis()->isHoming();
}
void PositionControlledMachine::startHoming(){
	animatablePosition->stopAnimation();
	getAxis()->startHoming();
}
void PositionControlledMachine::stopHoming(){
	getAxis()->cancelHoming();
}
bool PositionControlledMachine::didHomingSucceed(){
	return getAxis()->didHomingSucceed();
}
bool PositionControlledMachine::didHomingFail(){
	return getAxis()->didHomingFail();
}
const char* PositionControlledMachine::getHomingString(){
	return Enumerator::getDisplayString(getAxis()->getHomingStep());
}







void PositionControlledMachine::updateAnimatableParameters(){
	if(!isAxisConnected()) {
		animatablePosition->setUnit(Units::None::None);
		animatablePosition->lowerPositionLimit = 0.0;
		animatablePosition->upperPositionLimit = 0.0;
		animatablePosition->velocityLimit = 0.0;
		animatablePosition->accelerationLimit = 0.0;
		return;
	}
	auto axis = getAxis();
	positionUnit = axis->getPositionUnit();
	axisOffset->setUnit(positionUnit);
	lowerPositionLimit->setUnit(positionUnit);
	upperPositionLimit->setUnit(positionUnit);
	animatablePosition->setUnit(axis->getPositionUnit());
	animatablePosition->lowerPositionLimit = getLowerPositionLimit();
	animatablePosition->upperPositionLimit = getUpperPositionLimit();
	animatablePosition->velocityLimit = std::abs(axis->getVelocityLimit());
	animatablePosition->accelerationLimit = std::abs(axis->getAccelerationLimit());
}








//========= ANIMATABLE OWNER ==========

void PositionControlledMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	
	if(animation->getAnimatable() != animatablePosition) return;
	switch(animation->getType()){
		case ManoeuvreType::KEY:
			animation->toKey()->captureTarget();
			break;
		case ManoeuvreType::TARGET:
			animation->toTarget()->captureTarget();
			animation->toTarget()->inAcceleration->overwrite(animatablePosition->accelerationLimit);
			animation->toTarget()->outAcceleration->overwrite(animatablePosition->accelerationLimit);
			animation->toTarget()->velocityConstraint->overwrite(0.0);
			animation->toTarget()->timeConstraint->overwrite(0.0);
			animation->toTarget()->constraintType->overwrite(TargetAnimation::Constraint::TIME);
			break;
		case ManoeuvreType::SEQUENCE:
			animation->toSequence()->captureStart();
			animation->toSequence()->captureTarget();
			animation->toSequence()->duration->overwrite(0.0);
			animation->toSequence()->inAcceleration->overwrite(animatablePosition->accelerationLimit);
			animation->toSequence()->outAcceleration->overwrite(animatablePosition->accelerationLimit);
			break;
	}
	
}



void PositionControlledMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}



bool PositionControlledMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	 
	XMLElement* limitsXML = xml->InsertNewChildElement("Limits");
	invertAxis->save(limitsXML);
	axisOffset->save(limitsXML);
	lowerPositionLimit->save(limitsXML);
	upperPositionLimit->save(limitsXML);
	
	XMLElement* userSetupXML = xml->InsertNewChildElement("UserSetup");
	allowUserZeroEdit->save(userSetupXML);
	allowUserLowerLimitEdit->save(userSetupXML);
	allowUserUpperLimitEdit->save(userSetupXML);
	allowUserHoming->save(userSetupXML);
	allowUserEncoderRangeReset->save(userSetupXML);
	allowUserEncoderValueOverride->save(userSetupXML);
	invertControlGui->save(userSetupXML);
	
	XMLElement* widgetXML = xml->InsertNewChildElement("ControWidget");
	widgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	
	return true;
}

bool PositionControlledMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	 
	XMLElement* limitsXML;
	if(!loadXMLElement("Limits", xml, limitsXML)) return false;
	if(!invertAxis->load(limitsXML)) return false;
	if(!axisOffset->load(limitsXML)) return false;
	if(!lowerPositionLimit->load(limitsXML)) return false;
	if(!upperPositionLimit->load(limitsXML)) return false;
	
	XMLElement* userSetupXML;
	if(!loadXMLElement("UserSetup", xml, userSetupXML)) return false;
	if(!allowUserZeroEdit->load(userSetupXML)) return false;
	if(!allowUserLowerLimitEdit->load(userSetupXML)) return false;
	if(!allowUserUpperLimitEdit->load(userSetupXML)) return false;
	if(!allowUserHoming->load(userSetupXML)) return false;
	if(!allowUserEncoderRangeReset->load(userSetupXML)) return false;
	if(!allowUserEncoderValueOverride->load(userSetupXML)) return false;
	if(!invertControlGui->load(userSetupXML)) return false;
	 
	XMLElement* widgetXML = xml->FirstChildElement("ControWidget");
	if(widgetXML == nullptr) return Logger::warn("Could not find Control Widget Attribute");
	if(widgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS) return Logger::warn("Could not find machine control widget uid attribute");
	 
	return true;
}



double PositionControlledMachine::axisPositionToMachinePosition(double axisPosition){
	if(invertAxis->value) return -1.0f * (axisPosition - axisOffset->value);
	return axisPosition - axisOffset->value;
}

double PositionControlledMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(invertAxis->value) return axisVelocity * -1.0;
	return axisVelocity;
}

double PositionControlledMachine::axisAccelerationToMachineAcceleration(double axisAcceleration){
	if(invertAxis->value) return axisAcceleration * -1.0;
	return axisAcceleration;
}

double PositionControlledMachine::machinePositionToAxisPosition(double machinePosition){
	if(invertAxis->value) return (-1.0f * machinePosition) + axisOffset->value;
	return machinePosition + axisOffset->value;
}

double PositionControlledMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(invertAxis->value) return machineVelocity * -1.0;
	return machineVelocity;
}

double PositionControlledMachine::machineAccelerationToAxisAcceleration(double machineAcceleration){
	if(invertAxis->value) return machineAcceleration * -1.0;
	return machineAcceleration;
}
