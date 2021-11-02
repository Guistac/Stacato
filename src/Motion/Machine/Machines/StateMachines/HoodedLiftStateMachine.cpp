#include <pch.h>

#include "HoodedLiftStateMachine.h"
#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"

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

	std::shared_ptr<Machine> thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	stateParameter = std::make_shared<AnimatableParameter>("State", thisMachine, &states);
	animatableParameters.push_back(stateParameter);

	f3Parameter = std::make_shared<AnimatableParameter>("3F", thisMachine, ParameterDataType::KINEMATIC_3D_POSITION_CURVE);
	animatableParameters.push_back(f3Parameter);
	f2Parameter = std::make_shared<AnimatableParameter>("2F", thisMachine, ParameterDataType::VECTOR_2D_PARAMETER);
	animatableParameters.push_back(f2Parameter);
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

bool HoodedLiftStateMachine::isEnabled() {
	return b_enabled;
}

bool HoodedLiftStateMachine::isReady() { 
	if (!areGpioSignalsReady()) return false;
	else if (actualState == MachineState::State::UNEXPECTED_STATE) return false;
	else if (actualState == MachineState::State::UNKNOWN) return false;
	else if (localControlEnabled) return false;
	else if (hoodMotorCircuitBreakerTripped) return false;
	else if (liftMotorCircuitBreakerTripped) return false;
	else if (!emergencyStopClear) return false;
	return true;
}

void HoodedLiftStateMachine::enable() {
	if (!isEnabled() && isReady()) {
		requestedState = actualState;
		b_enabled = true;
	}
}

void HoodedLiftStateMachine::disable() {
	b_enabled = false;
	shutLid = false;
	openLid = false;
	lowerPlatform = false;
	raisePlatform = false;
	requestedState = MachineState::State::UNKNOWN;
	updateGpioOutSignals();
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

void HoodedLiftStateMachine::moveToParameter() {}

void HoodedLiftStateMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isGpioDeviceConnected()) output.push_back(getGpioDevice()->parentDevice);
}

std::vector<HoodedLiftStateMachine::MachineState> machineStates = {
	{HoodedLiftStateMachine::MachineState::State::UNKNOWN,					"Unknown State"},
	{HoodedLiftStateMachine::MachineState::State::UNEXPECTED_STATE,			"Unexpected State"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_SHUT,	"Lift Lowered, Hood Shut"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_MOVING,	"Lift Lowered, Hood Moving"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_LOWERED_HOOD_OPEN,	"Lift Lowered, Hood Open"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_MOVING_HOOD_OPEN,	"Lift Moving, Hood Open"},
	{HoodedLiftStateMachine::MachineState::State::LIFT_RAISED_HOOD_OPEN,	"Lift Raised, Hood Open"},
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