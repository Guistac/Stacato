#include <pch.h>

#include "DeadMansSwitch.h"

void DeadMansSwitch::assignIoData(){
	//gpioDevicePin->assignData()
	addIoData(gpioDevicePin);
	
	switchPressedPin->assignData(switchPressedValue);
	addIoData(switchPressedPin);
	
	switchConnectedPin->assignData(switchConnectedValue);
	addIoData(switchConnectedPin);
	
	switchLedPin->assignData(switchLedValue);
	addIoData(switchLedPin);
	
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::dynamic_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->assignData(thisDeadMansSwitch);
	addIoData(deadMansSwitchLink);
}

void DeadMansSwitch::process(){}
