#include <pch.h>

#include "HoodedLiftStateMachine.h"
#include "Motion/SubDevice.h"
#include "Animation/Animatable.h"

#include "Animation/Animation.h"

std::vector<StateAnimationValue::Value> HoodedLiftStateMachine::stateParameterValues = {
	{-1, "Stopped", "Stopped"},
	{0, "Shut", "Shut"},
	{1, "Open", "Open"},
	{2, "Raised", "Raised"},
	{3, "Unknown", "Unknown"}
};

void HoodedLiftStateMachine::initialize() {
	
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

	addAnimatable(stateParameter);
}

void HoodedLiftStateMachine::inputProcess() {
	
	//update inputs signals & state machine
	if (b_enabled || areGpioSignalsReady()) {
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
	if (b_enabled) {
		if (actualState == MachineState::State::UNEXPECTED_STATE) disable();
		else if (actualState == MachineState::State::UNKNOWN) disable();
		else if (!emergencyStopClear) disable();
		else if (localControlEnabled) disable();
		else if (liftMotorCircuitBreakerTripped) disable();
		else if (hoodMotorCircuitBreakerTripped) disable();
	}

	//update outputs signals
	if (b_enabled) {

		if (stateParameter->hasAnimation()) {
			auto state = stateParameter->getAnimationValue()->toState();
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

void HoodedLiftStateMachine::outputProcess(){
	Logger::critical("Output Process not defined for hooded lift state machine");
	abort();
	//not applicable here since we drive inputs directly ?
	//or maybe we should catch blocking commands from the environnement script
}

bool HoodedLiftStateMachine::isHardwareReady() { 
	if (!areGpioSignalsReady()) return false;
	else if (actualState == MachineState::State::UNEXPECTED_STATE) return false;
	else if (actualState == MachineState::State::UNKNOWN) return false;
	else if (localControlEnabled) return false;
	else if (hoodMotorCircuitBreakerTripped) return false;
	else if (liftMotorCircuitBreakerTripped) return false;
	else if (!emergencyStopClear) return false;
	return true;
}

void HoodedLiftStateMachine::enableHardware() {
	if (!isEnabled() && isReady()) {
		requestedState = actualState;
		b_enabled = true;
		onEnableHardware();
	}
}

void HoodedLiftStateMachine::disableHardware() {
	b_enabled = false;
	shutLid = false;
	openLid = false;
	lowerPlatform = false;
	raisePlatform = false;
	requestedState = MachineState::State::UNKNOWN;
	updateGpioOutSignals();
	onDisableHardware();
}

bool HoodedLiftStateMachine::isGpioDeviceConnected() {
	return gpioDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> HoodedLiftStateMachine::getGpioDevice() {
	return gpioDeviceLink->getConnectedPins().front()->getSharedPointer<GpioDevice>();
}

void HoodedLiftStateMachine::updateGpioInSignals() {
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

void HoodedLiftStateMachine::updateGpioOutSignals() {
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

bool HoodedLiftStateMachine::areGpioSignalsReady() {
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

bool HoodedLiftStateMachine::isMoving() { 
	return actualState == MachineState::State::LIFT_LOWERED_HOOD_MOVING || actualState == MachineState::State::LIFT_MOVING_HOOD_OPEN;
}

void HoodedLiftStateMachine::rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	if (animatable == stateParameter) {
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

float HoodedLiftStateMachine::getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable) {
	if (animatable == stateParameter) {
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

bool HoodedLiftStateMachine::isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	if (animatable == stateParameter) {
		auto state = value->toState()->value;
		//TODO: this is broken
		if (state->integerEquivalent == getState(actualState)->floatEquivalent) return true;
	}
	return false;
}

void HoodedLiftStateMachine::onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) {}

void HoodedLiftStateMachine::onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) {}

void HoodedLiftStateMachine::onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) {}

std::shared_ptr<AnimationValue> HoodedLiftStateMachine::getActualAnimatableValue(std::shared_ptr<Animatable> animatable) {
	auto output = AnimationValue::makeState();
	output->value = &stateParameterValues.front();
	switch(actualState){
		case HoodedLiftStateMachine::MachineState::State::UNKNOWN:
		case HoodedLiftStateMachine::MachineState::State::UNEXPECTED_STATE: output->value = &stateParameterValues.front(); break;
		case HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_SHUT: output->value = &stateParameterValues[1]; break;
		case HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_MOVING: output->value = &stateParameterValues.front(); break;
		case HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_OPEN: output->value = &stateParameterValues[2]; break;
		case HoodedLiftStateMachine::MachineState::State::LIFT_MOVING_HOOD_OPEN: output->value = &stateParameterValues.front(); break;
		case HoodedLiftStateMachine::MachineState::State::LIFT_RAISED_HOOD_OPEN: output->value = &stateParameterValues[2]; break;
	}
	return output;
}



void HoodedLiftStateMachine::cancelAnimatableRapid(std::shared_ptr<Animatable> animatable) {
	
}

void HoodedLiftStateMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isGpioDeviceConnected()) output.push_back(getGpioDevice()->parentDevice);
}


void HoodedLiftStateMachine::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	
}

bool HoodedLiftStateMachine::validateAnimation(const std::shared_ptr<Animation> animation) {
	/*
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
	 */
	return true;
}

bool HoodedLiftStateMachine::generateTargetAnimation(std::shared_ptr<TargetAnimation> targetAnimation){
	return false;
}

void HoodedLiftStateMachine::onEnableHardware() {
	actualState = MachineState::State::UNKNOWN;
	requestedState = MachineState::State::UNKNOWN;
}

void HoodedLiftStateMachine::onDisableHardware() {
	actualState = MachineState::State::UNKNOWN;
	requestedState = MachineState::State::UNKNOWN;
}

void HoodedLiftStateMachine::simulateProcess() {

	//update outputs signals
	if (isEnabled()) {
		if (stateParameter->hasAnimation()) {
			auto state = stateParameter->getAnimationValue()->toState()->value;
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

bool HoodedLiftStateMachine::isSimulationReady(){
	return true;
}


void HoodedLiftStateMachine::onEnableSimulation() {
	if(actualState == MachineState::State::UNKNOWN) actualState = MachineState::State::LIFT_LOWERED_HOOD_SHUT;
	requestedState = actualState;
}

void HoodedLiftStateMachine::onDisableSimulation() {
	actualState = requestedState;
}



bool HoodedLiftStateMachine::loadMachine(tinyxml2::XMLElement* xml) { return true; }
bool HoodedLiftStateMachine::saveMachine(tinyxml2::XMLElement* xml) { return true; }





std::vector<HoodedLiftStateMachine::MachineState> machineStates = {
	{HoodedLiftStateMachine::MachineState::State::UNKNOWN,					-1.0,	"Unknown State"},
	{HoodedLiftStateMachine::MachineState::State::UNEXPECTED_STATE,			-2.0,	"Unexpected State"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_SHUT,	0.0,	"Lift Lowered, Hood Shut"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_MOVING,	0.5,	"Lift Lowered, Hood Moving"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_OPEN,	1.0,	"Lift Lowered, Hood Open"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_MOVING_HOOD_OPEN,	1.5, 	"Lift Moving, Hood Open"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_RAISED_HOOD_OPEN,	2.0,	"Lift Raised, Hood Open"},
};

std::vector<HoodedLiftStateMachine::MachineState>& HoodedLiftStateMachine::getStates() {
	return machineStates;
}
HoodedLiftStateMachine::MachineState* HoodedLiftStateMachine::getState(HoodedLiftStateMachine::MachineState::State s) {
	for (auto& state : machineStates) {
		if (s == state.state) return &state;
	}
	return nullptr;
}
