#include <pch.h>

#include "FlipStateMachine.h"
#include "Motion/SubDevice.h"
#include "Animation/Animatable.h"

#include <tinyxml2.h>

#include "Animation/Animation.h"

AnimatableStateStruct FlipStateMachine::stateUnknown = 			{-1, 	"Unknown", 			"Unknown"};
AnimatableStateStruct FlipStateMachine::stateStopped = 			{0, 	"Stopped", 			"Stopped"};
AnimatableStateStruct FlipStateMachine::stateClosed = 			{1, 	"Closed", 			"Closed"};
AnimatableStateStruct FlipStateMachine::stateClosingOpening = 	{2, 	"Opening/Closing", 	"OpeningClosing"};
AnimatableStateStruct FlipStateMachine::stateOpenLowered = 		{3, 	"Open & Lowered", 	"OpenLowered"};
AnimatableStateStruct FlipStateMachine::stateRaisingLowering = 	{4, 	"Raising/Lowering", "RaisingLowering"};
AnimatableStateStruct FlipStateMachine::stateRaised = 			{5, 	"Raised", 			"Raised"};

std::vector<AnimatableStateStruct*> FlipStateMachine::allStates = {
	&FlipStateMachine::stateUnknown,
	&FlipStateMachine::stateStopped,
	&FlipStateMachine::stateClosed,
	&FlipStateMachine::stateClosingOpening,
	&FlipStateMachine::stateOpenLowered,
	&FlipStateMachine::stateRaisingLowering,
	&FlipStateMachine::stateRaised
};

std::vector<AnimatableStateStruct*> FlipStateMachine::selectableStates = {
	&FlipStateMachine::stateStopped,
	&FlipStateMachine::stateClosed,
	&FlipStateMachine::stateOpenLowered,
	&FlipStateMachine::stateRaised
};



void FlipStateMachine::initialize() {
		
	addNodePin(gpioDeviceLink);
	addNodePin(hoodOpenSignalPin);
	addNodePin(hoodShutSignalPin);
	addNodePin(liftRaisedSignalPin);
	addNodePin(liftLoweredSignalPin);
	addNodePin(hoodMotorCircuitBreakerSignalPin);
	addNodePin(liftMotorCircuitBreakerSignalPin);
	addNodePin(emergencyStopClearSignalPin);
	addNodePin(localControlEnabledSignalPin);

	addNodePin(openHoodCommandPin);
	addNodePin(shutHoodCommandPin);
	addNodePin(raiseLiftCommandPin);
	addNodePin(lowerLiftCommandPin);
	addNodePin(stateIntegerPin);

	addAnimatable(animatableState);
	
	auto thisMachine = std::static_pointer_cast<FlipStateMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
}

std::string FlipStateMachine::getStatusString(){
	return "No Status String Available Yet";
}

void FlipStateMachine::inputProcess() {
		
	if(!areAllPinsConnected()){
		state = MotionState::OFFLINE;
		actualState = State::UNKNOWN;
		b_emergencyStopActive = false;
		b_halted = false;
		return;
	}
	
	auto gpioDevice = getGpioDevice();
	MotionState gpioState = gpioDevice->getState();
		
	//if the gpio device provides valid values, update the state of the flip machine
	if(gpioState != MotionState::OFFLINE){
		hoodOpenSignalPin->copyConnectedPinValue();
		hoodShutSignalPin->copyConnectedPinValue();
		liftRaisedSignalPin->copyConnectedPinValue();
		liftLoweredSignalPin->copyConnectedPinValue();
		hoodMotorCircuitBreakerSignalPin->copyConnectedPinValue();
		liftMotorCircuitBreakerSignalPin->copyConnectedPinValue();
		emergencyStopClearSignalPin->copyConnectedPinValue();
		localControlEnabledSignalPin->copyConnectedPinValue();
		
		b_emergencyStopActive = !*emergencyStopClearSignal;
		b_halted = isEnabled() && !isMotionAllowed();
		
		if (*hoodShutSignal && !*hoodOpenSignal) {
			if (*liftLoweredSignal && !*liftRaisedSignal) actualState = State::CLOSED;
			else actualState = State::UNKNOWN;
		}
		else if (*hoodOpenSignal && !*hoodShutSignal) {
			if (*liftLoweredSignal && !*liftRaisedSignal) actualState = State::OPEN_LOWERED;
			else if (*liftRaisedSignal && !*liftLoweredSignal) actualState = State::RAISED;
			else if (!*liftLoweredSignal && !*liftRaisedSignal) actualState = State::LOWERING_RAISING;
			else actualState = State::UNKNOWN;
		}
		else if (!*hoodOpenSignal && !*hoodShutSignal) {
			if (*liftLoweredSignal && !*liftRaisedSignal) actualState = State::OPENING_CLOSING;
			else actualState = State::UNKNOWN;
		}
		else actualState = State::UNKNOWN;
	}else{
		state = MotionState::OFFLINE;
		actualState = State::UNKNOWN;
		b_emergencyStopActive = false;
		b_halted = false;
		return;
	}
	
	MotionState newState;
	if(actualState == State::UNKNOWN) newState = MotionState::OFFLINE;
	else if (!*emergencyStopClearSignal) newState = MotionState::NOT_READY;
	else if (*localControlEnabledSignal) newState = MotionState::NOT_READY;
	else if (*liftMotorCircuitBreakerSignal) newState = MotionState::NOT_READY;
	else if (*hoodMotorCircuitBreakerSignal) newState = MotionState::NOT_READY;
	else if (!isEnabled()) newState = MotionState::READY;
	else newState = MotionState::ENABLED;
		
	if(isEnabled() && newState != MotionState::ENABLED) disable();
	state = newState;
	
	//update animatable state
	if(state == MotionState::OFFLINE) animatableState->state = Animatable::State::OFFLINE;
	else if(state == MotionState::ENABLED && !b_halted) animatableState->state = Animatable::State::READY;
	else animatableState->state = Animatable::State::NOT_READY;
	
	auto actualStateValue = AnimationValue::makeState();
	switch(actualState){
		case State::CLOSED: 		actualStateValue->value = &stateClosed; break;
		case State::OPEN_LOWERED: 	actualStateValue->value = &stateOpenLowered; break;
		case State::RAISED: 		actualStateValue->value = &stateRaised; break;
		default: 					actualStateValue->value = &stateStopped; break;
	}
	animatableState->updateActualValue(actualStateValue);
	
	*stateIntegerValue = getStateInteger(actualState);
}

void FlipStateMachine::outputProcess(){
	
	//handle dead mans switch
	if(!isMotionAllowed()){
		if(animatableState->hasAnimation()) animatableState->getAnimation()->pausePlayback();
		animatableState->stopMovement();
	}
	
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	//update outputs signals
	if (!isEnabled()) {
		
		animatableState->followActualValue(profileTime_seconds, profileDeltaTime_seconds);
		
		*shutHoodSignal = false;
		*openHoodSignal = false;
		*lowerLiftSignal = false;
		*raiseLiftSignal = false;
		
	}else{
		
		animatableState->updateTargetValue(profileTime_seconds, profileDeltaTime_seconds);
		auto targetValue = animatableState->getTargetValue()->toState()->value;
		
		if(targetValue == &stateClosed) requestedState = State::CLOSED;
		else if(targetValue == &stateOpenLowered) requestedState = State::OPEN_LOWERED;
		else if(targetValue == &stateRaised) requestedState = State::RAISED;
		else requestedState = State::STOPPED;
		
		switch (requestedState) {
			case State::CLOSED:
				*shutHoodSignal = true;
				*openHoodSignal = false;
				*lowerLiftSignal = true;
				*raiseLiftSignal = false;
				break;
			case State::OPEN_LOWERED:
				*shutHoodSignal = false;
				*openHoodSignal = true;
				*lowerLiftSignal = true;
				*raiseLiftSignal = false;
				break;
			case State::RAISED:
				*shutHoodSignal = false;
				*openHoodSignal = true;
				*lowerLiftSignal = false;
				*raiseLiftSignal = true;
				break;
			default:
				//in any other state (include STOPPED), we stop all motion
				*shutHoodSignal = false;
				*openHoodSignal = false;
				*lowerLiftSignal = false;
				*raiseLiftSignal = false;
				break;
		}
	}
	
}

bool FlipStateMachine::isHardwareReady() { 
	if (!areAllPinsConnected()) return false;
	else if (actualState == State::UNKNOWN) return false;
	else if (*localControlEnabledSignal) return false;
	else if (*hoodMotorCircuitBreakerSignal) return false;
	else if (*liftMotorCircuitBreakerSignal) return false;
	else if (!*emergencyStopClearSignal) return false;
	return true;
}

void FlipStateMachine::enableHardware() {
	if (!isEnabled() && isReady()) {
		requestedState = State::STOPPED;
		state = MotionState::ENABLED;
		onEnableHardware();
	}
}

void FlipStateMachine::disableHardware() {
	if(isEnabled()){
		state = MotionState::READY;
		requestedState = State::STOPPED;
		onDisableHardware();
	}
}


void FlipStateMachine::onEnableHardware() {}

void FlipStateMachine::onDisableHardware() {}

void FlipStateMachine::simulateInputProcess() {
	
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
}

void FlipStateMachine::simulateOutputProcess() {
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
	
}

bool FlipStateMachine::isSimulationReady(){
	return true;
}


void FlipStateMachine::onEnableSimulation() {}
void FlipStateMachine::onDisableSimulation() {}




bool FlipStateMachine::isGpioDeviceConnected() {
	return gpioDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> FlipStateMachine::getGpioDevice() {
	return gpioDeviceLink->getConnectedPins().front()->getSharedPointer<GpioDevice>();
}

bool FlipStateMachine::areAllPinsConnected() {
	//device
	if (!isGpioDeviceConnected()) return false;
	//inputs
	if (!hoodOpenSignalPin->isConnected()) return false;
	if (!hoodShutSignalPin->isConnected()) return false;
	if (!liftRaisedSignalPin->isConnected()) return false;
	if (!liftLoweredSignalPin->isConnected()) return false;
	if (!emergencyStopClearSignalPin->isConnected()) return false;
	if (!localControlEnabledSignalPin->isConnected()) return false;
	if (!hoodMotorCircuitBreakerSignalPin->isConnected()) return false;
	if (!liftMotorCircuitBreakerSignalPin->isConnected()) return false;
	//outputs
	if (!raiseLiftCommandPin->isConnected()) return false;
	if (!lowerLiftCommandPin->isConnected()) return false;
	if (!shutHoodCommandPin->isConnected()) return false;
	if (!openHoodCommandPin->isConnected()) return false;
	return true;
}

bool FlipStateMachine::isMoving() { 
	return actualState == State::LOWERING_RAISING || actualState == State::OPENING_CLOSING;
}


void FlipStateMachine::requestState(State newState){
	animatableState->stopMovement();
	auto targetValue = AnimationValue::makeState();
	switch(newState){
		case State::STOPPED: 		targetValue->value = &stateStopped; break;
		case State::CLOSED:			targetValue->value = &stateClosed; break;
		case State::OPEN_LOWERED:	targetValue->value = &stateOpenLowered; break;
		case State::RAISED:			targetValue->value = &stateRaised; break;
		default:
			targetValue = nullptr;
			Logger::error("machine {} = state {} is not selectable", getName(), Enumerator::getDisplayString(newState));
			break;
	}
	if(targetValue) {
		animatableState->rapidToValue(targetValue);
		Logger::info("rapid {} to value {}", getName(), targetValue->value->displayName);
	}
}


/*

void FlipStateMachine::rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	if (animatable == animatableState) {
		auto state = value->toState()->value;
		switch (state->integerEquivalent) {
			case 0:
				requestedState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
				break;
			case 1:
				requestedState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
				break;
			case 2:
				requestedState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
				break;
		}
		parameterMovementTargetState = requestedState;
	}
}

float FlipStateMachine::getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable) {
	if (animatable == animatableState) {
		float actual = getState(actualState)->floatEquivalent;
		float start = getState(parameterMovementStartState)->floatEquivalent;
		float target = getState(parameterMovementTargetState)->floatEquivalent;
		float progress = (actual - start) / (target - start);
		if (progress < start) progress = start;
		else if (progress > target) progress = target;
		return progress;
	}
	return 0.0;
}

bool FlipStateMachine::isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	if (animatable == animatableState) {
		auto state = value->toState()->value;
		//TODO: this is broken
		if (state->integerEquivalent == getState(actualState)->floatEquivalent) return true;
	}
	return false;
}

void FlipStateMachine::onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) {}

void FlipStateMachine::onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) {}

void FlipStateMachine::onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) {}

std::shared_ptr<AnimationValue> FlipStateMachine::getActualAnimatableValue(std::shared_ptr<Animatable> animatable) {
	auto output = AnimationValue::makeState();
	output->value = &stateParameterValues.front();
	switch(actualState){
		case FlipStateMachine::MachineState::State::UNKNOWN:
		case FlipStateMachine::MachineState::State::UNEXPECTED_STATE: output->value = &stateParameterValues.front(); break;
		case FlipStateMachine::MachineState::State::LIFT_LOWERED_HOOD_SHUT: output->value = &stateParameterValues[1]; break;
		case FlipStateMachine::MachineState::State::LIFT_LOWERED_HOOD_MOVING: output->value = &stateParameterValues.front(); break;
		case FlipStateMachine::MachineState::State::LIFT_LOWERED_HOOD_OPEN: output->value = &stateParameterValues[2]; break;
		case FlipStateMachine::MachineState::State::LIFT_MOVING_HOOD_OPEN: output->value = &stateParameterValues.front(); break;
		case FlipStateMachine::MachineState::State::LIFT_RAISED_HOOD_OPEN: output->value = &stateParameterValues[2]; break;
	}
	return output;
}



void FlipStateMachine::cancelAnimatableRapid(std::shared_ptr<Animatable> animatable) {
	
}


void FlipStateMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	
}

bool FlipStateMachine::validateAnimation(const std::shared_ptr<Animation> animation) {
	parameterTrack->b_valid = true;
	for (auto& curve : parameterTrack->curves) {
		curve->b_valid = true;
		for (auto& point : curve->points) {
			point->b_valid = true;
			point->validationError = Motion::ValidationError::NO_VALIDATION_ERROR;
		}
		for (auto& interpolation : curve->interpolations) {
			interpolation->b_valid = true;
			interpolation->validationError = Motion::ValidationError::NO_VALIDATION_ERROR;
		}
	}
	return true;
}

bool FlipStateMachine::generateTargetAnimation(std::shared_ptr<TargetAnimation> targetAnimation){
	return false;
}

*/



//========= ANIMATABLE OWNER ==========

void FlipStateMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){}


void FlipStateMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isGpioDeviceConnected()) output.push_back(getGpioDevice()->parentDevice);
}



bool FlipStateMachine::loadMachine(tinyxml2::XMLElement* xml) {
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
bool FlipStateMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* controlWidgetXML = xml->InsertNewChildElement("ControlWidget");
	controlWidgetXML->SetAttribute("UniqueID", controlWidget->uniqueID);
	return true;
}
