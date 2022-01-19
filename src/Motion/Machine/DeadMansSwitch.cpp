#include <pch.h>

#include "DeadMansSwitch.h"

void DeadMansSwitch::initialize(){
	//gpioDevicePin->assignData()
	addNodePin(gpioDevicePin);
	
	switchPressedPin->assignData(switchPressedValue);
	addNodePin(switchPressedPin);
	
	switchConnectedPin->assignData(switchConnectedValue);
	addNodePin(switchConnectedPin);
	
	switchLedPin->assignData(switchLedValue);
	addNodePin(switchLedPin);
	
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::dynamic_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->assignData(thisDeadMansSwitch);
	addNodePin(deadMansSwitchLink);
}

void DeadMansSwitch::process(){}
