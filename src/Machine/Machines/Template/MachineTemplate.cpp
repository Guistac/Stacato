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

void MachineTemplate::process() {
	//machine processing
	//actuator command sending
}

void MachineTemplate::simulateProcess() {
	//same as process();
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




//======= PLOT INTERFACE =========


void MachineTemplate::rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	//check against all animatable parameters
	//start moving parameter to requested value
}

float MachineTemplate::getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable) {
	//check against all animatable parameters
	//report status of rapid movement of parameter
	//0.0 = 0%
	//1.0 = 100%
}

void MachineTemplate::cancelAnimatableRapid(std::shared_ptr<Animatable> animatable) {
	//check against all animatable parameters
	//stop rapid movement of specified parameter
}


bool MachineTemplate::isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) {
	//check against all animatable parameters
	//report if playback of the parameter is ready to start from the given value
}

void MachineTemplate::onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) {
	//check against all animatable parameters
	//called when playback of that parameter starts
}

void MachineTemplate::onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) {
	//check against all animatable parameters
	//called when playback of that parameter is interrupted
}

void MachineTemplate::onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) {
	//check against all animatable parameters
	//called when playback of that parameter end / finishes
}

std::shared_ptr<AnimationValue> MachineTemplate::getActualAnimatableValue(std::shared_ptr<Animatable> animatable) {
	//check against all animatable parameters
	//write actual value of parameter to value argument
}


void MachineTemplate::fillAnimationDefaults(std::shared_ptr<Animation> animation){
	
}

bool MachineTemplate::validateAnimation(const std::shared_ptr<Animation> animation) {
	//check the parameter of the given track against all animatable parameters
	//check all curves of the parameters track
	//check all control points of each curve
	//check all interpolations of each curve
	
	//set validity flag for:
	//-each curve
	//-each control point
	//-each interpolation
	
	//set validation error for:
	//-each control point
	//-each interpolation
	
	//aditionally, interpolations can have their preview points checked for range violation
	
	//interpolations may already have a validation status from previous external validation
	//in this case validity and validation errors needs to be left untouched
	
	//return overall validity
}


bool MachineTemplate::generateTargetAnimation(std::shared_ptr<TargetAnimation> targetAnimation){
	//check against all animatable parameters
	//generate timed motion curves to the target points and write them to the parameter track curves
	//return if success
	return false;
}





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
