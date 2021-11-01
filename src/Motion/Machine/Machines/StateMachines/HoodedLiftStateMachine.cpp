#include <pch.h>

#include "HoodedLiftStateMachine.h"
#include "Motion/Subdevice.h"
#include "Motion/Machine/AnimatableParameter.h"

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
	targetStateParameter = std::make_shared<AnimatableParameter>("State", thisMachine, &states);
	animatableParameters.push_back(targetStateParameter);
	targetIntParameter = std::make_shared<AnimatableParameter>("Int Test", thisMachine, ParameterDataType::INTEGER_PARAMETER);
	animatableParameters.push_back(targetIntParameter);
	dummyRealParameter = std::make_shared<AnimatableParameter>("Real Test", thisMachine, ParameterDataType::REAL_PARAMETER);
	animatableParameters.push_back(dummyRealParameter);
	dummyVec2Parameter = std::make_shared<AnimatableParameter>("Vec2 Test", thisMachine, ParameterDataType::VECTOR_2D_PARAMETER);
	animatableParameters.push_back(dummyVec2Parameter);
	dummyVec3Parameter = std::make_shared<AnimatableParameter>("Vec3 Test", thisMachine, ParameterDataType::VECTOR_3D_PARAMETER);
	animatableParameters.push_back(dummyVec3Parameter);
}

void HoodedLiftStateMachine::process() {
	
	if (isGpioDeviceConnected()) updateGpioInSignals();
	else {
		liftLowered = false;
		liftRaised = false;
		hoodShut = false;
		hoodOpen = false;
	}

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

	if (b_enabled) {
		if (actualState == MachineState::State::UNEXPECTED_STATE) disable();
		else if (actualState == MachineState::State::UNKNOWN) disable();
	}

	if (b_enabled && actualState != requestedState) {
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
				//this condition should never be hit, since the only requested states are the ones above
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
	if (!isGpioDeviceConnected()) return false;
	if (!getGpioDevice()->isReady()) return false;
}

void HoodedLiftStateMachine::enable() {
	if (isReady()) {
		requestedState = MachineState::State::UNKNOWN;
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