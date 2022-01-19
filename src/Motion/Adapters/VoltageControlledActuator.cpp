#include <pch.h>

#include "VoltageControlledActuator.h"

void VoltageControlledActuator::initialize(){
	//output pins
	addNodePin(enablePin);
	addNodePin(controlSignalPin);
	addNodePin(actuatorPin);
	//input pins
	addNodePin(gpioDevicePin);
	addNodePin(readyPin);
	addNodePin(brakePin);
}

void VoltageControlledActuator::process(){}
