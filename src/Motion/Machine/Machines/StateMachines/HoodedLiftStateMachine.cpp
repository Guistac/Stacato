#include <pch.h>

#include "HoodedLiftStateMachine.h"
#include "Motion/SubDevice.h"
#include "Motion/AnimatableParameter.h"

#include "Motion/Manoeuvre/ParameterTrack.h"

std::vector<StateParameterValue> HoodedLiftStateMachine::stateParameterValues = {
	{0, "Shut", "Shut"},
	{1, "Open", "Open"},
	{2, "Raised", "Raised"}
};

void HoodedLiftStateMachine::assignIoData() {
	addIoData(gpioDeviceLink);

	addIoData(hoodOpenSignalPin);
	addIoData(hoodShutSignalPin);
	addIoData(liftRaisedSignalPin);
	addIoData(liftLoweredSignalPin);
	addIoData(hoodMotorCircuitBreakerSignalPin);
	addIoData(liftMotorCircuitBreakerSignalPin);
	addIoData(emergencyStopClearSignalPin);
	addIoData(localControlEnabledSignalPin);

	addIoData(openHoodCommandPin);
	addIoData(shutHoodCommandPin);
	addIoData(raiseLiftCommandPin);
	addIoData(lowerLiftCommandPin);

	addAnimatableParameter(stateParameter);
}

void HoodedLiftStateMachine::process() {
	
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

		if (stateParameter->hasParameterTrack()) {
			AnimatableParameterValue value;
			stateParameter->getActiveTrackParameterValue(value);
			switch (value.stateValue->integerEquivalent) {
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
		onEnable();
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
	onDisable();
}

bool HoodedLiftStateMachine::isGpioDeviceConnected() {
	return gpioDeviceLink->isConnected();
}

std::shared_ptr<GpioDevice> HoodedLiftStateMachine::getGpioDevice() {
	return gpioDeviceLink->getConnectedPins().front()->getGpioDevice();
}

void HoodedLiftStateMachine::updateGpioInSignals() {
	hoodOpen = hoodOpenSignalPin->getConnectedPins().front()->getBoolean();
	hoodShut = hoodShutSignalPin->getConnectedPins().front()->getBoolean();
	liftRaised = liftRaisedSignalPin->getConnectedPins().front()->getBoolean();
	liftLowered = liftLoweredSignalPin->getConnectedPins().front()->getBoolean();
	hoodMotorCircuitBreakerTripped = hoodMotorCircuitBreakerSignalPin->getConnectedPins().front()->getBoolean();
	liftMotorCircuitBreakerTripped = liftMotorCircuitBreakerSignalPin->getConnectedPins().front()->getBoolean();
	emergencyStopClear = emergencyStopClearSignalPin->getConnectedPins().front()->getBoolean();
	localControlEnabled = localControlEnabledSignalPin->getConnectedPins().front()->getBoolean();
}

void HoodedLiftStateMachine::updateGpioOutSignals() {
	openHoodCommandPin->set(openLid);
	shutHoodCommandPin->set(shutLid);
	lowerLiftCommandPin->set(lowerPlatform);
	raiseLiftCommandPin->set(raisePlatform);
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

void HoodedLiftStateMachine::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter->dataType == value.type) {
		if (parameter == stateParameter) {
			parameterMovementStartState = actualState;
			switch (value.stateValue->integerEquivalent) {
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
}

float HoodedLiftStateMachine::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == stateParameter) {
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

bool HoodedLiftStateMachine::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter->dataType == value.type) {
		if (parameter == stateParameter) {
			if ((float)value.stateValue->integerEquivalent == getState(actualState)->floatEquivalent) return true;
		}
	}
	return false;
}

void HoodedLiftStateMachine::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {}

void HoodedLiftStateMachine::onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter) {}

void HoodedLiftStateMachine::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {}



void HoodedLiftStateMachine::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	
}

void HoodedLiftStateMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isGpioDeviceConnected()) output.push_back(getGpioDevice()->parentDevice);
}



bool HoodedLiftStateMachine::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
	parameterTrack->b_valid = true;
	for (auto& curve : parameterTrack->curves) {
		curve->b_valid = true;
		for (auto& point : curve->points) {
			point->b_valid = true;
			point->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
		}
		for (auto& interpolation : curve->interpolations) {
			interpolation->b_valid = true;
			interpolation->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
		}
	}
	return true;
}

bool HoodedLiftStateMachine::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	lowLimit = 0.0;
	highLimit = 2.0;
	return true;
}

void HoodedLiftStateMachine::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {}


void HoodedLiftStateMachine::simulateProcess() {
	//TODO: Simulate Flips
}


void HoodedLiftStateMachine::onEnable() {
}

void HoodedLiftStateMachine::onDisable() {
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
