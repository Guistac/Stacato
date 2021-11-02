#include <pch.h>

#include "HoodedLiftStateMachine.h"
#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"

void HoodedLiftStateMachine::assignIoData() {
	addIoData(gpioDeviceLink);

	addIoData(hoodShutSignalPin);
	addIoData(hoodOpenSignalPin);
	addIoData(liftLoweredSignalPin);
	addIoData(liftRaisedSignalPin);
	addIoData(emergencyStopSignalPin);
	addIoData(remoteControlEnabledSignalPin);
	addIoData(hoodMotorFuseSignalPin);
	addIoData(liftMotorFuseSignalPin);

	addIoData(shutLidCommandPin);
	addIoData(openLidCommandPin);
	addIoData(lowerPlatformCommandPin);
	addIoData(raisePlatformCommandPin);

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
		else if (emergencyStopActive) disable();
		else if (!remoteControlEnabled) disable();
		else if (liftMotorFuseBurned) disable();
		else if (hoodMotorFuseBurned) disable();
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
	else if (!remoteControlEnabled) return false;
	else if (hoodMotorFuseBurned) return false;
	else if (liftMotorFuseBurned) return false;
	else if (emergencyStopActive) return false;
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
	hoodShut = hoodShutSignalPin->getConnectedPins().front()->getBoolean();
	hoodOpen = hoodOpenSignalPin->getConnectedPins().front()->getBoolean();
	liftLowered = liftLoweredSignalPin->getConnectedPins().front()->getBoolean();
	liftRaised = liftRaisedSignalPin->getConnectedPins().front()->getBoolean();
	emergencyStopActive = emergencyStopSignalPin->getConnectedPins().front()->getBoolean();
	remoteControlEnabled = remoteControlEnabledSignalPin->getConnectedPins().front()->getBoolean();
	hoodMotorFuseBurned = hoodMotorFuseSignalPin->getConnectedPins().front()->getBoolean();
	liftMotorFuseBurned = liftMotorFuseSignalPin->getConnectedPins().front()->getBoolean();
}

void HoodedLiftStateMachine::updateGpioOutSignals() {
	raisePlatformCommandPin->set(raisePlatform);
	lowerPlatformCommandPin->set(lowerPlatform);
	shutLidCommandPin->set(shutLid);
	openLidCommandPin->set(openLid);
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
	if (!emergencyStopSignalPin->isConnected()) return false;
	if (!remoteControlEnabledSignalPin->isConnected()) return false;
	if (!hoodMotorFuseSignalPin->isConnected()) return false;
	if (!liftMotorFuseSignalPin->isConnected()) return false;
	//outputs
	if (!raisePlatformCommandPin->isConnected()) return false;
	if (!lowerPlatformCommandPin->isConnected()) return false;
	if (!shutLidCommandPin->isConnected()) return false;
	if (!openLidCommandPin->isConnected()) return false;
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