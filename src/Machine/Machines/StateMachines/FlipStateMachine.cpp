#include <pch.h>

#include "FlipStateMachine.h"
#include "Motion/SubDevice.h"
#include "Animation/Animatable.h"

#include <tinyxml2.h>

#include "Animation/Animation.h"

std::vector<AnimatableStateStruct> FlipStateMachine::stateParameterValues = {
	{-1, "Stopped", "Stopped"},
	{0, "Shut", "Shut"},
	{1, "Open", "Open"},
	{2, "Raised", "Raised"},
	{3, "Unknown", "Unknown"}
};

void FlipStateMachine::initialize() {
	
	openHoodCommandPin->assignData(openHoodCommandPinValue);
	shutHoodCommandPin->assignData(shutHoodCommandPinValue);
	raiseLiftCommandPin->assignData(raiseLiftCommandPinValue);
	lowerLiftCommandPin->assignData(lowerLiftCommandPinValue);
	
	hoodOpenSignalPin->assignData(hoodOpenSignalPinValue);
	hoodShutSignalPin->assignData(hoodShutSignalPinValue);
	liftRaisedSignalPin->assignData(liftRaisedSignalPinValue);
	liftLoweredSignalPin->assignData(liftLoweredSignalPinValue);
	hoodMotorCircuitBreakerSignalPin->assignData(hoodMotorCircuitBreakerSignalPinValue);
	liftMotorCircuitBreakerSignalPin->assignData(liftMotorCircuitBreakerSignalPinValue);
	emergencyStopClearSignalPin->assignData(emergencyStopClearSignalPinValue);
	localControlEnabledSignalPin->assignData(localControlEnabledSignalPinValue);
	
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

	addAnimatable(animatableState);
	
	auto thisMachine = std::static_pointer_cast<FlipStateMachine>(shared_from_this());
	controlWidget = std::make_shared<ControlWidget>(thisMachine, getName());
}

std::string FlipStateMachine::getStatusString(){
	
}

void FlipStateMachine::inputProcess() {
	
	//update inputs signals & state machine
	if (/*b_enabled*/ isEnabled() || areGpioSignalsReady()) {
		updateGpioInSignals();
		
		if (hoodShut && !hoodOpen) {
			if (liftLowered && !liftRaised) actualState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
			else if (liftRaised && !liftLowered) actualState = MachineState::State::UNEXPECTED_STATE;
			else if (!liftLowered && !liftRaised) actualState = MachineState::State::UNEXPECTED_STATE;
			else if (liftLowered && liftRaised) actualState = MachineState::State::UNEXPECTED_STATE;
		}
		else if (hoodOpen && !hoodShut) {
			if (liftLowered && !liftRaised) actualState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
			else if (liftRaised && !liftLowered) actualState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
			else if (!liftLowered && !liftRaised) actualState = MachineState::State::LIFT_MOVING_HOOD_OPEN;
			else if (liftLowered && liftRaised) actualState = MachineState::State::UNEXPECTED_STATE;
		}
		else if (!hoodOpen && !hoodShut) {
			if (liftLowered && !liftRaised) actualState = MachineState::State::LIFT_LOWERED_HOOD_MOVING;
			else if (liftRaised && !liftLowered) actualState = MachineState::State::UNEXPECTED_STATE;
			else if (!liftLowered && !liftRaised) actualState = MachineState::State::UNKNOWN;
			else if (liftLowered && liftRaised) actualState = MachineState::State::UNEXPECTED_STATE;
		}
		else if (hoodOpen && hoodShut) {
			actualState = MachineState::State::UNEXPECTED_STATE;
		}
	}

	//handle disabling condition
	if (/*b_enabled*/ isEnabled()) {
		if (actualState == MachineState::State::UNEXPECTED_STATE) disable();
		else if (actualState == MachineState::State::UNKNOWN) disable();
		else if (!emergencyStopClear) disable();
		else if (localControlEnabled) disable();
		else if (liftMotorCircuitBreakerTripped) disable();
		else if (hoodMotorCircuitBreakerTripped) disable();
	}
}

void FlipStateMachine::outputProcess(){
	//update outputs signals
	if (/*b_enabled*/ isEnabled()) {
		if (animatableState->hasAnimation()) {
			auto state = animatableState->getAnimationValue()->toState();
			switch (state->value->integerEquivalent) {
				case 0:
					requestedState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
					break;
				case 1:
					requestedState = MachineState::State::LIFT_LOWERED_HOOD_OPEN;
					break;
				case 2:
					requestedState = MachineState::State::LIFT_RAISED_HOOD_OPEN;
					break;
				default:
					break;
			}
		}

		switch (requestedState) {
			case MachineState::State::LIFT_LOWERED_HOOD_SHUT:
				shutLid = true;
				openLid = false;
				lowerPlatform = true;
				raisePlatform = false;
				break;
			case MachineState::State::LIFT_LOWERED_HOOD_OPEN:
				shutLid = false;
				openLid = true;
				lowerPlatform = true;
				raisePlatform = false;
				break;
			case MachineState::State::LIFT_RAISED_HOOD_OPEN:
				shutLid = false;
				openLid = true;
				lowerPlatform = false;
				raisePlatform = true;
				break;
			default:
				//in case a transition state is requested, we stop all movement
				//the only case where this is possible is when the machine was activated while being in a transition case
				//here we don't move until a selection to return to one of the fixed states is made
				shutLid = false;
				openLid = false;
				lowerPlatform = false;
				raisePlatform = false;
				break;
		}
	}
	else {
		shutLid = false;
		openLid = false;
		lowerPlatform = false;
		raisePlatform = false;
	}
	updateGpioOutSignals();
}

bool FlipStateMachine::isHardwareReady() { 
	if (!areGpioSignalsReady()) return false;
	else if (actualState == MachineState::State::UNEXPECTED_STATE) return false;
	else if (actualState == MachineState::State::UNKNOWN) return false;
	else if (localControlEnabled) return false;
	else if (hoodMotorCircuitBreakerTripped) return false;
	else if (liftMotorCircuitBreakerTripped) return false;
	else if (!emergencyStopClear) return false;
	return true;
}

void FlipStateMachine::enableHardware() {
	if (!isEnabled() && isReady()) {
		requestedState = actualState;
		state = MotionState::ENABLED;
		//b_enabled = true;
		onEnableHardware();
	}
}

void FlipStateMachine::disableHardware() {
	state = MotionState::READY;
	//b_enabled = false;
	shutLid = false;
	openLid = false;
	lowerPlatform = false;
	raisePlatform = false;
	requestedState = MachineState::State::UNKNOWN;
	updateGpioOutSignals();
	onDisableHardware();
}

bool FlipStateMachine::isGpioDeviceConnected() {
	return gpioDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> FlipStateMachine::getGpioDevice() {
	return gpioDeviceLink->getConnectedPins().front()->getSharedPointer<GpioDevice>();
}

void FlipStateMachine::updateGpioInSignals() {
	if(hoodOpenSignalPin->isConnected()) hoodOpenSignalPin->copyConnectedPinValue();
	hoodOpen = *hoodOpenSignalPinValue;
	
	if(hoodShutSignalPin->isConnected()) hoodShutSignalPin->copyConnectedPinValue();
	hoodShut = *hoodShutSignalPinValue;
	
	if(liftRaisedSignalPin->isConnected()) liftRaisedSignalPin->copyConnectedPinValue();
	liftRaised = *liftRaisedSignalPinValue;
	
	if(liftLoweredSignalPin->isConnected()) liftLoweredSignalPin->copyConnectedPinValue();
	liftLowered = *liftLoweredSignalPinValue;
	
	if(hoodMotorCircuitBreakerSignalPin->isConnected()) hoodMotorCircuitBreakerSignalPin->copyConnectedPinValue();
	hoodMotorCircuitBreakerTripped = *hoodMotorCircuitBreakerSignalPinValue;
	
	if(liftMotorCircuitBreakerSignalPin->isConnected()) liftMotorCircuitBreakerSignalPin->copyConnectedPinValue();
	liftMotorCircuitBreakerTripped = *liftMotorCircuitBreakerSignalPinValue;
	
	if(emergencyStopClearSignalPin->isConnected()) emergencyStopClearSignalPin->copyConnectedPinValue();
	emergencyStopClear = *emergencyStopClearSignalPinValue;
	
	if(localControlEnabledSignalPin->isConnected()) localControlEnabledSignalPin->copyConnectedPinValue();
	localControlEnabled = *localControlEnabledSignalPinValue;
}

void FlipStateMachine::updateGpioOutSignals() {
	/*
	openHoodCommandPin->set(openLid);
	shutHoodCommandPin->set(shutLid);
	lowerLiftCommandPin->set(lowerPlatform);
	raiseLiftCommandPin->set(raisePlatform);
	 */
	*openHoodCommandPinValue = openLid;
	*shutHoodCommandPinValue = shutLid;
	*lowerLiftCommandPinValue = lowerPlatform;
	*raiseLiftCommandPinValue = raisePlatform;
}

bool FlipStateMachine::areGpioSignalsReady() {
	//device
	if (!isGpioDeviceConnected()) return false;
	if (!getGpioDevice()->isReady()) return false;
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
	return actualState == MachineState::State::LIFT_LOWERED_HOOD_MOVING || actualState == MachineState::State::LIFT_MOVING_HOOD_OPEN;
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



void FlipStateMachine::onEnableHardware() {
	actualState = MachineState::State::UNKNOWN;
	requestedState = MachineState::State::UNKNOWN;
}

void FlipStateMachine::onDisableHardware() {
	actualState = MachineState::State::UNKNOWN;
	requestedState = MachineState::State::UNKNOWN;
}

void FlipStateMachine::simulateInputProcess() {

	//update outputs signals
	if (isEnabled()) {
		if (animatableState->hasAnimation()) {
			auto state = animatableState->getAnimationValue()->toState()->value;
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
				default:
					break;
			}
		}
		actualState = requestedState;
	}
	
}

void FlipStateMachine::simulateOutputProcess() {
	
}

bool FlipStateMachine::isSimulationReady(){
	return true;
}


void FlipStateMachine::onEnableSimulation() {
	if(actualState == MachineState::State::UNKNOWN) actualState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
	requestedState = actualState;
}

void FlipStateMachine::onDisableSimulation() {
	actualState = requestedState;
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





std::vector<FlipStateMachine::MachineState> machineStates = {
	{FlipStateMachine::MachineState::State::UNKNOWN,					-1.0,	"Unknown State"},
	{FlipStateMachine::MachineState::State::UNEXPECTED_STATE,			-2.0,	"Unexpected State"},
	{FlipStateMachine::MachineState::State::LIFT_LOWERED_HOOD_SHUT,	0.0,	"Lift Lowered, Hood Shut"},
	{FlipStateMachine::MachineState::State::LIFT_LOWERED_HOOD_MOVING,	0.5,	"Lift Lowered, Hood Moving"},
	{FlipStateMachine::MachineState::State::LIFT_LOWERED_HOOD_OPEN,	1.0,	"Lift Lowered, Hood Open"},
	{FlipStateMachine::MachineState::State::LIFT_MOVING_HOOD_OPEN,	1.5, 	"Lift Moving, Hood Open"},
	{FlipStateMachine::MachineState::State::LIFT_RAISED_HOOD_OPEN,	2.0,	"Lift Raised, Hood Open"},
};

std::vector<FlipStateMachine::MachineState>& FlipStateMachine::getStates() {
	return machineStates;
}
FlipStateMachine::MachineState* FlipStateMachine::getState(FlipStateMachine::MachineState::State s) {
	for (auto& state : machineStates) {
		if (s == state.state) return &state;
	}
	return nullptr;
}

