#include <pch.h>

#include "AxisStateMachine.h"
#include "Motion/SubDevice.h"
#include "Animation/Animatable.h"

#include <tinyxml2.h>

#include "Animation/Animation.h"

#include "Motion/Axis/VelocityControlledAxis.h"

AnimatableStateStruct AxisStateMachine::stateUnknown =					{0, "Unknown", 					"Unknown"};
AnimatableStateStruct AxisStateMachine::stateStopped =					{0, "Stopped", 					"Stopped"};
AnimatableStateStruct AxisStateMachine::stateMovingToNegativeLimit =	{0, "Moving To Negative Limit", "MovingToNegativeLimit"};
AnimatableStateStruct AxisStateMachine::stateMovingToPositiveLimit =	{0, "Moving To Positive Limit", "MovingToPositiveLimit"};
AnimatableStateStruct AxisStateMachine::stateNegativeLimit =			{0, "At Negative Limit", 		"AtNegativeLimit"};
AnimatableStateStruct AxisStateMachine::statePositiveLimit =			{0, "At Positive Limit", 		"AtPositiveLimit"};

std::vector<AnimatableStateStruct*> AxisStateMachine::allStates = {
	&AxisStateMachine::stateUnknown,
	&AxisStateMachine::stateStopped,
	&AxisStateMachine::stateMovingToNegativeLimit,
	&AxisStateMachine::stateMovingToPositiveLimit,
	&AxisStateMachine::stateNegativeLimit,
	&AxisStateMachine::statePositiveLimit
};

std::vector<AnimatableStateStruct*> AxisStateMachine::selectableStates = {
	&AxisStateMachine::stateNegativeLimit,
	&AxisStateMachine::statePositiveLimit
};

void AxisStateMachine::initialize() {
		
	//input pin
	addNodePin(axisPin);
	
	//output pin
	addNodePin(statePin);
		
	addAnimatable(animatableState);
	addAnimatable(animatableVelocity);
	
	auto thisMachine = std::static_pointer_cast<AxisStateMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine);
}

std::string AxisStateMachine::getStatusString(){
	return "No Status String Available Yet";
}

void AxisStateMachine::inputProcess() {
		
	if(!areAllPinsConnected()){
		state = MotionState::OFFLINE;
		actualState = State::UNKNOWN;
		b_emergencyStopActive = false;
		b_halted = false;
		return;
	}
	
	auto axis = getAxis();
	
	//update estop state
	b_emergencyStopActive = axis->isEmergencyStopActive();
	
	//handle transition from enabled state
	MotionState newState = axis->getState();
	if(isEnabled() && newState != MotionState::ENABLED) disable();
	state = newState;
	
	//update state of animatables
	switch(state){
		case MotionState::OFFLINE:
			animatableState->state = Animatable::State::OFFLINE;
			animatableVelocity->state = Animatable::State::OFFLINE;
			break;
		case MotionState::NOT_READY:
		case MotionState::READY:
            animatableState->state = Animatable::State::NOT_READY;
            animatableVelocity->state = Animatable::State::NOT_READY;
            break;
		case MotionState::ENABLED:
			animatableState->state = Animatable::State::READY;
			animatableVelocity->state = Animatable::State::READY;
			break;
	}
	 
	
	//get actual axis state
	if(axis->isAtLowerLimit()) actualState = State::AT_NEGATIVE_LIMIT;
	else if(axis->isAtUpperLimit()) actualState = State::AT_POSITIVE_LIMIT;
	else if(axis->getProfileVelocity_axisUnitsPerSecond() > 0.0) actualState = State::MOVING_TO_POSITIVE_LIMIT;
	else if(axis->getProfileVelocity_axisUnitsPerSecond() < 0.0) actualState = State::MOVING_TO_NEGATIVE_LIMIT;
	else if(axis->getProfileVelocity_axisUnitsPerSecond() == 0.0) actualState = State::STOPPED;
	else actualState = State::UNKNOWN;
	
	//update actual value of animatables
	std::shared_ptr<AnimatableStateValue> newStateValue = AnimationValue::makeState();
	std::shared_ptr<AnimatableRealValue> newVelocityValue = AnimationValue::makeReal();
	newStateValue->value = getStateStruct(actualState);
	newVelocityValue->value = *axis->actualVelocity;
	animatableState->updateActualValue(newStateValue);
	animatableVelocity->updateActualValue(newVelocityValue);

	//update state pin
	*stateInteger = getStateInteger(actualState);
}

void AxisStateMachine::outputProcess(){
	
	//handle dead mans switch
	if(!isMotionAllowed()){
		if(animatableState->hasAnimation()) animatableState->getAnimation()->pausePlayback();
		animatableState->stopMovement();
		if(animatableVelocity->hasAnimation()) animatableVelocity->getAnimation()->pausePlayback();
		animatableVelocity->stopMovement();
	}
	
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	//update outputs signals
	if (getState() != MotionState::ENABLED) {
		
		animatableState->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		animatableVelocity->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		
	}else{
		
		//get new target values
		animatableState->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		animatableVelocity->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		requestedState = getStateEnumerator(animatableState->getTargetValue()->toState()->value);
		velocityTarget = animatableVelocity->getTargetValue()->toReal()->value;
		
		switch(requestedState){
			case State::UNKNOWN:
			case State::STOPPED:
				velocityTarget = 0.0;
				break;
			case State::MOVING_TO_POSITIVE_LIMIT:
			case State::AT_POSITIVE_LIMIT:
				velocityTarget = std::abs(velocityTarget);
				break;
			case State::MOVING_TO_NEGATIVE_LIMIT:
			case State::AT_NEGATIVE_LIMIT:
				velocityTarget = -std::abs(velocityTarget);
				break;
		}
		
		auto axis = getAxis();
		axis->setVelocityCommand(velocityTarget, axis->getAccelerationLimit());
		
	}
}



bool AxisStateMachine::isHardwareReady() {
    if (!isAxisConnected()) return false;
    auto axis = getAxis();
    if(axis->getState() != MotionState::READY) return false;
    return true;
}

void AxisStateMachine::enableHardware() {
    if (isReady()) {
        std::thread machineEnabler([this]() {
            using namespace std::chrono;
            auto axis = getAxis();
            axis->enable();
            time_point enableRequestTime = system_clock::now();
            while (duration(system_clock::now() - enableRequestTime) < milliseconds(500)) {
                std::this_thread::sleep_for(milliseconds(10));
                if (axis->getState() == MotionState::ENABLED) {
                    state = MotionState::ENABLED;
                    onEnableHardware();
                    break;
                }
            }
            });
        machineEnabler.detach();
    }
}

void AxisStateMachine::disableHardware() {
    state = MotionState::READY;
    if (isAxisConnected()) getAxis()->disable();
    onDisableHardware();
}


void AxisStateMachine::onEnableHardware() {
    requestState(State::STOPPED);
}

void AxisStateMachine::onDisableHardware() {
    requestState(State::STOPPED);
}

void AxisStateMachine::simulateInputProcess() {
	/*
	if(state != MotionState::ENABLED) state = MotionState::READY;
	
	*stateIntegerValue = getStateInteger(actualState);
	
	auto actualValue = AnimationValue::makeState();
	switch(actualState){
		case State::UNKNOWN:			actualValue->value = &stateUnknown; break;
		case State::STOPPED:			actualValue->value = &stateStopped; break;
		case State::CLOSED:				actualValue->value = &stateClosed; break;
		case State::OPENING_CLOSING:	actualValue->value = &stateClosingOpening; break;
		case State::OPEN_LOWERED:		actualValue->value = &stateOpenLowered; break;
		case State::LOWERING_RAISING:	actualValue->value = &stateRaisingLowering; break;
		case State::RAISED:				actualValue->value = &stateRaised; break;
	}
	animatableState->updateActualValue(actualValue);
	 */
}

void AxisStateMachine::simulateOutputProcess() {
	/*
	//update outputs signals
	if (isEnabled()) {
		
		double profileTime_seconds = Environnement::getTime_seconds();
		double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
		animatableState->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		auto targetValue = animatableState->getTargetValue()->toState();
		animatableState->updateActualValue(targetValue);
		
		auto targetState = targetValue->value;
		
		if(targetState == &stateClosed) requestedState = State::CLOSED;
		else if(targetState == &stateOpenLowered) requestedState = State::OPEN_LOWERED;
		else if(targetState == &stateRaised) requestedState = State::RAISED;
		else requestedState = State::STOPPED;
		
		if(requestedState == State::STOPPED) actualState = State::CLOSED;
		else actualState = requestedState;
	}
	*/
}

bool AxisStateMachine::isSimulationReady(){
	return true;
}


void AxisStateMachine::onEnableSimulation() {}
void AxisStateMachine::onDisableSimulation() {}



bool isAxisConnected();
std::shared_ptr<VelocityControlledAxis> getAxis();


bool AxisStateMachine::isAxisConnected(){
	return axisPin->isConnected();
}

std::shared_ptr<VelocityControlledAxis> AxisStateMachine::getAxis(){
	return axisPin->getConnectedPin()->getSharedPointer<VelocityControlledAxis>();
}

bool AxisStateMachine::areAllPinsConnected() {
	if(!isAxisConnected()) return false;
	return true;
}

bool AxisStateMachine::isMoving() {
	return actualState == State::MOVING_TO_NEGATIVE_LIMIT || actualState == State::MOVING_TO_POSITIVE_LIMIT;
}


void AxisStateMachine::requestState(State newState){
	animatableState->stopMovement();
	animatableVelocity->stopMovement();
    
    AnimatableStateStruct* targetState;
	switch(newState){
		case State::AT_NEGATIVE_LIMIT:
            targetState = &stateNegativeLimit;
			break;
		case State::AT_POSITIVE_LIMIT:
            targetState = &statePositiveLimit;
            break;
		default:
            targetState = nullptr;
			break;
	}
    
    if(targetState){
        auto targetValue = AnimationValue::makeState();
        targetValue->value = targetState;
        auto targetVelocity = AnimationValue::makeReal();
        targetVelocity->value = getAxis()->getVelocityLimit();
        animatableState->rapidToValue(targetValue);
        animatableVelocity->rapidToValue(targetVelocity);
    }
    
}

void AxisStateMachine::requestVelocityNormalized(double velocityNormalized){
	animatableState->stopMovement();
	animatableVelocity->stopMovement();
    
    double velocityLimit = getAxis()->getVelocityLimit();
    double requestedVelocity = std::clamp(velocityNormalized * velocityLimit, -velocityLimit, velocityLimit);
    
    auto stateTarget = AnimationValue::makeState();
    auto velocityTarget = AnimationValue::makeReal();
    velocityTarget->value = requestedVelocity;
    
    if(requestedVelocity > 0.0) stateTarget->value = &statePositiveLimit;
    else if(requestedVelocity < 0.0) stateTarget->value = &stateNegativeLimit;
    else stateTarget->value = &stateStopped;
	
    animatableState->rapidToValue(stateTarget);
    animatableVelocity->rapidToValue(velocityTarget);
}

//========= ANIMATABLE OWNER ==========

void AxisStateMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){}


void AxisStateMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if(isAxisConnected()){
		auto axis = getAxis();
		axis->getDevices(output);
	}
}



bool AxisStateMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* controlWidgetXML = xml->FirstChildElement("ControlWidget");
	if(!controlWidgetXML){
		Logger::warn("Could not find hooded lift state machine control widget unique id");
		return false;
	}
	if(controlWidgetXML->QueryIntAttribute("UniqueID", &controlWidget->uniqueID) != XML_SUCCESS){
		Logger::warn("Could not find hooded lift state machine control widget unique id");
		return false;
	}
	return true;
}
bool AxisStateMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	return true;
}
