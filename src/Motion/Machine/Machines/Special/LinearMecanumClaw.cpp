#include <pch.h>
#include "LinearMecanumClaw.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/Axis/VelocityControlledAxis.h"
#include "Motion/SubDevice.h"

#include <tinyxml2.h>


//======= CONFIGURATION =========

void LinearMecanumClaw::initialize() {
	addNodePin(linearAxisPin);
	addNodePin(clawAxisPin);
	addNodePin(clawPositionFeedbackPin);
	addNodePin(clawReferenceDevicePin);
	addNodePin(clawClosedPin);
	addNodePin(clawOpenPin);
	
	addNodePin(railPositionPin);
	addNodePin(railVelocityPin);
	addNodePin(clawPositionPin);
	addNodePin(clawVelocityPin);
	
	addAnimatableParameter(linearAxisPositionParameter);
	addAnimatableParameter(clawAxisPositionParameter);
}

//======= STATE CHANGES =========

bool LinearMecanumClaw::isHardwareReady() {
	if(!areHardwareNodesConnected()) return false;
	if(!getLinearAxis()->isReady()) return false;
	if(!getClawAxis()->isReady()) return false;
	if(!getClawFeedbackDevice()->isReady()) return false;
	if(!getClawReferenceDevice()->isReady()) return false;
	return true;
}

bool LinearMecanumClaw::isSimulationReady(){
	if(!isLinearAxisConnected()) return false;
	if(!isClawAxisConnected()) return false;
	return true;
}

void LinearMecanumClaw::enableHardware() {
	std::thread deviceEnabler([&](){
		getLinearAxis()->enable();
		getClawAxis()->enable();
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		while (!getLinearAxis()->isEnabled() || !getClawAxis()->isEnabled()) {
			if (std::chrono::system_clock::now() - start > std::chrono::milliseconds(500)) {
				Logger::warn("Could not enable Axis '{}', actuator did not enable on time", getName());
				return;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		b_enabled = true;
		onEnableHardware();
	});
	deviceEnabler.detach();
}

void LinearMecanumClaw::disableHardware() {
	getLinearAxis()->disable();
	getClawAxis()->disable();
	b_enabled = false;
	onDisableHardware();
}

void LinearMecanumClaw::onEnableHardware() {
	//called when hardware was enabled
}

void LinearMecanumClaw::onDisableHardware() {
	//called when hardware was disabled
}

void LinearMecanumClaw::onEnableSimulation() {
	//called when simulation is enabled
}

void LinearMecanumClaw::onDisableSimulation() {
	//called when simulation is disabled
}



//======= PROCESSING =========

void LinearMecanumClaw::process() {
	//machine processing
	//actuator command sending
}

void LinearMecanumClaw::simulateProcess() {
	//same as process();
	//but used to simulate motion without hardware
}

bool LinearMecanumClaw::isMoving() {
	//true when machine is moving
	return false;
}




//======= PLOT INTERFACE =========


void LinearMecanumClaw::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	//check against all animatable parameters
	//start moving parameter to requested value
}

float LinearMecanumClaw::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//report status of rapid movement of parameter
	//0.0 = 0%
	//1.0 = 100%
}

void LinearMecanumClaw::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//stop rapid movement of specified parameter
}


bool LinearMecanumClaw::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	//check against all animatable parameters
	//report if playback of the parameter is ready to start from the given value
}

void LinearMecanumClaw::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//called when playback of that parameter starts
}

void LinearMecanumClaw::onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//called when playback of that parameter is interrupted
}

void LinearMecanumClaw::onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//called when playback of that parameter end / finishes
}

void LinearMecanumClaw::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	//check against all animatable parameters
	//write actual value of parameter to value argument
}


bool LinearMecanumClaw::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
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

bool LinearMecanumClaw::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	//check against all animatable parameters
	//return the lower and upper position limits for the specified curve, in the set of parameter curves at the given time
	//write values to lowLimit & highLimit arguments
	
	//return true if the arguments make sense and false if they don't
	
	return false;
}


void LinearMecanumClaw::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
	//check against all animatable parameters
	//generate timed motion curves to the target points and write them to the outputcurves argument
}





//======= DEVICE ENUMERATION =========

void LinearMecanumClaw::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//append all connected devices to output argument
}


//======= SAVING & LOADING =========

bool LinearMecanumClaw::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* mecanumWheelXML = xml->InsertNewChildElement("MecanumWheels");
	mecanumWheelXML->SetAttribute("DistanceFromClawPivot", mecanumWheelDistanceFromClawPivot);
	mecanumWheelXML->SetAttribute("PivotAngleWhenClosed", mecanumWheelClawPivotRadiusAngleWhenClosed);
	mecanumWheelXML->SetAttribute("WheelCircumference", mecanumWheelCircumference);
	
	XMLElement* clawXML = xml->InsertNewChildElement("Claw");
	clawXML->SetAttribute("PositionUnit", Enumerator::getSaveString(clawPositionUnit));
	clawXML->SetAttribute("FeedbackUnitsPerClawUnit", clawFeedbackUnitsPerClawUnit);
	clawXML->SetAttribute("VelocityLimit", clawVelocityLimit);
	clawXML->SetAttribute("AccelerationLimit", clawAccelerationLimit);
	clawXML->SetAttribute("OpenPositionLimit", clawPositionLimit);
	
	return true;
}

bool LinearMecanumClaw::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* mecanumWheelXML = xml->FirstChildElement("MecanumWheels");
	if(mecanumWheelXML == nullptr) return Logger::warn("Could not find mecanum wheel attribute");
	if(mecanumWheelXML->QueryDoubleAttribute("DistanceFromClawPivot", &mecanumWheelDistanceFromClawPivot) != XML_SUCCESS) return Logger::warn("could not find pivot distance attribute");
	if(mecanumWheelXML->QueryDoubleAttribute("PivotAngleWhenClosed", &mecanumWheelClawPivotRadiusAngleWhenClosed) != XML_SUCCESS) return Logger::warn("could not find anglewhenclosed attribute");
	if(mecanumWheelXML->QueryDoubleAttribute("WheelCircumference", &mecanumWheelCircumference) != XML_SUCCESS) return Logger::warn("could not find wheel circumference attribute");
	
	XMLElement* clawXML = xml->FirstChildElement("Claw");
	if(clawXML == nullptr) return Logger::warn("Could not find mecanum wheel attribute");
	const char* clawUnitString;
	if(clawXML->QueryStringAttribute("PositionUnit", &clawUnitString) != XML_SUCCESS) return Logger::warn("could not find claw position unit attribute");
	if(!Enumerator::isValidSaveName<PositionUnit>(clawUnitString)) return Logger::warn("Could not identify claw position unit attrifbute");
	clawPositionUnit = Enumerator::getEnumeratorFromSaveString<PositionUnit>(clawUnitString);
	if(clawXML->QueryDoubleAttribute("FeedbackUnitsPerClawUnit", &clawFeedbackUnitsPerClawUnit) != XML_SUCCESS) return Logger::warn("could not find claw feedback ratio attribute");
	if(clawXML->QueryDoubleAttribute("VelocityLimit", &clawVelocityLimit) != XML_SUCCESS) return Logger::warn("could not find claw velocity limit attribute");
	if(clawXML->QueryDoubleAttribute("AccelerationLimit", &clawAccelerationLimit) != XML_SUCCESS) return Logger::warn("could not find claw acceleration limit attribute");
	if(clawXML->QueryDoubleAttribute("OpenPositionLimit", &clawPositionLimit) != XML_SUCCESS) return Logger::warn("could not find claw open position limit attribute");
	
	return true;
}
