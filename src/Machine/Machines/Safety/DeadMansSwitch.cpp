#include <pch.h>

#include "DeadMansSwitch.h"

void DeadMansSwitch::initialize(){
	addNodePin(gpioDevicePin);
	addNodePin(switchPressedPin);
	addNodePin(switchConnectedPin);
	addNodePin(switchLedPin);
	std::shared_ptr<DeadMansSwitch> thisDeadMansSwitch = std::dynamic_pointer_cast<DeadMansSwitch>(shared_from_this());
	deadMansSwitchLink->assignData(thisDeadMansSwitch);
	addNodePin(deadMansSwitchLink);
}

void DeadMansSwitch::inputProcess(){
	
	switchPressedPin->copyConnectedPinValue();
	switchConnectedPin->copyConnectedPinValue();
	
	*b_switchLed = *b_switchPressed;
}

void DeadMansSwitch::outputProcess(){
	Logger::critical("output process not defined for dead mans switch");
	abort();
}
