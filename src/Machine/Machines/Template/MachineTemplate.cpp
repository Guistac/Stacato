#include <pch.h>
#include "MachineTemplate.h"

//======= CONFIGURATION =========

void MachineTemplate::initialize() {
	//add node pins	addNodePin(..);
	//add animatable parameters addAnimatableParameter(...);
}

//======= STATE CHANGES =========

bool MachineTemplate::isHardwareReady() {
	//check connected hardware and report ready status
	return false;
}

bool MachineTemplate::isSimulationReady(){
	//return true when simulation can be enabled
	return false;
}

void MachineTemplate::enableHardware() {
	//enable hardware
	//when enabled:
	//set b_enabled = true;
	//and call onEnableHardware();
}

void MachineTemplate::disableHardware() {
	//disable hardware
	//when enabled:
	//set b_enabled = false;
	//call onDisableHardware();
}

void MachineTemplate::onEnableHardware() {
	//called when hardware was enabled
}

void MachineTemplate::onDisableHardware() {
	//called when hardware was disabled
}

void MachineTemplate::onEnableSimulation() {
	//called when simulation is enabled
}

void MachineTemplate::onDisableSimulation() {
	//called when simulation is disabled
}



//======= PROCESSING =========

void MachineTemplate::inputProcess() {
	//machine inputs processing
}

void MachineTemplate::outputProcess() {
	//actuator command sending
}

void MachineTemplate::simulateInputProcess() {
	//same as inputProcess();
	//but used to simulate motion without hardware
}

void MachineTemplate::simulateOutputProcess(){
	//same as outputProcess();
	//but used to simulate motion without hardware
}

bool MachineTemplate::isMoving() {
	//true when machine is moving
	return false;
}



//========== HOMING ==========

bool MachineTemplate::canStartHoming(){ return false; }

bool MachineTemplate::isHoming(){ return false; }

void MachineTemplate::startHoming(){}

void MachineTemplate::stopHoming(){}

bool MachineTemplate::didHomingSucceed(){ return false; }

bool MachineTemplate::didHomingFail(){ return false; }

float MachineTemplate::getHomingProgress(){ return 0.0; }

const char* MachineTemplate::getHomingStateString(){ return "Homing Step String"; }



//========= ANIMATABLE OWNER ==========

void MachineTemplate::fillAnimationDefaults(std::shared_ptr<Animation> animation){}






//======= DEVICE ENUMERATION =========

void MachineTemplate::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//append all connected devices to output argument
}


//======= SAVING & LOADING =========

bool MachineTemplate::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	//save machine parameters & data
	//return true on success and false on failure
	return true;
}

bool MachineTemplate::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	//load machine parameters & data
	//return true on success and false on failure
	return true;
}
