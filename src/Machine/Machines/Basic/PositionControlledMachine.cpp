#include <pch.h>

#include "PositionControlledMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Animation/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

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
	
	auto thisMachine = std::dynamic_pointer_cast<PositionControlledMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
}

void PositionControlledMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) animatablePosition->setUnit(getAxis()->getPositionUnit());
}

void PositionControlledMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) animatablePosition->setUnit(getAxis()->getPositionUnit());
}

void PositionControlledMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) animatablePosition->setUnit(Units::None::None);
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
			while (duration(system_clock::now() - enableRequestTime) < milliseconds(100)) {
				std::this_thread::sleep_for(milliseconds(10));
				if (axis->isEnabled()) {
					b_enabled = true;
					onEnableHardware();
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void PositionControlledMachine::disableHardware() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
	onDisableHardware();
}

void PositionControlledMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
}

void PositionControlledMachine::onDisableHardware() {
	//setVelocityTarget(0.0);
	animatablePosition->stop();
	Logger::info("Disabled Machine {}", getName());
}

bool PositionControlledMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledMachine::onEnableSimulation() {
	/*
	motionProfile.resetInterpolation();
	motionProfile.setVelocity(0.0);
	motionProfile.setAcceleration(0.0);
	setVelocityTarget(0.0);
	 */
}

void PositionControlledMachine::onDisableSimulation() {
	animatablePosition->stop();
	/*
	setVelocityTarget(0.0);
	motionProfile.setVelocity(0.0);
	motionProfile.setAcceleration(0.0);
	 */
}

void PositionControlledMachine::inputProcess() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//Get Realtime values from axis (for position and velocity pins only)
	*positionPinValue = axisPositionToMachinePosition(axis->getActualPosition());
	*velocityPinValue = axisVelocityToMachineVelocity(axis->getActualVelocity());

	//Handle Axis state changes
	if (isEnabled() && !axis->isEnabled()) disable();
}


void PositionControlledMachine::outputProcess(){
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	
	//if the axis is not enabled or is homing, the machine has no control over it, we just update the motion profile
	if (!isEnabled() || isHoming()) {
		/*
		motionProfile.setPosition(axis->getActualPosition());
		motionProfile.setVelocity(axis->getActualVelocity());
		motionProfile.setAcceleration(0.0);
		 */
		return; //don't send motion commands to the axis
	}
	
	//profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	//profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//Update Motion Profile
	/*
	switch(controlMode){
		case ControlMode::PARAMETER_TRACK:{
			auto value = animatablePosition->getAnimationValue()->toPosition();
			
			motionProfile.matchPositionAndRespectPositionLimits(profileDeltaTime_seconds,
																value->position,
																value->velocity,
																value->acceleration,
																rapidAcceleration_machineUnitsPerSecond,
																rapidVelocity_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit());
			}break;
			
		case ControlMode::VELOCITY_TARGET:{
			motionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																manualVelocityTarget_machineUnitsPerSecond,
																rapidAcceleration_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit(),
																axis->getAccelerationLimit());
			}break;
			
		case ControlMode::POSITION_TARGET:{
			motionProfile.updateInterpolation(profileTime_seconds);
			if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
		}break;
	}
	 */
	
	//Send motion values to axis profile
	/*
	double axisPosition = machinePositionToAxisPosition(motionProfile.getPosition());
	double axisVelocity = machineVelocityToAxisVelocity(motionProfile.getVelocity());
	double axisAcceleration = machineAccelerationToAxisAcceleration(motionProfile.getAcceleration());
	axis->setMotionCommand(axisPosition, axisVelocity, axisAcceleration);
	 */
}

void PositionControlledMachine::simulateInputProcess() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//update position and velocity pins
	/*
	*positionPinValue = motionProfile.getPosition();
	*velocityPinValue = motionProfile.getVelocity();
	 */
	 
	//Update Time
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();

	//Update Motion Profile
	/*
	switch(controlMode){
			
		case ControlMode::PARAMETER_TRACK:{
			auto value = animatablePosition->getAnimationValue()->toPosition();
			motionProfile.matchPositionAndRespectPositionLimits(profileDeltaTime_seconds,
																value->position,
																value->velocity,
																value->acceleration,
																rapidAcceleration_machineUnitsPerSecond,
																rapidVelocity_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit());
			}break;
		case ControlMode::VELOCITY_TARGET:{
			double lowLimit_machineUnits = getLowPositionLimit();
			double highLimit_machineUnits = getHighPositionLimit();
			double accelerationLimit = axis->getAccelerationLimit();
			motionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																  manualVelocityTarget_machineUnitsPerSecond,
																  rapidAcceleration_machineUnitsPerSecond,
																  lowLimit_machineUnits,
																  highLimit_machineUnits,
																  accelerationLimit);
		}break;
			
		case ControlMode::POSITION_TARGET:{
			motionProfile.updateInterpolation(profileTime_seconds);
			if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
		}break;
			
	}
	 */
	
}

void PositionControlledMachine::simulateOutputProcess(){}

bool PositionControlledMachine::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

bool PositionControlledMachine::isHoming(){
	return isAxisConnected() && getAxis()->isHoming();
}
void PositionControlledMachine::startHoming(){
	animatablePosition->stop();
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


bool PositionControlledMachine::isAxisConnected() {
	return positionControlledAxisPin->isConnected();
}

std::shared_ptr<PositionControlledAxis> PositionControlledMachine::getAxis() {
	return positionControlledAxisPin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}

















//========= ANIMATABLE OWNER ==========

void PositionControlledMachine::onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable){}

void PositionControlledMachine::onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable){}

void PositionControlledMachine::onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable){}

void PositionControlledMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
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
}



void PositionControlledMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}



bool PositionControlledMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", rapidVelocity_machineUnitsPerSecond);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", rapidAcceleration_machineUnitsPerSecond);
	
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
	if (rapidsXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &rapidVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid velocity attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &rapidAcceleration_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid acceleration attribute");
	
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
