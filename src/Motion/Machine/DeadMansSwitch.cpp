#include <pch.h>

#include "DeadMansSwitch.h"

void DeadMansSwitch::assignIoData(){
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::dynamic_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->set(thisDeadMansSwitch);
	addIoData(gpioDevicePin);
	addIoData(switchPressedPin);
	addIoData(switchConnectedPin);
	addIoData(switchLedPin);
	addIoData(deadMansSwitchLink);
}

void DeadMansSwitch::process(){}
