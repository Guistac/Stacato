#include <pch.h>
#include "LinearMecanumClaw.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/Axis/VelocityControlledAxis.h"
#include "Motion/SubDevice.h"

#include "Motion/Manoeuvre/ParameterTrack.h"

#include <tinyxml2.h>

#include "Fieldbus/EtherCatFieldbus.h"

#include "Environnement/Environnement.h"


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
	
	addAnimatable(linearAxisPositionParameter);
	addAnimatable(clawAxisPositionParameter);
	clawAxisPositionParameter->setUnit(clawPositionUnit);
}

void LinearMecanumClaw::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == linearAxisPin) linearAxisPositionParameter->setUnit(getLinearAxis()->getPositionUnit());
}

void LinearMecanumClaw::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == linearAxisPin) linearAxisPositionParameter->setUnit(Units::None::None);
}

void LinearMecanumClaw::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == linearAxisPin) linearAxisPositionParameter->setUnit(getLinearAxis()->getPositionUnit());
}


void LinearMecanumClaw::setClawPositionUnit(Unit unit){
	clawPositionUnit = unit;
	clawAxisPositionParameter->setUnit(unit);
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
	stopHoming();
	linearAxisMotionProfile.resetInterpolation();
	clawAxisMotionProfile.resetInterpolation();
	setLinearVelocity(0.0);
	setClawVelocity(0.0);
}

void LinearMecanumClaw::onDisableHardware() {
	stopHoming();
	linearAxisMotionProfile.resetInterpolation();
	clawAxisMotionProfile.resetInterpolation();
}

void LinearMecanumClaw::onEnableSimulation() {
	linearAxisMotionProfile.resetInterpolation();
	clawAxisMotionProfile.resetInterpolation();
	setLinearVelocity(0.0);
	setClawVelocity(0.0);
}

void LinearMecanumClaw::onDisableSimulation() {
	linearAxisMotionProfile.resetInterpolation();
	clawAxisMotionProfile.resetInterpolation();
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
	
	//update claw limit signals
	clawClosedPin->copyConnectedPinValue();
	clawOpenPin->copyConnectedPinValue();
	
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
	
	
	//TODO: zero claw angle component when closed signal is high
	
	if(isHoming()) {
		//control logic for homing routine
		homingControl();
		clawAxisMotionProfile.matchVelocity(profileDeltaTime_seconds,
											clawAxisManualVelocityTarget,
											clawManualAcceleration);
	}else{
		//control logic for manual moves
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
				if(linearAxisPositionParameter->hasAnimation()){
					auto value = linearAxisPositionParameter->getAnimationValue()->toPosition();
					linearAxisMotionProfile.setVelocity(value->velocity);
					linearAxisMotionProfile.setPosition(value->position);
				}
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
				if(clawAxisPositionParameter->hasAnimation()){
					auto value = clawAxisPositionParameter->getAnimationValue()->toPosition();
					clawAxisMotionProfile.setVelocity(value->velocity);
					clawAxisMotionProfile.setPosition(value->position);
				}
				break;
		}
	}
	
	//if the claw is definitely closed, don't allow negative motion of the claw profile
	if(*clawClosedSignal && clawAxisMotionProfile.getVelocity() < 0.0){
		clawAxisMotionProfile.setVelocity(0.0);
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
	calculatedWheelVelocity -= getLinearAxis()->getProfileVelocity() / wheelFrictionVector.y;
	
	//claw closed loop position correction
	//here we measure the positional error of the claw axis and apply a weighted correction to eliminate the error
	double clawTargetVelocity = clawAxisMotionProfile.getVelocity();
	double clawTargetPosition = clawAxisMotionProfile.getPosition();
	double clawPositionError = clawTargetPosition - *clawPosition;
	
	//force closing of the heart when these conditions are met
	if(clawTargetPosition == 0.0 && clawTargetVelocity == 0.0 && !*clawClosedSignal){
		calculatedWheelVelocity -= 0.05;
		//TODO: choose closing velocity
	}else if(!*clawClosedSignal && std::abs(clawPositionError) > clawPositionErrorThreshold){
		//correct error by applying proportional gain weighted corrections
		calculatedWheelVelocity += clawPositionError * clawPositionLoopProportionalGain;
	}

		
	//disable the machine if the position error of the heart axis was exceeded
	if(std::abs(clawPositionError) > clawMaxPositionFollowingError) {
		disable();
		Logger::critical("Machine {} Disabled : Max Claw Position Following Error Was Exceeded", getName());
	}
	
	//finally, send motion commands to the actuators
	//dont send commands to linear axis while it is homing
	if(!linearAxis->isHoming()) linearAxis->setMotionCommand(linearAxisMotionProfile.getPosition(), linearAxisMotionProfile.getVelocity());
	clawAxis->setVelocityCommand(calculatedWheelVelocity);
}

void LinearMecanumClaw::simulateProcess() {
	
	//always update profile time
	profileTime_seconds = Environnement::getTime_seconds();
	profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();
	
	//if the machine is not enabled, update the motion profiles by copying sensor data
	if(!isEnabled()) {
		*clawPosition = 0.0;
		*clawVelocity = 0.0;
		*railPosition = 0.0;
		*railVelocity = 0.0;
		return;
	}
	
	//get device references
	auto linearAxis = getLinearAxis();
	auto clawAxis = getClawAxis();
	
	//update machine state
	*clawPosition = clawAxisMotionProfile.getPosition();
	*clawVelocity = clawAxisMotionProfile.getVelocity();
	*railPosition = linearAxisMotionProfile.getPosition();
	*railVelocity = linearAxisMotionProfile.getVelocity();
	
	//control logic for manual moves
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
			if(linearAxisPositionParameter->hasAnimation()){
				auto value = linearAxisPositionParameter->getAnimationValue()->toPosition();
				linearAxisMotionProfile.setVelocity(value->velocity);
				linearAxisMotionProfile.setPosition(value->position);
			}
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
			if(clawAxisPositionParameter->hasAnimation()){
				auto value = clawAxisPositionParameter->getAnimationValue()->toPosition();
				clawAxisMotionProfile.setVelocity(value->velocity);
				clawAxisMotionProfile.setPosition(value->position);
			}
			break;
	}
}

bool LinearMecanumClaw::isMoving() {
	if(linearAxisMotionProfile.getVelocity() != 0.0) return true;
	if(clawAxisMotionProfile.getVelocity() != 0.0) return true;
	return false;
}

void LinearMecanumClaw::startHoming(){
	b_isHoming = true;
	b_homingFailed = false;
	b_homingSucceeded = false;
	homingStep = ClawHomingStep::NOT_STARTED;
}

void LinearMecanumClaw::stopHoming(){
	b_isHoming = false;
	b_homingFailed = true;
	b_homingSucceeded = false;
	homingStep = ClawHomingStep::NOT_STARTED;
	setLinearVelocity(0.0);
	setClawVelocity(0.0);
}

void LinearMecanumClaw::onHomingSuccess(){
	b_isHoming = false;
	b_homingSucceeded = true;
	b_homingFailed = false;
	homingStep = ClawHomingStep::HOMING_FINISHED;
	setLinearVelocity(0.0);
	setClawVelocity(0.0);
}

void LinearMecanumClaw::onHomingFailure(){
	b_isHoming = false;
	b_homingSucceeded = false;
	b_homingFailed = true;
	homingStep = ClawHomingStep::HOMING_FAILED;
	setLinearVelocity(0.0);
	setClawVelocity(0.0);
}



void LinearMecanumClaw::homingControl(){
	switch(homingStep){
		case ClawHomingStep::NOT_STARTED:
			setClawVelocity(-clawHomingVelocity);
			homingStep = ClawHomingStep::SEARCHING_HEART_CLOSED_LIMIT;
			break;
		case ClawHomingStep::SEARCHING_HEART_CLOSED_LIMIT:
			if(*clawClosedSignal){
				setClawVelocity(0.0);
				homingStep = ClawHomingStep::FOUND_HEART_CLOSED_LIMIT;
			}
			break;
		case ClawHomingStep::FOUND_HEART_CLOSED_LIMIT:
			if(clawAxisMotionProfile.getVelocity() == 0.0){
				auto clawFeedbackDevice = getClawFeedbackDevice();
				if(clawFeedbackDevice->canHardReset()){
					clawFeedbackDevice->resetOffset();
					clawFeedbackDevice->hardReset();
					homingStep = ClawHomingStep::RESETTING_HEART_FEEDBACK;
				}else{
					clawFeedbackDevice->setPosition(0.0);
					homingStep = ClawHomingStep::RESETTING_HEART_FEEDBACK;
				}
			}
			break;
		case ClawHomingStep::RESETTING_HEART_FEEDBACK:
			if(getClawFeedbackDevice()->canHardReset()){
				if(!getClawFeedbackDevice()->isHardResetting()){
					homingStep = ClawHomingStep::HOMING_LINEAR_AXIS;
					clawAxisMotionProfile.setPosition(clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getPosition()));
					clawAxisMotionProfile.setVelocity(0.0);
					getLinearAxis()->startHoming();
				}
			}else{
				homingStep = ClawHomingStep::HOMING_LINEAR_AXIS;
				getLinearAxis()->startHoming();
			}
			break;
		case ClawHomingStep::HOMING_LINEAR_AXIS:
			if(getLinearAxis()->didHomingSucceed()){
				homingStep = ClawHomingStep::HOMING_FINISHED;
				auto linearAxis = getLinearAxis();
				linearAxisMotionProfile.setPosition(linearAxis->getActualPosition());
				linearAxisMotionProfile.setVelocity(linearAxis->getActualVelocity());
			}else if(getLinearAxis()->didHomingFail()){
				onHomingFailure();
			}
			break;
		case ClawHomingStep::HOMING_FINISHED:
			onHomingSuccess();
			break;
		default:
			break;
	}
}


bool LinearMecanumClaw::canStartHoming(){
	return isEnabled() && !Environnement::isSimulating();
}

bool LinearMecanumClaw::isHoming(){
	return b_isHoming;
}

float LinearMecanumClaw::getHomingProgress(){
	switch(homingStep){
		case ClawHomingStep::NOT_STARTED: return 0.0;
		case ClawHomingStep::SEARCHING_HEART_CLOSED_LIMIT: return 0.1;
		case ClawHomingStep::FOUND_HEART_CLOSED_LIMIT: return 0.3;
		case ClawHomingStep::RESETTING_HEART_FEEDBACK: return 0.4;
		case ClawHomingStep::HOMING_LINEAR_AXIS: return (0.4 + (getLinearAxis()->getHomingProgress() * 0.6));
		case ClawHomingStep::HOMING_FINISHED: return 1.0;
		case ClawHomingStep::HOMING_FAILED: return 0.0;
	}
}

bool LinearMecanumClaw::didHomingSucceed(){
	return b_homingSucceeded;
}

bool LinearMecanumClaw::didHomingFail(){
	return b_homingFailed;
}

const char* LinearMecanumClaw::getHomingStateString(){
	switch(homingStep){
		case ClawHomingStep::HOMING_LINEAR_AXIS:
			return Enumerator::getDisplayString(getLinearAxis()->homingStep);
		default:
			return Enumerator::getDisplayString(homingStep);
	}
}


//==================================== MANUAL CONTROLS ========================================

void LinearMecanumClaw::setLinearVelocity(double velocity){
	linearControlMode = ControlMode::VELOCITY_TARGET;
	linearAxisManualVelocityTarget = velocity;
	linearAxisMotionProfile.resetInterpolation();
}

void LinearMecanumClaw::fastStopLinear(){
	linearAxisManualVelocityTarget = 0.0;
	linearControlMode = ControlMode::FAST_STOP;
	linearAxisMotionProfile.resetInterpolation();
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
	clawAxisMotionProfile.resetInterpolation();
}

void LinearMecanumClaw::fastStopClaw(){
	clawControlMode = ControlMode::FAST_STOP;
	clawAxisManualVelocityTarget = 0.0;
	clawAxisMotionProfile.resetInterpolation();
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


//==================================== GETTERS FOR GUI DATA ========================================

double LinearMecanumClaw::getLinearAxisPosition(){
	if(!isLinearAxisConnected()) return 0.0;
	if(isSimulating()) return linearAxisMotionProfile.getPosition();
	return getLinearAxis()->getActualPosition();
}
double LinearMecanumClaw::getLinearAxisVelocity(){
	if(!isLinearAxisConnected()) return 0.0;
	if(isSimulating()) return linearAxisMotionProfile.getVelocity();
	//return getLinearAxis()->getActualVelocity();
	return linearAxisMotionProfile.getVelocity();
}
double LinearMecanumClaw::getClawAxisPosition(){
	if(!isClawFeedbackConnected()) return 0.0;
	if(isSimulating()) return clawAxisMotionProfile.getPosition();
	return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getPosition());
}
double LinearMecanumClaw::getClawAxisVelocity(){
	if(!isClawFeedbackConnected()) return 0.0;
	if(isSimulating()) return clawAxisMotionProfile.getVelocity();
	//return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getVelocity());
	return clawAxisMotionProfile.getVelocity();
}

float LinearMecanumClaw::getLinearAxisPositionProgress(){
	if(!isLinearAxisConnected()) return 0.0;
	if(isSimulating()){
		auto linearAxis = getLinearAxis();
		double min = linearAxis->getLowPositionLimit();
		double max = linearAxis->getHighPositionLimit();
		double position = linearAxisMotionProfile.getPosition();
		return (position - min) / (max - min);
	}
	return getLinearAxis()->getActualPositionNormalized();
}
float LinearMecanumClaw::getLinearAxisVelocityProgress(){
	if(!isLinearAxisConnected()) return 0.0;
	auto linearAxis = getLinearAxis();
	double max = linearAxis->getVelocityLimit();
	if(isSimulating()) return std::abs(linearAxisMotionProfile.getVelocity()) / max;
	return linearAxisMotionProfile.getVelocity() / max;
	//return getLinearAxis()->getActualVelocityNormalized();
}
float LinearMecanumClaw::getClawAxisPositionProgress(){
	if(!isClawFeedbackConnected()) return 0.0;
	if(isSimulating()) return clawAxisMotionProfile.getPosition() / clawPositionLimit;
	return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getPosition()) / clawPositionLimit;
}
float LinearMecanumClaw::getClawAxisVelocityProgress(){
	if(!isClawFeedbackConnected()) return 0.0;
	if(isSimulating()) return std::abs(clawAxisMotionProfile.getVelocity()) / clawVelocityLimit;
	//return clawFeedbackUnitsToClawUnits(getClawFeedbackDevice()->getVelocity()) / clawVelocityLimit;
	return std::abs(clawAxisMotionProfile.getVelocity()) / clawVelocityLimit;
}
Unit LinearMecanumClaw::getLinearAxisPositionUnit(){
	if(isLinearAxisConnected()) return Units::LinearDistance::Meter;
	return getLinearAxis()->getPositionUnit();
}
Unit LinearMecanumClaw::getClawAxisPositionUnit(){
	return clawPositionUnit;
}
double LinearMecanumClaw::getLinearAxisMovementTargetNormalized(){
	auto linearAxis = getLinearAxis();
	double lowLimit = linearAxis->getLowPositionLimit();
	double highLimit = linearAxis->getHighPositionLimit();
	return (linearAxisMotionProfile.getInterpolationTarget() - lowLimit) / (highLimit - lowLimit);
}
double LinearMecanumClaw::getClawAxisMovementTargetNormalized(){
	double lowLimit = 0.0;
	double highLimit = clawPositionLimit;
	return (clawAxisMotionProfile.getInterpolationTarget() - lowLimit) / (highLimit - lowLimit);
}


double LinearMecanumClaw::getLinearAxisFollowingError(){
	if(!isLinearAxisConnected()) return 0.0;
	return getLinearAxis()->getActualFollowingError();
}

float LinearMecanumClaw::getLinearAxisFollowingErrorProgress(){
	if(!isLinearAxisConnected()) return 0.0;
	return getLinearAxis()->getActualFollowingErrorNormalized();
}














//======= PLOT INTERFACE =========


void LinearMecanumClaw::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<AnimatableParameterValue> value) {
	if(parameter == linearAxisPositionParameter){
		moveLinearToTargetWithVelocity(value->toPosition()->position, linearAxisRapidVelocity);
	}else if(parameter == clawAxisPositionParameter){
		moveClawToTargetWithVelocity(value->toPosition()->position, clawRapidVelocity);
	}
}

float LinearMecanumClaw::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	if(parameter == linearAxisPositionParameter){
		return getLinearAxisTargetMovementProgress();
	}else if(parameter == clawAxisPositionParameter){
		return getClawAxisTargetMovementProgress();
	}
}

void LinearMecanumClaw::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	if(parameter == linearAxisPositionParameter){
		setLinearVelocity(0.0);
	}else if(parameter == clawAxisPositionParameter){
		setClawVelocity(0.0);
	}
}


bool LinearMecanumClaw::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<AnimatableParameterValue> value) {
	if(parameter == linearAxisPositionParameter){
		return value->toPosition()->position == linearAxisMotionProfile.getPosition() && linearAxisMotionProfile.getVelocity() == 0.0;
	}else if(parameter == clawAxisPositionParameter){
		return value->toPosition()->position == clawAxisMotionProfile.getPosition() && clawAxisMotionProfile.getVelocity() == 0.0;
	}
}

void LinearMecanumClaw::onParameterPlaybackStart(std::shared_ptr<MachineParameter> parameter) {
	if(parameter == linearAxisPositionParameter){
		linearControlMode = ControlMode::EXTERNAL;
	}else if(parameter == clawAxisPositionParameter){
		clawControlMode = ControlMode::EXTERNAL;
	}
}

void LinearMecanumClaw::onParameterPlaybackInterrupt(std::shared_ptr<MachineParameter> parameter) {
	if(parameter == linearAxisPositionParameter){
		setLinearVelocity(0.0);
	}else if(parameter == clawAxisPositionParameter){
		setClawVelocity(0.0);
	}
}

void LinearMecanumClaw::onParameterPlaybackEnd(std::shared_ptr<MachineParameter> parameter) {
	//here we have to make sure that the last position of the manoeuvre stays in the motion profile on the next loop
	//to make sure of this we manually set the profile velocity to 0.0, and the target velocity to 0.0 to make sure nothing moves after the manoeuvre is done playing
	if(parameter == linearAxisPositionParameter){
		setLinearVelocity(0.0);
		linearAxisMotionProfile.setVelocity(0.0);
	}else if(parameter == clawAxisPositionParameter){
		setClawVelocity(0.0);
		clawAxisMotionProfile.setVelocity(0.0);
	}
}

std::shared_ptr<AnimatableParameterValue> LinearMecanumClaw::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter) {
	//check against all animatable parameters
	//write actual value of parameter to value argument
	if(parameter == linearAxisPositionParameter){
		auto output = AnimatableParameterValue::makePosition();
		output->position = linearAxisMotionProfile.getPosition();
		output->velocity = linearAxisMotionProfile.getVelocity();
		output->acceleration = linearAxisMotionProfile.getAcceleration();
		return output;
	}else if(parameter == clawAxisPositionParameter){
		auto output = AnimatableParameterValue::makePosition();
		output->position = clawAxisMotionProfile.getPosition();
		output->velocity = clawAxisMotionProfile.getVelocity();
		output->acceleration = clawAxisMotionProfile.getAcceleration();
		return output;
	}
	return nullptr;
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
	
	/*
	using namespace Motion;
	bool b_curveValid = true;
	
	if(parameterTrack->parameter == linearAxisPositionParameter){
		
		assert(parameterTrack->curves.size() == 1);
		if(!isLinearAxisConnected()) return false;

		auto linearAxis = getLinearAxis();
		double lowLimit_machineUnits = linearAxis->getLowPositionLimit();
		double highLimit_machineUnits = linearAxis->getHighPositionLimit();
		double velocityLimit_machineUnits = linearAxis->getVelocityLimit();
		double accelerationLimit_machineUnits = linearAxis->getAccelerationLimit();
		
		//get a reference to the curve we need to validate
		auto curve = parameterTrack->curves.front();

		//validate all control points
		for (auto& controlPoint : curve->points) {
			//check all validation conditions and find validaiton error state
			if (controlPoint->position < lowLimit_machineUnits || controlPoint->position > highLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			else if (std::abs(controlPoint->velocity) > velocityLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->rampIn) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->rampOut) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (controlPoint->rampIn == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO;
			else if (controlPoint->rampOut == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO;
			else controlPoint->validationError = ValidationError::NO_VALIDATION_ERROR; //All Checks Passed: No Validation Error !
			//set valid flag for point, if invalid, set flag for whole curve
			controlPoint->b_valid = controlPoint->validationError == ValidationError::NO_VALIDATION_ERROR;
			if(!controlPoint->b_valid) b_curveValid = false;
		}

		//validate all interpolations of the curve
		for (auto& interpolation : curve->interpolations) {
			//if the interpolation is already marked invalid an validation error type was already set by the interpolation engine
			//in this case we don't overwrite the validation error value
			if (!interpolation->b_valid) {
				b_curveValid = false;
				continue;
			}
			//check if the velocity of the interpolation exceeds the limit
			if (std::abs(interpolation->interpolationVelocity) > velocityLimit_machineUnits) {
				interpolation->validationError = ValidationError::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED;
				interpolation->b_valid = false;
				b_curveValid = false;
				continue;
			}
			//if all interpolation checks passed, we check all interpolation preview points for their range
			for (auto& point : interpolation->displayPoints) {
				if (point.position > highLimit_machineUnits || point.position < lowLimit_machineUnits) {
					interpolation->validationError = ValidationError::INTERPOLATION_POSITION_OUT_OF_RANGE;
					interpolation->b_valid = false;
					b_curveValid = false;
					break;
				}
			}
		}
		
		//after performing all checks, we assign the curve validation flag
		//the curve itself doesn't have a validation error value
		curve->b_valid = b_curveValid;
		
		//we return the result of the validation
		return b_curveValid;
		
	}else if(parameterTrack->parameter == clawAxisPositionParameter){
		
		assert(parameterTrack->curves.size() == 1);
		if(!isClawAxisConnected()) return false;

		auto clawAxis = getClawAxis();
		double lowLimit_machineUnits = 0.0;
		double highLimit_machineUnits = clawPositionLimit;
		double velocityLimit_machineUnits = clawAxis->getVelocityLimit();
		double accelerationLimit_machineUnits = clawAxis->getAccelerationLimit();
		
		//get a reference to the curve we need to validate
		auto curve = parameterTrack->curves.front();

		//validate all control points
		for (auto& controlPoint : curve->points) {
			//check all validation conditions and find validaiton error state
			if (controlPoint->position < lowLimit_machineUnits || controlPoint->position > highLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			else if (std::abs(controlPoint->velocity) > velocityLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->rampIn) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->rampOut) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (controlPoint->rampIn == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO;
			else if (controlPoint->rampOut == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO;
			else controlPoint->validationError = ValidationError::NO_VALIDATION_ERROR; //All Checks Passed: No Validation Error !
			//set valid flag for point, if invalid, set flag for whole curve
			controlPoint->b_valid = controlPoint->validationError == ValidationError::NO_VALIDATION_ERROR;
			if(!controlPoint->b_valid) b_curveValid = false;
		}

		//validate all interpolations of the curve
		for (auto& interpolation : curve->interpolations) {
			//if the interpolation is already marked invalid an validation error type was already set by the interpolation engine
			//in this case we don't overwrite the validation error value
			if (!interpolation->b_valid) {
				b_curveValid = false;
				continue;
			}
			//check if the velocity of the interpolation exceeds the limit
			if (std::abs(interpolation->interpolationVelocity) > velocityLimit_machineUnits) {
				interpolation->validationError = ValidationError::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED;
				interpolation->b_valid = false;
				b_curveValid = false;
				continue;
			}
			//if all interpolation checks passed, we check all interpolation preview points for their range
			for (auto& point : interpolation->displayPoints) {
				if (point.position > highLimit_machineUnits || point.position < lowLimit_machineUnits) {
					interpolation->validationError = ValidationError::INTERPOLATION_POSITION_OUT_OF_RANGE;
					interpolation->b_valid = false;
					b_curveValid = false;
					break;
				}
			}
		}
		
		//after performing all checks, we assign the curve validation flag
		//the curve itself doesn't have a validation error value
		curve->b_valid = b_curveValid;
		
		//we return the result of the validation
		return b_curveValid;
	}
	 */
	
	return false;
}

bool LinearMecanumClaw::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	//check against all animatable parameters
	//return the lower and upper position limits for the specified curve, in the set of parameter curves at the given time
	//write values to lowLimit & highLimit arguments
	
	//return true if the arguments make sense and false if they don't
	
	if(parameter == linearAxisPositionParameter){
		assert(parameterCurves.size() == 1);
		auto linearAxis = getLinearAxis();
		lowLimit = linearAxis->getLowPositionLimit();
		highLimit = linearAxis->getHighPositionLimit();
	}else if(parameter == clawAxisPositionParameter){
		assert(parameterCurves.size() == 1);
		lowLimit = 0.0;
		highLimit = clawPositionLimit;
	}
	
	return false;
}




bool LinearMecanumClaw::generateTargetParameterTrackCurves(std::shared_ptr<TargetParameterTrack> parameterTrack){
	//check against all animatable parameters
	//generate timed motion curves to the target points and write them to the outputcurves argument
	return false;
}





//======= DEVICE ENUMERATION =========

void LinearMecanumClaw::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	//append all connected devices to output argument
}




void LinearMecanumClaw::sanitizeParameters(){
	clawFeedbackUnitsPerClawUnit = std::abs(clawFeedbackUnitsPerClawUnit);
	clawVelocityLimit = std::abs(clawVelocityLimit);
	clawAccelerationLimit = std::abs(clawAccelerationLimit);
	clawPositionLimit = std::abs(clawPositionLimit);
	clawPositionLoopProportionalGain = std::abs(clawPositionLoopProportionalGain);
	clawMaxPositionFollowingError = std::abs(clawMaxPositionFollowingError);
	clawHomingVelocity = std::min(std::abs(clawHomingVelocity), clawVelocityLimit);
	
	mecanumWheelDistanceFromClawPivot = std::abs(mecanumWheelDistanceFromClawPivot);
	mecanumWheelClawPivotRadiusAngleWhenClosed = std::abs(mecanumWheelClawPivotRadiusAngleWhenClosed);
	mecanumWheelCircumference = std::abs(mecanumWheelCircumference);
	
	clawManualAcceleration = std::min(std::abs(clawManualAcceleration), clawAccelerationLimit);
	linearAxisManualAcceleration = std::abs(linearAxisManualAcceleration);
	if(isLinearAxisConnected()) linearAxisManualAcceleration = std::min(linearAxisManualAcceleration, getLinearAxis()->getAccelerationLimit());
	
	clawRapidVelocity = std::min(std::abs(clawRapidVelocity), clawRapidVelocity);
	
	linearAxisRapidVelocity = std::abs(linearAxisRapidVelocity);
	if(isLinearAxisConnected()) linearAxisRapidVelocity = std::min(linearAxisRapidVelocity, getLinearAxis()->getVelocityLimit());
}


//======= SAVING & LOADING =========

bool LinearMecanumClaw::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	
	XMLElement* mecanumWheelXML = xml->InsertNewChildElement("MecanumWheels");
	mecanumWheelXML->SetAttribute("DistanceFromClawPivot", mecanumWheelDistanceFromClawPivot);
	mecanumWheelXML->SetAttribute("PivotAngleWhenClosed", mecanumWheelClawPivotRadiusAngleWhenClosed);
	mecanumWheelXML->SetAttribute("WheelCircumference", mecanumWheelCircumference);
	
	XMLElement* clawXML = xml->InsertNewChildElement("Claw");
	clawXML->SetAttribute("PositionUnit", clawPositionUnit->saveString);
	clawXML->SetAttribute("FeedbackUnitsPerClawUnit", clawFeedbackUnitsPerClawUnit);
	clawXML->SetAttribute("VelocityLimit", clawVelocityLimit);
	clawXML->SetAttribute("ManualAcceleration", clawManualAcceleration);
	clawXML->SetAttribute("AccelerationLimit", clawAccelerationLimit);
	clawXML->SetAttribute("OpenPositionLimit", clawPositionLimit);
	clawXML->SetAttribute("HomingVelocity", clawHomingVelocity);
	clawXML->SetAttribute("PositionProportionalGain", clawPositionLoopProportionalGain);
	clawXML->SetAttribute("MaxFollowingError", clawMaxPositionFollowingError);
	clawXML->SetAttribute("PositionErrorThreshold", clawPositionErrorThreshold);
	
	XMLElement* linearXML = xml->InsertNewChildElement("Linear");
	linearXML->SetAttribute("ManualAcceleration", linearAxisManualAcceleration);
	
	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("LinearAxisVelocity", linearAxisRapidVelocity);
	rapidsXML->SetAttribute("ClawVelocity", clawRapidVelocity);
	
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
	if(!Units::AngularDistance::isValidSaveString(clawUnitString)) return Logger::warn("Could not identify claw position unit attrifbute");
	setClawPositionUnit(Units::fromSaveString(clawUnitString));
	
	if(clawXML->QueryDoubleAttribute("FeedbackUnitsPerClawUnit", &clawFeedbackUnitsPerClawUnit) != XML_SUCCESS) return Logger::warn("could not find claw feedback ratio attribute");
	if(clawXML->QueryDoubleAttribute("VelocityLimit", &clawVelocityLimit) != XML_SUCCESS) return Logger::warn("could not find claw velocity limit attribute");
	if(clawXML->QueryDoubleAttribute("AccelerationLimit", &clawAccelerationLimit) != XML_SUCCESS) return Logger::warn("could not find claw acceleration limit attribute");
	if(clawXML->QueryDoubleAttribute("ManualAcceleration", &clawManualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find manual claw acceleration attribute");
	if(clawXML->QueryDoubleAttribute("OpenPositionLimit", &clawPositionLimit) != XML_SUCCESS) return Logger::warn("could not find claw open position limit attribute");
	if(clawXML->QueryDoubleAttribute("HomingVelocity", &clawHomingVelocity) != XML_SUCCESS) return Logger::warn("Could not find claw homing velocity");
	if(clawXML->QueryDoubleAttribute("PositionProportionalGain", &clawPositionLoopProportionalGain) != XML_SUCCESS) return Logger::warn("Could not find claw proportional gain");
	if(clawXML->QueryDoubleAttribute("MaxFollowingError", &clawMaxPositionFollowingError) != XML_SUCCESS) return Logger::warn("Could not find claw max following error");
	if(clawXML->QueryDoubleAttribute("PositionErrorThreshold", &clawPositionErrorThreshold) != XML_SUCCESS) return Logger::warn("Could not find claw position error treshold");
	
	XMLElement* linearXML = xml->FirstChildElement("Linear");
	if(linearXML == nullptr) return Logger::warn("Could not find linear axis attribute");
	if(linearXML->QueryDoubleAttribute("ManualAcceleration", &linearAxisManualAcceleration) != XML_SUCCESS) return Logger::warn("Could not find linear axis manual accceleration");
	
	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if(rapidsXML == nullptr) return Logger::warn("Could not find rapids attribute");
	if(rapidsXML->QueryDoubleAttribute("LinearAxisVelocity", &linearAxisRapidVelocity) != XML_SUCCESS) return Logger::warn("Could not find linear axis rapid velocity Attribute");
	if(rapidsXML->QueryDoubleAttribute("ClawVelocity", &clawRapidVelocity) != XML_SUCCESS) return Logger::warn("Could not find claw axis rapid velocity Attribute");
	
	return true;
}

