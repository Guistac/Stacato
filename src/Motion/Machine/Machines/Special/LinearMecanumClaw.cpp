#include <pch.h>
#include "LinearMecanumClaw.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/Axis/VelocityControlledAxis.h"
#include "Motion/SubDevice.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"


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
	std::thread deviceEnabler([this](){
		auto linearAxis = getLinearAxis();
		auto clawAxis = getClawAxis();
		linearAxis->enable();
		clawAxis->enable();
		std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
		while(std::chrono::system_clock::now() - start < std::chrono::milliseconds(500)){
			if(linearAxis->isEnabled() && clawAxis->isEnabled()){
				b_enabled = true;
				onEnableHardware();
				Logger::info("Enabled Claw Machine '{}'", getName());
				return;
			}
		}
		linearAxis->disable();
		clawAxis->disable();
		Logger::warn("Could not enable Machine '{}', actuators did not enable on time", getName());
		b_enabled = false;
		return;
	});
	deviceEnabler.detach();
}

void LinearMecanumClaw::disableHardware() {
	if(isEnabled()){
		getLinearAxis()->disable();
		getClawAxis()->disable();
		b_enabled = false;
		onDisableHardware();
	}
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


struct vec2d{
	double x, y;
};

//======= PROCESSING =========

void LinearMecanumClaw::process() {

	//always update profile time
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
	
	//if the machine is not enabled, update the motion profiles by copying sensor data
	if(!isEnabled()) {
		if(isLinearAxisConnected()){
			auto linearAxis = getLinearAxis();
			linearAxisMotionProfile.setPosition(linearAxis->getActualPosition());
			linearAxisMotionProfile.setVelocity(linearAxis->getActualVelocity());
		}
		if(isClawFeedbackConnected()){
			auto clawFeedbackDevice = getClawFeedbackDevice();
			clawAxisMotionProfile.setPosition(clawFeedbackUnitsToClawUnits(clawFeedbackDevice->getPosition()));
			clawAxisMotionProfile.setVelocity(clawFeedbackUnitsToClawUnits(clawFeedbackDevice->getVelocity()));
		}
		return;
	}
	
	//get device references
	auto linearAxis = getLinearAxis();
	auto clawAxis = getClawAxis();
	auto clawFeedbackDevice = getClawFeedbackDevice();
	
	//update machine state
	*clawPosition = clawFeedbackUnitsToClawUnits(clawFeedbackDevice->getPosition());
	*clawVelocity = clawUnitsToClawFeedbackUnits(clawFeedbackDevice->getVelocity());
	*railPosition = linearAxis->getProfilePosition();
	*railVelocity = linearAxis->getProfileVelocity();
	
	//handle device state changes to disable machine
	if(isEnabled()){
		if(!linearAxis->isEnabled()) {
			disable();
			Logger::critical("Machine '{}' was disabled : Linear Axis '{}' was disabled", getName(), getLinearAxis()->getName());
		}
		if(!clawAxis->isEnabled()) {
			disable();
			Logger::critical("Machine '{}' was disabled : Claw Axis '{}' was disabled", getName(), getClawAxis()->getName());
		}
		if(!clawFeedbackDevice->isReady()) {
			disable();
			Logger::critical("Machine '{}' was disabled : Claw Feedback Devices '{}' was not ready anymore", getName(), getClawFeedbackDevice()->getName());
		}
	}
	
	//update motion profile of linear axis
	switch(linearControlMode){
		case ControlMode::VELOCITY_TARGET:
			linearAxisMotionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																		  linearAxisManualVelocityTarget,
																		  linearAxisManualAcceleration,
																		  linearAxis->getLowPositionLimit(),
																		  linearAxis->getHighPositionLimit(),
																		  linearAxis->getAccelerationLimit());
			break;
		case ControlMode::FAST_STOP:
			linearAxisMotionProfile.matchVelocity(profileDeltaTime_seconds, 0.0, linearAxis->getAccelerationLimit());
			break;
		case ControlMode::POSITION_TARGET:
			linearAxisMotionProfile.updateInterpolation(profileTime_seconds);
			break;
		case ControlMode::EXTERNAL:
			//get value from parameter track
			break;
	}
	
	//update motion profile of claw axis
	switch(clawControlMode){
		case ControlMode::VELOCITY_TARGET:
			clawAxisMotionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																		clawAxisManualVelocityTarget,
																		clawManualAcceleration,
																		0.0,
																		clawPositionLimit,
																		clawAccelerationLimit);
			break;
		case ControlMode::FAST_STOP:
			clawAxisMotionProfile.matchVelocity(profileDeltaTime_seconds, 0.0, clawAccelerationLimit);
			break;
		case ControlMode::POSITION_TARGET:
			clawAxisMotionProfile.updateInterpolation(profileTime_seconds);
			break;
		case ControlMode::EXTERNAL:
			//get value from parameter track
			break;
	}
	
	//================= MECANUM CONTROL LOGIC ==================
	//here we calculate a wheel velocity for the claw axis
	//this velocity is synchronized with the linear axis and the claw motion profile
	
	//get the magnitude of the velocity vector at the mecanum wheel
	//the velocity vector is perpendicular to the pivot radius
	double targetVelocity = clawAxisMotionProfile.getVelocity(); //degrees per second
	double rotationRadius = mecanumWheelDistanceFromClawPivot; //meters (linear axis unit)
	double rotationCirclePerimeter = 2.0 * M_PI * rotationRadius;
	double rotationVectorMagnitude = rotationCirclePerimeter * targetVelocity / 360.0; //actual linear velocity at the mecanum wheel in the direction of rotation
	
	//get the current position of the mecanum wheel relative to the pivot point
	double currentHeartAngle_degrees = *clawPosition;
	double mecanumWheelPivotAngle_degrees = currentHeartAngle_degrees + mecanumWheelClawPivotRadiusAngleWhenClosed;
	double mecanumWheelPivotAngle_radians = 2.0 * M_PI * mecanumWheelPivotAngle_degrees / 360.0;
	vec2d mecanumWheelPosition = {-std::sin(mecanumWheelPivotAngle_radians), -std::cos(mecanumWheelPivotAngle_radians)};
	mecanumWheelPosition.x *= mecanumWheelDistanceFromClawPivot;
	mecanumWheelPosition.y *= mecanumWheelDistanceFromClawPivot;
	
	//get the vector perpendicular to the pivot axis
	//this gets the direction of rotational motion at the mecanum wheel
	vec2d mecanumWheelTargetVelocityVector = {-mecanumWheelPosition.y, mecanumWheelPosition.x}; //might need to invert this

	//normalize the perpendicular vector
	double normalisationMagnitude = sqrt(std::pow(mecanumWheelTargetVelocityVector.x, 2.0) + std::pow(mecanumWheelTargetVelocityVector.y, 2.0));
	mecanumWheelTargetVelocityVector.x /= normalisationMagnitude;
	mecanumWheelTargetVelocityVector.y /= normalisationMagnitude;

	//set the rotation vector magnitude to the rotation speed
	mecanumWheelTargetVelocityVector.x *= rotationVectorMagnitude;
	mecanumWheelTargetVelocityVector.y *= rotationVectorMagnitude;
	
	//we now have the direction and magnitude of motion to be executed by the mecanum wheel
	
	//the wheel friction vector is the velocity output generated by the wheel per wheel rotation
	vec2d wheelFrictionVector = { -std::sin(mecanumWheelCircumference), std::sin(mecanumWheelCircumference) };
	
	//velocity needed to satisfy rotational motion of the heart
	double calculatedWheelVelocity = 0.0;
	calculatedWheelVelocity -= mecanumWheelTargetVelocityVector.x / wheelFrictionVector.x;
	calculatedWheelVelocity -= mecanumWheelTargetVelocityVector.y / wheelFrictionVector.y;
	
	//velocity needed to satisfy synchronisation with the linear axis
	double linearAxisVelocity = *railVelocity;
	calculatedWheelVelocity -= linearAxisVelocity / wheelFrictionVector.y;
	
	//claw closed loop position correction
	//here we measure the positional error of the claw axis and apply a weighted correction to eliminate the error
	double clawTargetPosition = clawAxisMotionProfile.getPosition();
	double clawPositionError = clawTargetPosition - *clawPosition;
	calculatedWheelVelocity += clawPositionError * clawPositionLoopProportionalGain;
	
	//disable the machine if the position error of the heart axis was exceeded
	if(std::abs(clawPositionError) > clawMaxPositionFollowingError) {
		disable();
		Logger::critical("Machine {} Disabled : Max Claw Position Following Error Was Exceeded", getName());
	}
	
	//finally, send motion commands to the actuators
	linearAxis->setMotionCommand(linearAxisMotionProfile.getPosition(), linearAxisMotionProfile.getVelocity());
	clawAxis->setVelocityCommand(calculatedWheelVelocity);
}

void LinearMecanumClaw::simulateProcess() {
	//same as process();
	//but used to simulate motion without hardware
}

bool LinearMecanumClaw::isMoving() {
	//true when machine is moving
	return false;
}







//==================================== MANUAL CONTROLS ========================================

void LinearMecanumClaw::setLinearVelocity(double velocity){
	linearControlMode = ControlMode::VELOCITY_TARGET;
	linearAxisManualVelocityTarget = velocity;
}


void LinearMecanumClaw::moveLinearToTargetInTime(double positionTarget, double timeTarget){
	auto linearAxis = getLinearAxis();
	positionTarget = std::max(positionTarget, linearAxis->getLowPositionLimit());
	positionTarget = std::min(positionTarget, linearAxis->getHighPositionLimit());
	bool success = linearAxisMotionProfile.moveToPositionInTime(profileTime_seconds,
																positionTarget,
																timeTarget,
																linearAxisManualAcceleration,
																linearAxis->getVelocityLimit());
	if(success) linearControlMode = ControlMode::POSITION_TARGET;
	else setLinearVelocity(0.0);
}

void LinearMecanumClaw::fastStopLinear(){
	linearAxisManualVelocityTarget = 0.0;
	linearControlMode = ControlMode::FAST_STOP;
}

void LinearMecanumClaw::moveLinearToTargetWithVelocity(double positionTarget, double velocityTarget){
	auto linearAxis = getLinearAxis();
	positionTarget = std::max(positionTarget, linearAxis->getLowPositionLimit());
	positionTarget = std::min(positionTarget, linearAxis->getHighPositionLimit());
	bool success = linearAxisMotionProfile.moveToPositionWithVelocity(profileTime_seconds,
																	  positionTarget,
																	  velocityTarget,
																	  linearAxisManualAcceleration);
	if(success) linearControlMode = ControlMode::POSITION_TARGET;
	else setLinearVelocity(0.0);
}


void LinearMecanumClaw::setClawVelocity(double velocity){
	clawControlMode = ControlMode::VELOCITY_TARGET;
	clawAxisManualVelocityTarget = velocity;
}

void LinearMecanumClaw::fastStopClaw(){
	clawControlMode = ControlMode::FAST_STOP;
	clawAxisManualVelocityTarget = 0.0;
}

void LinearMecanumClaw::moveClawToTargetInTime(double positionTarget, double timeTarget){
	positionTarget = std::max(positionTarget, 0.0);
	positionTarget = std::min(positionTarget, clawPositionLimit);
	bool success = clawAxisMotionProfile.moveToPositionInTime(profileTime_seconds,
															  positionTarget,
															  timeTarget,
															  clawManualAcceleration,
															  clawVelocityLimit);
	if(success) clawControlMode = ControlMode::POSITION_TARGET;
	else setClawVelocity(0.0);
}


void LinearMecanumClaw::moveClawToTargetWithVelocity(double positionTarget, double velocityTarget){
	positionTarget = std::max(positionTarget, 0.0);
	positionTarget = std::min(positionTarget, clawPositionLimit);
	bool success = clawAxisMotionProfile.moveToPositionWithVelocity(profileTime_seconds,
																	positionTarget,
																	velocityTarget,
																	clawManualAcceleration);
	if(success) clawControlMode = ControlMode::POSITION_TARGET;
	else setClawVelocity(0.0);
}




//getters for data display
double LinearMecanumClaw::getLinearAxisPosition(){
	if(!isLinearAxisConnected()) return 0.0;
	return getLinearAxis()->getActualPosition();
}
double LinearMecanumClaw::getLinearAxisVelocity(){
	if(!isLinearAxisConnected()) return 0.0;
	return getLinearAxis()->getActualVelocity();
}
double LinearMecanumClaw::getClawAxisPosition(){
	if(!isClawFeedbackConnected()) return 0.0;
	return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getPosition());
}
double LinearMecanumClaw::getClawAxisVelocity(){
	if(!isClawFeedbackConnected()) return 0.0;
	return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getVelocity());
}

float LinearMecanumClaw::getLinearAxisPositionProgress(){
	if(!isLinearAxisConnected()) return 0.0;
	auto linearAxis = getLinearAxis();
	double lowLimit = linearAxis->getLowPositionLimit();
	double highLimit = linearAxis->getHighPositionLimit();
	return linearAxis->getActualPosition() - lowLimit / (highLimit - lowLimit);
}
float LinearMecanumClaw::getLinearAxisVelocityProgress(){
	if(!isLinearAxisConnected()) return 0.0;
	auto linearAxis = getLinearAxis();
	return linearAxis->getActualVelocity() / linearAxis->getVelocityLimit();
}
float LinearMecanumClaw::getClawAxisPositionProgress(){
	if(!isClawFeedbackConnected()) return 0.0;
	return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getPosition()) / clawPositionLimit;
}
float LinearMecanumClaw::getClawAxisVelocityProgress(){
	if(!isClawFeedbackConnected()) return 0.0;
	return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getVelocity()) / clawVelocityLimit;
}


PositionUnit LinearMecanumClaw::getLinearAxisPositionUnit(){
	if(isLinearAxisConnected()) return PositionUnit::METER;
	return getLinearAxis()->getPositionUnit();
}
PositionUnit LinearMecanumClaw::getClawAxisPositionUnit(){
	return clawPositionUnit;
}


double LinearMecanumClaw::getLinearAxisMovementTargetNormalized(){
	auto linearAxis = getLinearAxis();
	double lowLimit = linearAxis->getLowPositionLimit();
	double highLimit = linearAxis->getHighPositionLimit();
	return linearAxisMotionProfile.getInterpolationTarget() - lowLimit / (highLimit - lowLimit);
}
double LinearMecanumClaw::getClawAxisMovementTargetNormalized(){
	auto clawAxis = getClawAxis();
	double lowLimit = 0.0;
	double highLimit = clawPositionLimit;
	return clawAxisMotionProfile.getInterpolationTarget() - lowLimit / (highLimit - lowLimit);
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
	clawXML->SetAttribute("ManualAcceleration", clawManualAcceleration);
	clawXML->SetAttribute("AccelerationLimit", clawAccelerationLimit);
	clawXML->SetAttribute("OpenPositionLimit", clawPositionLimit);
	clawXML->SetAttribute("PositionProportionalGain", clawPositionLoopProportionalGain);
	clawXML->SetAttribute("MaxFollowingError", clawMaxPositionFollowingError);
	
	XMLElement* linearXML = xml->InsertNewChildElement("Linear");
	linearXML->SetAttribute("ManualAcceleration", linearAxisManualAcceleration);
	
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
	if(clawXML->QueryDoubleAttribute("ManualAcceleration", &clawManualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find manual claw acceleration attribute");
	if(clawXML->QueryDoubleAttribute("OpenPositionLimit", &clawPositionLimit) != XML_SUCCESS) return Logger::warn("could not find claw open position limit attribute");
	if(clawXML->QueryDoubleAttribute("PositionProportionalGain", &clawPositionLoopProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find claw proportional gain");
	if(clawXML->QueryDoubleAttribute("MaxFollowingError", &clawMaxPositionFollowingError) != XML_SUCCESS) return Logger::warn("Could not find claw max following error");
	
	XMLElement* linearXML = xml->FirstChildElement("Linear");
	if(linearXML == nullptr) return Logger::warn("Could not find linear axis attribute");
	if(linearXML->QueryDoubleAttribute("ManualAcceleration", &linearAxisManualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find linear axis manual accceleration");
	
	return true;
}
