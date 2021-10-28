#include <pch.h>

#include "HoodedLiftStateMachine.h"

void HoodedLiftStateMachine::assignIoData() {
	addIoData(gpioDeviceLink);

	addIoData(hoodShutSignalPin);
	addIoData(hoodOpenSignalPin);
	addIoData(platformLoweredSignalPin);
	addIoData(platformRaisedSignalPin);
	addIoData(emergencyStopSignalPin);
	addIoData(remoteControlEnabledSignalPin);
	addIoData(hoodMotorFuseSignalPin);
	addIoData(platformMotorFuseSignalPin);

	addIoData(shutLidCommandPin);
	addIoData(openLidCommandPin);
	addIoData(lowerPlatformCommandPin);
	addIoData(raisePlatformCommandPin);
}

void HoodedLiftStateMachine::process() {

}

bool HoodedLiftStateMachine::isEnabled() { return false; }
bool HoodedLiftStateMachine::isReady() { return false; }
void HoodedLiftStateMachine::enable() {}
void HoodedLiftStateMachine::disable() {}
bool HoodedLiftStateMachine::isMoving() { return false; }
void HoodedLiftStateMachine::moveToParameter() {}