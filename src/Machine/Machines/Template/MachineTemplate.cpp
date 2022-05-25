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


void MachineTemplate::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<AnimatableParameterValue> value) {
	//check against all animatable parameters
	//start moving parameter to requested value
}

float MachineTemplate::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//report status of rapid movement of parameter
	//0.0 = 0%
	//1.0 = 100%
}

void MachineTemplate::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//stop rapid movement of specified parameter
}


bool MachineTemplate::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<AnimatableParameterValue> value) {
	//check against all animatable parameters
	//report if playback of the parameter is ready to start from the given value
}

void MachineTemplate::onParameterPlaybackStart(std::shared_ptr<MachineParameter> parameter) {
	//check against all animatable parameters
	//called when playback of that parameter starts
}

void MachineTemplate::onParameterPlaybackInterrupt(std::shared_ptr<MachineParameter> parameter) {
	//check against all animatable parameters
	//called when playback of that parameter is interrupted
}

void MachineTemplate::onParameterPlaybackEnd(std::shared_ptr<MachineParameter> parameter) {
	//check against all animatable parameters
	//called when playback of that parameter end / finishes
}

std::shared_ptr<AnimatableParameterValue> MachineTemplate::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//write actual value of parameter to value argument
}


bool MachineTemplate::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
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

bool MachineTemplate::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	//check against all animatable parameters
	//return the lower and upper position limits for the specified curve, in the set of parameter curves at the given time
	//write values to lowLimit & highLimit arguments
	
	//return true if the arguments make sense and false if they don't
	
	return false;
}

bool MachineTemplate::generateTargetParameterTrackCurves(std::shared_ptr<TargetParameterTrack> parameterTrack){
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
