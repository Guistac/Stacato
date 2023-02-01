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

#include "Motion/Curve/Profile.h"

/*

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
	controlWidget = std::make_shared<ControlWidget>(thisMachine);
	
	
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
	synchronizedAnimatable->lowerPositionLimit = std::min(axis1Animatable->lowerPositionLimit, axis2Animatable->lowerPositionLimit);
	
	axis1Animatable->upperPositionLimit = axis1PositionToMachinePosition(axis1->getHighPositionLimit());
	axis2Animatable->upperPositionLimit = axis2PositionToMachinePosition(axis2->getHighPositionLimit());
	synchronizedAnimatable->upperPositionLimit = std::max(axis1Animatable->upperPositionLimit, axis2Animatable->upperPositionLimit);
	
    if(invertAxis1->value) std::swap(axis1Animatable->lowerPositionLimit, axis1Animatable->upperPositionLimit);
    if(invertAxis2->value) std::swap(axis2Animatable->lowerPositionLimit, axis2Animatable->upperPositionLimit);
    
	axis1Animatable->velocityLimit = velocityLimit->value;
	axis2Animatable->velocityLimit = velocityLimit->value;
	synchronizedAnimatable->velocityLimit = velocityLimit->value;
	
	axis1Animatable->accelerationLimit = accelerationLimit->value;
	axis2Animatable->accelerationLimit = accelerationLimit->value;
	synchronizedAnimatable->accelerationLimit = accelerationLimit->value;
}

void SharedAxisMachine::addConstraints(){
	axis1AnticollisionConstraint = std::make_shared<AnimatablePosition_KeepoutConstraint>("anti collision", 0.0, 0.0);
	axis2AnticollisionConstraint = std::make_shared<AnimatablePosition_KeepoutConstraint>("anti collision", 0.0, 0.0);
	axis1Animatable->addConstraint(axis1AnticollisionConstraint);
	axis2Animatable->addConstraint(axis2AnticollisionConstraint);
	synchronousLowerKeepoutConstraint = std::make_shared<AnimatablePosition_KeepoutConstraint>("Lower Synchronized Limit", 0.0, 0.0);
	synchronousUpperKeepoutConstraint = std::make_shared<AnimatablePosition_KeepoutConstraint>("Upper Synchronized Limit", 0.0, 0.0);
	synchronizedAnimatable->addConstraint(synchronousLowerKeepoutConstraint);
	synchronizedAnimatable->addConstraint(synchronousUpperKeepoutConstraint);
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
	if(axis1->getState() != DeviceState::READY) return false;
	if(axis2->getState() != DeviceState::READY) return false;
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
				if (axis1->getState() == DeviceState::ENABLED && axis2->getState() == DeviceState::ENABLED) {
					state = DeviceState::ENABLED;
					onEnableHardware();
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void SharedAxisMachine::disableHardware() {
	state = DeviceState::READY;
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
	axis1Animatable->stopMovement();
	axis1Animatable->stopAnimation();
	axis2Animatable->stopMovement();
	axis2Animatable->stopAnimation();
	synchronizedAnimatable->stopMovement();
	synchronizedAnimatable->stopAnimation();
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
	switch(state){
		case DeviceState::OFFLINE:
			output = "Machine is Offline:\n";
			if(!areAxesConnected()) {
				output += "Axes are not connected.\n";
				return output;
			}
			if(!axesHaveSamePositionUnit()) {
				output += "Axes don't have the same position unit.\n";
				return output;
			}
			if(getAxis1()->getState() == DeviceState::OFFLINE)
				output += "Axis 1 \"" + std::string(getAxis1()->getName()) + "\" is Offline :\n" + getAxis1()->getStatusString() + "\n";
			if(getAxis2()->getState() == DeviceState::OFFLINE)
				output += "Axis 2 \"" + std::string(getAxis2()->getName()) + "\" is Offline :\n" + getAxis2()->getStatusString() + "\n";
			return output;
		case DeviceState::NOT_READY:
			output = "Machine is Not Ready :";
			if(getAxis1()->getState() == DeviceState::NOT_READY)
				output += "Axis 1 \"" + std::string(getAxis1()->getName()) + "\" is Not Ready :\n" + getAxis1()->getStatusString() + "\n";
			if(getAxis2()->getState() == DeviceState::NOT_READY)
				output += "Axis 2 \"" + std::string(getAxis2()->getName()) + "\" is Not Ready :\n" + getAxis2()->getStatusString() + "\n";
			return output;
		case DeviceState::READY:
			output = "Machine is Ready.\n";
			return output;
		case DeviceState::ENABLED:
			output = "Machine is Enabled.\n";
			if(b_halted){
				if(!isSimulating()){
					if(!isMotionAllowed()){
						for(auto connectedDeadMansSwitchPin : deadMansSwitchPin->getConnectedPins()){
							auto deadMansSwitch = connectedDeadMansSwitchPin->getSharedPointer<DeadMansSwitch>();
							output += "\nMovement is prohibited by Dead Mans Switch \"" + std::string(deadMansSwitch->getName()) + "\"\n";
						}
					}
				}
			}
			return output;
	}
}


void SharedAxisMachine::inputProcess() {
	if (!areAxesConnected() || !axesHaveSamePositionUnit()) {
		state = DeviceState::OFFLINE;
		b_halted = false;
		b_emergencyStopActive = false;
		return;
	}
	
	//update machine state
	DeviceState newState = DeviceState::ENABLED;
	auto checkState = [&](DeviceState checkedState){ if(int(checkedState) < int(newState)) newState = checkedState; };
	
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	checkState(axis1->getState());
	checkState(axis2->getState());
	
	//handle transition from enabled state
	if(state == DeviceState::ENABLED && newState != DeviceState::ENABLED) disable();
	state = newState;
	
	if(state == DeviceState::OFFLINE){
		b_emergencyStopActive = false;
		b_halted = false;
	}
	
	//update animatable states
	switch(axis1->getState()){
		case DeviceState::OFFLINE:
			axis1Animatable->state = Animatable::State::OFFLINE;
			break;
		case DeviceState::NOT_READY:
		case DeviceState::READY:
            if(b_halted) axis1Animatable->state = Animatable::State::HALTED;
            else axis1Animatable->state = Animatable::State::NOT_READY;
            break;
		case DeviceState::ENABLED:
            if(b_halted) axis1Animatable->state = Animatable::State::HALTED;
			else axis1Animatable->state = Animatable::State::READY;
			break;
	}
	switch(axis2->getState()){
		case DeviceState::OFFLINE:
			axis2Animatable->state = Animatable::State::OFFLINE;
			break;
		case DeviceState::NOT_READY:
		case DeviceState::READY:
            if(b_halted) axis2Animatable->state = Animatable::State::HALTED;
            else axis2Animatable->state = Animatable::State::NOT_READY;
            break;
		case DeviceState::ENABLED:
			axis2Animatable->state = Animatable::State::READY;
			break;
	}
    
    if(axis1Animatable->state == Animatable::State::READY && axis2Animatable->state == Animatable::State::READY) synchronizedAnimatable->state = Animatable::State::READY;
    else if(axis1Animatable->state == Animatable::State::OFFLINE || axis2Animatable->state == Animatable::State::OFFLINE) synchronizedAnimatable->state = Animatable::State::OFFLINE;
    else synchronizedAnimatable->state = Animatable::State::NOT_READY;
	
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
	
	auto synchronizedActualPosition = AnimationValue::makePosition();
	if(axis1isMaster->value){
		synchronizedActualPosition->position = axis1Animatable->motionProfile.getPosition();
		synchronizedActualPosition->velocity = axis1Animatable->motionProfile.getVelocity();
		synchronizedActualPosition->acceleration = axis1Animatable->motionProfile.getAcceleration();
	}else{
		synchronizedActualPosition->position = axis2Animatable->motionProfile.getPosition();
		synchronizedActualPosition->velocity = axis2Animatable->motionProfile.getVelocity();
		synchronizedActualPosition->acceleration = axis2Animatable->motionProfile.getAcceleration();
	}
	synchronizedAnimatable->updateActualValue(synchronizedActualPosition);
	
	*position1PinValue = axis1RealPosition->position;
	*velocity1PinValue = axis1RealPosition->velocity;
	*position2PinValue = axis2RealPosition->position;
	*velocity2PinValue = axis2RealPosition->velocity;
	
	//update position limits
	axis1Animatable->lowerPositionLimit = axis1PositionToMachinePosition(axis1->getLowPositionLimit());
	axis1Animatable->upperPositionLimit = axis1PositionToMachinePosition(axis1->getHighPositionLimit());
	if(invertAxis1->value) std::swap(axis1Animatable->lowerPositionLimit, axis1Animatable->upperPositionLimit); //swap limits if the axis is inverted
	
	axis2Animatable->lowerPositionLimit = axis2PositionToMachinePosition(axis2->getLowPositionLimit());
	axis2Animatable->upperPositionLimit = axis2PositionToMachinePosition(axis2->getHighPositionLimit());
	if(invertAxis2->value) std::swap(axis2Animatable->lowerPositionLimit, axis2Animatable->upperPositionLimit); //swap limits if the axis is inverted
	
	
	if(enableSynchronousControl->value){
		double axis1Position = axis1Animatable->getActualPosition();
		double axis2Position = axis2Animatable->getActualPosition();
		double positionDifference = std::abs(axis1Position - axis2Position);
		double infinity = std::numeric_limits<double>::infinity();
		if(axis1isMaster->value){
			if(axis1Position < axis2Position) {
				synchronousLowerKeepoutConstraint->adjust(-infinity, axis1Animatable->lowerPositionLimit);
				synchronousUpperKeepoutConstraint->adjust(axis1Animatable->upperPositionLimit - positionDifference, infinity);
			}
			else{
				synchronousLowerKeepoutConstraint->adjust(-infinity, axis1Animatable->lowerPositionLimit + positionDifference);
				synchronousUpperKeepoutConstraint->adjust(axis1Animatable->upperPositionLimit, infinity);
			}
		}else{
			if(axis2Position < axis1Position) {
				synchronousLowerKeepoutConstraint->adjust(-infinity, axis2Animatable->lowerPositionLimit);
				synchronousUpperKeepoutConstraint->adjust(axis2Animatable->upperPositionLimit - positionDifference, infinity);
			}
			else{
				synchronousLowerKeepoutConstraint->adjust(-infinity, axis2Animatable->lowerPositionLimit + positionDifference);
				synchronousUpperKeepoutConstraint->adjust(axis2Animatable->upperPositionLimit, infinity);
			}
		}
		synchronousLowerKeepoutConstraint->enable();
		synchronousUpperKeepoutConstraint->enable();
	}else{
		synchronousLowerKeepoutConstraint->disable();
		synchronousUpperKeepoutConstraint->disable();
	}
	
	
	if(enableAntiCollision->value){
		if(axis1isAboveAxis2->value){
			//Axis 1 > Axis 2
			double axis1MinPosition = axis2Animatable->getBrakingPosition() + std::abs(minimumDistanceBetweenAxes->value);
			axis1AnticollisionConstraint->adjust(-std::numeric_limits<double>::infinity(), axis1MinPosition);
			double axis2MaxPosition = axis1Animatable->getBrakingPosition() - std::abs(minimumDistanceBetweenAxes->value);
			axis2AnticollisionConstraint->adjust(axis2MaxPosition, std::numeric_limits<double>::infinity());
			
		}else{
			//Axis 1 < Axis 2
			double axis1MaxPosition = axis2Animatable->getBrakingPosition() - std::abs(minimumDistanceBetweenAxes->value);
			axis1AnticollisionConstraint->adjust(axis1MaxPosition, std::numeric_limits<double>::infinity());
			double axis2MinPosition = axis1Animatable->getBrakingPosition() + std::abs(minimumDistanceBetweenAxes->value);
			axis2AnticollisionConstraint->adjust(-std::numeric_limits<double>::infinity(), axis2MinPosition);
		}
		axis1AnticollisionConstraint->enable();
		axis2AnticollisionConstraint->enable();
	}else{
		axis1AnticollisionConstraint->disable();
		axis2AnticollisionConstraint->disable();
	}
	
	
	
}

void SharedAxisMachine::outputProcess(){
	
	if(b_emergencyStopActive) {
		axis1Animatable->stopMovement();
		axis1Animatable->stopAnimation();
		axis2Animatable->stopMovement();
		axis2Animatable->stopAnimation();
		synchronizedAnimatable->stopMovement();
		synchronizedAnimatable->stopAnimation();
	}
	
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
	
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	if (!isEnabled() || axis1->isHoming() || axis2->isHoming()) {
		//if the machine is not enabled or is homing, the animatable doesn't do anything
		//we juste copy real values to the motion profile of the animatable
		axis1Animatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		axis2Animatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		synchronizedAnimatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		controlMode = ControlMode::NONE;
	}
	else{
		
		//decide which control mode we are in
		//synchronous control has priority
		if(b_forceSynchronousControl || synchronizedAnimatable->isControlledManuallyOrByAnimation() || synchronizedAnimatable->controlMode == AnimatablePosition::POSITION_SETPOINT) {
			controlMode = ControlMode::SYNCHRONOUS;
		}
		else if(axis1Animatable->isControlledManuallyOrByAnimation() || axis2Animatable->isControlledManuallyOrByAnimation()) {
			controlMode = ControlMode::INDIVIDUAL;
		}
		else {
			controlMode = ControlMode::NONE;
		}
		
		auto masterAnimatable = axis1isMaster->value ? axis1Animatable : axis2Animatable;
		auto slaveAnimatable = axis1isMaster->value ? axis2Animatable : axis1Animatable;
		
		//if we are in synchronous control and the synchronoous animatable has a position setpoint
		//only exit synchronous mode if the master profile has reached the synchronous profile target
		if(controlMode == ControlMode::SYNCHRONOUS &&
		   synchronizedAnimatable->controlMode == AnimatablePosition::POSITION_SETPOINT &&
		   !synchronizedAnimatable->isControlledManuallyOrByAnimation()){
			
			if(masterAnimatable->motionProfile.getPosition() == synchronizedAnimatable->motionProfile.getPosition() &&
			   masterAnimatable->motionProfile.getAcceleration() == synchronizedAnimatable->motionProfile.getAcceleration() &&
			   masterAnimatable->motionProfile.getVelocity() == synchronizedAnimatable->motionProfile.getVelocity()){
				controlMode = ControlMode::NONE;
			}
			
		}
		
		
		switch(controlMode){
			case ControlMode::NONE:
				
				axis1Animatable->controlMode = AnimatablePosition::VELOCITY_SETPOINT;
				axis1Animatable->velocitySetpoint = 0.0;
				axis2Animatable->controlMode = AnimatablePosition::VELOCITY_SETPOINT;
				axis2Animatable->velocitySetpoint = 0.0;
				synchronizedAnimatable->controlMode = AnimatablePosition::VELOCITY_SETPOINT;
				synchronizedAnimatable->velocitySetpoint = 0.0;
				
				axis1Animatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				axis2Animatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				synchronizedAnimatable->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
				
				break;
				
			case ControlMode::INDIVIDUAL:
				
				synchronizedAnimatable->stopMovement();
				synchronizedAnimatable->stopAnimation();
			
				synchronizedAnimatable->controlMode = AnimatablePosition::FORCED_POSITION_SETPOINT;
				synchronizedAnimatable->motionProfile.setPosition(masterAnimatable->motionProfile.getPosition());
				synchronizedAnimatable->motionProfile.setVelocity(masterAnimatable->motionProfile.getVelocity());
				synchronizedAnimatable->motionProfile.setAcceleration(masterAnimatable->motionProfile.getAcceleration());
				
				axis1Animatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				axis2Animatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				break;
				
			case ControlMode::SYNCHRONOUS:
								
				axis1Animatable->stopMovement();
				axis1Animatable->stopAnimation();
				axis2Animatable->stopMovement();
				axis2Animatable->stopAnimation();
				
				synchronizedAnimatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);

				masterAnimatable->forcePositionTarget(synchronizedAnimatable->motionProfile.getPosition(),
													  synchronizedAnimatable->motionProfile.getVelocity(),
													  synchronizedAnimatable->motionProfile.getAcceleration());
				masterAnimatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				
				slaveAnimatable->forceVelocityTarget(masterAnimatable->motionProfile.getVelocity(), masterAnimatable->motionProfile.getAcceleration());
				slaveAnimatable->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
				
				
				 
				double sync = synchronizedAnimatable->motionProfile.getPosition();
				double master = masterAnimatable->motionProfile.getPosition();
				//Logger::warn("diff: {}", sync - master);

				break;
		}
			
		
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
	
	
	//update machine state
	if(state == DeviceState::ENABLED) { do nothing }
	else if(areAxesConnected() && axesHaveSamePositionUnit()) state = DeviceState::READY;
	else state = DeviceState::OFFLINE;
	
	//update halt and estop state
	b_emergencyStopActive = false;
	b_halted = false;
		
	//update animatable state
	if(state == DeviceState::OFFLINE) {
		axis1Animatable->state = Animatable::State::OFFLINE;
		axis2Animatable->state = Animatable::State::OFFLINE;
		synchronizedAnimatable->state = Animatable::State::OFFLINE;
	}
	else if(state == DeviceState::ENABLED && !b_halted){
		axis1Animatable->state = Animatable::State::READY;
		axis2Animatable->state = Animatable::State::READY;
		synchronizedAnimatable->state = Animatable::State::READY;
	}
	else{
		axis1Animatable->state = Animatable::State::NOT_READY;
		axis2Animatable->state = Animatable::State::NOT_READY;
		synchronizedAnimatable->state = Animatable::State::NOT_READY;
	}
	
	if(enableAntiCollision->value){
		if(axis1isAboveAxis2->value){
			//Axis 1 > Axis 2
			double axis1MinPosition = axis2Animatable->getBrakingPosition() + std::abs(minimumDistanceBetweenAxes->value);
			axis1AnticollisionConstraint->adjust(-std::numeric_limits<double>::infinity(), axis1MinPosition);
			double axis2MaxPosition = axis1Animatable->getBrakingPosition() - std::abs(minimumDistanceBetweenAxes->value);
			axis2AnticollisionConstraint->adjust(axis2MaxPosition, std::numeric_limits<double>::infinity());
			
		}else{
			//Axis 1 < Axis 2
			double axis1MaxPosition = axis2Animatable->getBrakingPosition() - std::abs(minimumDistanceBetweenAxes->value);
			axis1AnticollisionConstraint->adjust(axis1MaxPosition, std::numeric_limits<double>::infinity());
			double axis2MinPosition = axis1Animatable->getBrakingPosition() + std::abs(minimumDistanceBetweenAxes->value);
			axis2AnticollisionConstraint->adjust(-std::numeric_limits<double>::infinity(), axis2MinPosition);
		}
		axis1AnticollisionConstraint->enable();
		axis2AnticollisionConstraint->enable();
	}else{
		axis1AnticollisionConstraint->disable();
		axis2AnticollisionConstraint->disable();
	}
}

void SharedAxisMachine::simulateOutputProcess(){
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	axis1Animatable->simulateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
	axis2Animatable->simulateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
	synchronizedAnimatable->simulateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
	
	axis1Animatable->updateActualValue(axis1Animatable->getTargetValue());
	axis2Animatable->updateActualValue(axis2Animatable->getTargetValue());
}

bool SharedAxisMachine::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

void SharedAxisMachine::startHomingAxis1(){
	axis1Animatable->stopMovement();
	synchronizedAnimatable->stopMovement();
	getAxis1()->startHoming();
}

void SharedAxisMachine::startHomingAxis2(){
	axis2Animatable->stopMovement();
	synchronizedAnimatable->stopMovement();
	getAxis2()->startHoming();
}

void SharedAxisMachine::startHomingBothAxes(){
	startHomingAxis1();
	startHomingAxis2();
}












//========= ANIMATABLE OWNER ==========

void SharedAxisMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	auto animatable = animation->getAnimatable();
	if(animatable != axis1Animatable && animatable != axis2Animatable && animatable != synchronizedAnimatable) return;
	
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
	horizontalControls->save(generalXML);
	enableAntiCollision->save(generalXML);
	axis1isAboveAxis2->save(generalXML);
	minimumDistanceBetweenAxes->save(generalXML);
	enableSynchronousControl->save(generalXML);
	axis1isMaster->save(generalXML);
	allowUserHoming->save(generalXML);
	
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
	if(!horizontalControls->load(generalXML)) return false;
	if(!enableAntiCollision->load(generalXML)) return false;
	if(!axis1isAboveAxis2->load(generalXML)) return false;
	if(!minimumDistanceBetweenAxes->load(generalXML)) return false;
	if(!enableSynchronousControl->load(generalXML)) return false;
	enableSynchronousControl->onEdit();
	if(!axis1isMaster->load(generalXML)) return false;
	if(!allowUserHoming->load(generalXML)) return false;
	
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
	double positionSetpoint = getAxis1()->getProfilePosition();
	if(invertAxis1->value) axis1Offset->overwriteWithHistory(-1.f * positionSetpoint - machinePosition);
	else axis1Offset->overwriteWithHistory(positionSetpoint - machinePosition);
	axis1Animatable->motionProfile.setPosition(machinePosition);
}

void SharedAxisMachine::captureAxis2PositionToOffset(double machinePosition){
	double positionSetpoint = getAxis2()->getProfilePosition();
	if(invertAxis2->value) axis2Offset->overwriteWithHistory(positionSetpoint + machinePosition);
	else axis2Offset->overwriteWithHistory(positionSetpoint - machinePosition);
	axis2Animatable->motionProfile.setPosition(machinePosition);
}

double SharedAxisMachine::axis1PositionToMachinePosition(double axis1Position){
	if(invertAxis1->value) return -1.f * (axis1Position - axis1Offset->value);
	return axis1Position - axis1Offset->value;
}

double SharedAxisMachine::axis2PositionToMachinePosition(double axis2Position){
	if(invertAxis2->value) return -1.f * (axis2Position - axis2Offset->value);
	return axis2Position - axis2Offset->value;
}

double SharedAxisMachine::machinePositionToAxis1Position(double machineAxis1Position){
	if(invertAxis1->value) return (-1.f * machineAxis1Position) + axis1Offset->value;
	return machineAxis1Position + axis1Offset->value;
}

double SharedAxisMachine::machinePositionToAxis2Position(double machineAxis2Position){
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

*/
