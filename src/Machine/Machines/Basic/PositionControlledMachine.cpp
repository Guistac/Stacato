#include <pch.h>

#include "PositionControlledMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Machine/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

#include "Motion/Manoeuvre/Manoeuvre.h"

#include <tinyxml2.h>

#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"

void PositionControlledMachine::initialize() {
	//inputs
	addNodePin(positionControlledAxisPin);
		
	//outputs
	positionPin->assignData(positionPinValue);
	addNodePin(positionPin);
	velocityPin->assignData(velocityPinValue);
	addNodePin(velocityPin);

	//machine parameters
	addParameter(positionParameter);
}

void PositionControlledMachine::onPinUpdate(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) positionParameter->setUnit(getAxis()->getPositionUnit());
}

void PositionControlledMachine::onPinConnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) positionParameter->setUnit(getAxis()->getPositionUnit());
}

void PositionControlledMachine::onPinDisconnection(std::shared_ptr<NodePin> pin){
	if(pin == positionControlledAxisPin) positionParameter->setUnit(Units::None::None);
}

bool PositionControlledMachine::isHardwareReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	if (!axis->isReady()) return false;
	return true;
}

bool PositionControlledMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void PositionControlledMachine::enableHardware() {
	if (isReady()) {
		std::thread machineEnabler([this]() {
			using namespace std::chrono;
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			axis->enable();
			time_point enableRequestTime = system_clock::now();
			while (duration(system_clock::now() - enableRequestTime) < milliseconds(100)) {
				std::this_thread::sleep_for(milliseconds(10));
				if (axis->isEnabled()) {
					b_enabled = true;
					onEnableHardware();
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void PositionControlledMachine::disableHardware() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
	onDisableHardware();
}

void PositionControlledMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
}

void PositionControlledMachine::onDisableHardware() {
	Logger::info("Disabled Machine {}", getName());
}

bool PositionControlledMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledMachine::onEnableSimulation() {
	motionProfile.resetInterpolation();
	motionProfile.setVelocity(0.0);
	motionProfile.setAcceleration(0.0);
	setVelocityTarget(0.0);
}

void PositionControlledMachine::onDisableSimulation() {
	motionProfile.setVelocity(0.0);
	motionProfile.setAcceleration(0.0);
}

void PositionControlledMachine::process() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//Get Realtime values from axis (for position and velocity pins only)
	double actualPosition_machineUnits = axisPositionToMachinePosition(axis->getActualPosition());
	double actualVelocity_machineUnits = axisVelocityToMachineVelocity(axis->getActualVelocity());
	*positionPinValue = actualPosition_machineUnits;
	*velocityPinValue = actualVelocity_machineUnits;

	//Handle Axis state changes
	if (isEnabled() && !axis->isEnabled()) disable();
	
	//Abort the process if the axis is not enabled
	if (!isEnabled()) {
		//we still need to copy the current axis motion values to the machines motion profile
		//so they they are correct when we start the machine
		motionProfile.setPosition(actualPosition_machineUnits);
		motionProfile.setVelocity(actualVelocity_machineUnits);
		motionProfile.setAcceleration(0.0);
		return;
	}
	
	profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
		
	//Update Motion Profile
	switch(controlMode){
			
		case ControlMode::PARAMETER_TRACK:{
			auto value = positionParameter->getActiveParameterTrackValue()->toPosition();
			motionProfile.setPosition(value->position);
			motionProfile.setVelocity(value->velocity);
			}break;
			
		case ControlMode::VELOCITY_TARGET:{
			motionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																manualVelocityTarget_machineUnitsPerSecond,
																rapidAcceleration_machineUnitsPerSecond,
																getLowPositionLimit(),
																getHighPositionLimit(),
																axis->getAccelerationLimit());
			}break;
			
		case ControlMode::POSITION_TARGET:{
			motionProfile.updateInterpolation(profileTime_seconds);
			if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
		}break;
	}
	
	//Send motion values to axis profile
	axis->setMotionCommand(machinePositionToAxisPosition(motionProfile.getPosition()),
					 machineVelocityToAxisVelocity(motionProfile.getVelocity()));
}

void PositionControlledMachine::simulateProcess() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//update position and velocity pins
	*positionPinValue = motionProfile.getPosition();
	*velocityPinValue = motionProfile.getVelocity();

	//Update Time
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();

	//Update Motion Profile
	switch(controlMode){
			
		case ControlMode::PARAMETER_TRACK:{
			auto value = positionParameter->getActiveParameterTrackValue()->toPosition();
			motionProfile.setPosition(value->position);
			motionProfile.setVelocity(value->velocity);
		}break;
			
		case ControlMode::VELOCITY_TARGET:{
			double lowLimit_machineUnits = getLowPositionLimit();
			double highLimit_machineUnits = getHighPositionLimit();
			double accelerationLimit = axis->getAccelerationLimit();
			motionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																  manualVelocityTarget_machineUnitsPerSecond,
																  rapidAcceleration_machineUnitsPerSecond,
																  lowLimit_machineUnits,
																  highLimit_machineUnits,
																  accelerationLimit);
		}break;
			
		case ControlMode::POSITION_TARGET:{
			motionProfile.updateInterpolation(profileTime_seconds);
			if(motionProfile.isInterpolationFinished(profileTime_seconds)) setVelocityTarget(0.0);
		}break;
			
	}
	
}


bool PositionControlledMachine::isHoming(){ return false; }
void PositionControlledMachine::startHoming(){}
void PositionControlledMachine::stopHoming(){}
bool PositionControlledMachine::didHomingSucceed(){}
bool PositionControlledMachine::didHomingFail(){}
float PositionControlledMachine::getHomingProgress(){ return 0.0; }
const char* PositionControlledMachine::getHomingStateString(){ return "Work in progress."; }


bool PositionControlledMachine::isAxisConnected() {
	return positionControlledAxisPin->isConnected();
}

std::shared_ptr<PositionControlledAxis> PositionControlledMachine::getAxis() {
	return positionControlledAxisPin->getConnectedPins().front()->getSharedPointer<PositionControlledAxis>();
}



//===================== MANUAL CONTROLS =========================

void PositionControlledMachine::setVelocityTarget(double velocityTarget_machineUnitsPerSecond) {
	manualVelocityTarget_machineUnitsPerSecond = velocityTarget_machineUnitsPerSecond;
	if(controlMode == ControlMode::POSITION_TARGET) motionProfile.resetInterpolation();
	controlMode = ControlMode::VELOCITY_TARGET;
}

void PositionControlledMachine::moveToPosition(double target_machineUnits) {
	
	double lowLimit_machineUnits = getLowPositionLimit();
	double highLimit_machineUnits = getHighPositionLimit();
	target_machineUnits = std::min(target_machineUnits, highLimit_machineUnits);
	target_machineUnits = std::max(target_machineUnits, lowLimit_machineUnits);
	
	double startTime;
	if(isSimulating()) startTime = Environnement::getTime_seconds();
	else startTime = EtherCatFieldbus::getCycleProgramTime_seconds();
	
	bool success = motionProfile.moveToPositionWithVelocity(startTime,
															target_machineUnits,
															rapidVelocity_machineUnitsPerSecond,
															rapidAcceleration_machineUnitsPerSecond);
	
	double target = motionProfile.getInterpolationTarget();
	double left = motionProfile.getRemainingInterpolationTime(startTime);
	
	if(success) controlMode = ControlMode::POSITION_TARGET;
	else setVelocityTarget(0.0);
}






void PositionControlledMachine::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<AnimatableParameterValue> value) {
	if (parameter == positionParameter) {
		moveToPosition(value->toPosition()->position);
	}
}

float PositionControlledMachine::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		return motionProfile.getInterpolationProgress(Environnement::getTime_seconds());
	}
	return 0.0;
}

void PositionControlledMachine::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		setVelocityTarget(0.0);
	}
}

bool PositionControlledMachine::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, std::shared_ptr<AnimatableParameterValue> value) {
	if (parameter == positionParameter) {
		return motionProfile.getPosition() == value->toPosition()->position && motionProfile.getVelocity() == 0.0;
	}
	return false;
}

void PositionControlledMachine::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		controlMode = ControlMode::PARAMETER_TRACK;
	}
}

void PositionControlledMachine::onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter) {
	//here we just set the velocity target to 0 regardless of where we are at in the manoeuvre
	if (parameter == positionParameter) {
		setVelocityTarget(0.0);
	}
}

void PositionControlledMachine::onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter) {
	//here we have to make sure that the last position of the manoeuvre stays in the motion profile on the next loop
	//to make sure of this we manually set the profile velocity to 0.0, and the target velocity to 0.0 to make sure nothing moves after the manoeuvre is done playing
	if (parameter == positionParameter) {
		motionProfile.setVelocity(0.0);
		setVelocityTarget(0.0);
	}
}

std::shared_ptr<AnimatableParameterValue> PositionControlledMachine::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		auto output = AnimatableParameterValue::makePosition();
		output->position = motionProfile.getPosition();
		output->velocity = motionProfile.getVelocity();
		output->acceleration = motionProfile.getAcceleration();
		return output;
	}
	return nullptr;
}


bool PositionControlledMachine::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
	using namespace Motion;
	
	if (parameterTrack->getParameter() != positionParameter) {
		parameterTrack->appendValidationErrorString("Unknown Parameter");
		return false;
	}
	else if (!isAxisConnected()) {
		parameterTrack->appendValidationErrorString("No Axis Connected To Machine");
		return false;
	}
	
	bool b_trackValid = true;
	
	bool b_curveValid = true;
	
	double lowLimit_machineUnits = getLowPositionLimit();
	double highLimit_machineUnits = getHighPositionLimit();
	double velocityLimit_machineUnits = getAxis()->getVelocityLimit();
	double accelerationLimit_machineUnits = getAxis()->getAccelerationLimit();
	
	auto animatedTrack = parameterTrack->castToAnimated();
	ManoeuvreType manoeuvreType = animatedTrack->getType();
	
	//target validation
	auto targetParameter = std::dynamic_pointer_cast<BaseNumberParameter>(animatedTrack->target);
	if(!targetParameter->validateRange(lowLimit_machineUnits, highLimit_machineUnits)) {
		animatedTrack->appendValidationErrorString("Target is out of range. (" + std::to_string(lowLimit_machineUnits) + " - " + std::to_string(highLimit_machineUnits) + ")");
		b_trackValid = false;
	}
	
	if(manoeuvreType == ManoeuvreType::TARGET){
		
		//velocity validation
		auto targetTrack = parameterTrack->castToTarget();
		if(targetTrack->getConstraintType() == TargetParameterTrack::Constraint::VELOCITY){
			if(!targetTrack->velocityConstraint->validateRange(0.0, std::abs(velocityLimit_machineUnits), false, true)){
				if(targetTrack->velocityConstraint->getReal() == 0.0) animatedTrack->appendValidationErrorString("Velocity is Zero");
				else animatedTrack->appendValidationErrorString("Velocity is out of range. (max: " + std::to_string(std::abs(velocityLimit_machineUnits)) + ")");
				b_trackValid = false;
			}
		}
	}
	
	if(parameterTrack->isPlayable()){
	
		auto playableTrack = parameterTrack->castToPlayable();
		//validate acceleration
		if(!playableTrack->inAcceleration->validateRange(0.0, std::abs(accelerationLimit_machineUnits), false, true)) {
			if(playableTrack->inAcceleration->getReal() == 0.0) animatedTrack->appendValidationErrorString("In-Acceleration is Zero");
			else animatedTrack->appendValidationErrorString("In-Acceleration is out of range. (max: " + std::to_string(std::abs(accelerationLimit_machineUnits)) + ")");
			b_trackValid = false;
		}
		if(!playableTrack->outAcceleration->validateRange(0.0, std::abs(accelerationLimit_machineUnits), false, true)){
			if(playableTrack->outAcceleration->getReal() == 0.0) animatedTrack->appendValidationErrorString("Out-Acceleration is Zero");
			else animatedTrack->appendValidationErrorString("Out-Acceleration is out of range. (max: " + std::to_string(std::abs(accelerationLimit_machineUnits)) + ")");
			b_trackValid = false;
		}
	}
		
	if(manoeuvreType == ManoeuvreType::SEQUENCE){
		
		auto sequenceTrack = parameterTrack->castToSequence();
		auto startParameter = std::dynamic_pointer_cast<BaseNumberParameter>(sequenceTrack->start);
		if(!startParameter->validateRange(lowLimit_machineUnits, highLimit_machineUnits, true, true)) {
			animatedTrack->appendValidationErrorString("Start is out of range. (" + std::to_string(lowLimit_machineUnits) + " - " + std::to_string(highLimit_machineUnits) + ")");
			b_trackValid = false;
		}
		
		
		if(sequenceTrack->getCurves().size() != 1) {
			Logger::warn("Parameter Track has wrong curve count. Has {}, expected 1", sequenceTrack->getCurves().size());
			animatedTrack->appendValidationErrorString("Critical: ParameterTrack has wrong curve count.");
			return false;
		}
		
		auto& curve = sequenceTrack->getCurves().front();
		
		//validate all control points
		for (auto& controlPoint : curve.getPoints()) {
			//check all validation conditions and find validaiton error state
			if (controlPoint->position < lowLimit_machineUnits || controlPoint->position > highLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			else if (std::abs(controlPoint->velocity) > velocityLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->inAcceleration) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->outAcceleration) > accelerationLimit_machineUnits)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (controlPoint->inAcceleration == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO;
			else if (controlPoint->outAcceleration == 0.0)
				controlPoint->validationError = ValidationError::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO;
			else controlPoint->validationError = ValidationError::NONE; //All Checks Passed: No Validation Error !
			//set valid flag for point, if invalid, set flag for whole curve
			controlPoint->b_valid = controlPoint->validationError == ValidationError::NONE;
			if(!controlPoint->b_valid) b_curveValid = false;
		}

		//validate all interpolations of the curve
		for (auto& interpolation : curve.getInterpolations()) {
			
			if(interpolation->getType() != Motion::Interpolation::Type::TRAPEZOIDAL){
				Logger::warn("Sequence Track Curve Interpolation is wrong type. Is {}, expected Trapezoidal", Enumerator::getDisplayString(interpolation->getType()));
				animatedTrack->appendValidationErrorString("Critical: ParameterTrack Interpolation has wrong type.");
				return false;
			}
			auto kinematicInterpolation = interpolation->castToTrapezoidal();
			
			//if the interpolation is already marked invalid an validation error type was already set by the interpolation engine
			//in this case we don't overwrite the validation error value
			if (!interpolation->b_valid) {
				b_curveValid = false;
				continue;
			}
			//check if the velocity of the interpolation exceeds the limit
			if (std::abs(kinematicInterpolation->coastVelocity) > velocityLimit_machineUnits) {
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
		curve.b_valid = b_curveValid;
		if(!b_curveValid) animatedTrack->appendValidationErrorString("Curve could not be validated.\nCheck the Curve editor for details.");
		
	}
	
	//we return the result of the validation
	return b_trackValid;
}

bool PositionControlledMachine::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	if (parameter == positionParameter && parameterCurves.size() == 1) {
		lowLimit = getLowPositionLimit();
		highLimit = getHighPositionLimit();
		return true;
	}
	return false;
}


void PositionControlledMachine::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
	if (parameter == positionParameter && outputCurves.size() == 1 && targetPoints.size() == 1) {
	
		//TODO: this is a completely broken mess...
		
		/*
		
		std::shared_ptr<PositionControlledAxis> axis = getAxis();

		std::shared_ptr<Motion::ControlPoint> startPoint = std::make_shared<Motion::ControlPoint>();
		//MACHINE ZERO TEST
		startPoint->position =  axisPositionToMachinePosition(axis->getActualPosition());
		startPoint->velocity = axisVelocityToMachineVelocity(axis->getActualVelocity());
		startPoint->velocityOut = axisVelocityToMachineVelocity(axis->getActualVelocity());
		//startPoint->position = axis->getActualPosition_axisUnits();
		//startPoint->velocity = axis->getActualVelocity_axisUnitsPerSecond();
		//startPoint->velocityOut = axis->getActualVelocity_axisUnitsPerSecond();
		startPoint->rampOut = rampIn;

		std::shared_ptr<Motion::ControlPoint> endPoint = targetPoints.front();
		//MACHINE ZERO TEST
		endPoint->position = machinePositionToAxisPosition(endPoint->position);
		std::shared_ptr<Motion::Interpolation> timedInterpolation = std::make_shared<Motion::Interpolation>();

		Motion::TrapezoidalInterpolation::getClosestTimeAndVelocityConstrainedInterpolation(startPoint, endPoint, axis->getVelocityLimit(), timedInterpolation);
		 */
	}
	//movement from current position to the target position arriving at 0 velocity
}









void PositionControlledMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}



bool PositionControlledMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", rapidVelocity_machineUnitsPerSecond);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", rapidAcceleration_machineUnitsPerSecond);
	
	XMLElement* machineZeroXML = xml->InsertNewChildElement("MachineZero");
	machineZeroXML->SetAttribute("MachineZero_AxisUnits", machineZero_axisUnits);
	machineZeroXML->SetAttribute("InvertAxisDirection", b_invertDirection);
	
	return true;
}


bool PositionControlledMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if (rapidsXML == nullptr) return Logger::warn("Could not find Rapids attribute");
	if (rapidsXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &rapidVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid velocity attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &rapidAcceleration_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid acceleration attribute");
	
	XMLElement* machineZeroXML = xml->FirstChildElement("MachineZero");
	if(machineZeroXML == nullptr) return Logger::warn("Could not find Machine Zero Attribute");
	if(machineZeroXML->QueryDoubleAttribute("MachineZero_AxisUnits", &machineZero_axisUnits) != XML_SUCCESS) return Logger::warn("Could not find machine zero value attribute");
	if(machineZeroXML->QueryBoolAttribute("InvertAxisDirection", &b_invertDirection) != XML_SUCCESS) return Logger::warn("Could not find invert axis direction attribute");
	
	return true;
}




void PositionControlledMachine::captureMachineZero(){
	machineZero_axisUnits = motionProfile.getPosition();
}




double PositionControlledMachine::axisPositionToMachinePosition(double axisPosition){
	if(b_invertDirection) return -1.0f * (axisPosition - machineZero_axisUnits);
	return axisPosition - machineZero_axisUnits;
}

double PositionControlledMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(b_invertDirection) return axisVelocity * -1.0;
	return axisVelocity;
}

double PositionControlledMachine::machinePositionToAxisPosition(double machinePosition){
	if(b_invertDirection) return (-1.0f * machinePosition) + machineZero_axisUnits;
	return machinePosition + machineZero_axisUnits;
}

double PositionControlledMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(b_invertDirection) return machineVelocity * -1.0;
	return machineVelocity;
}


double PositionControlledMachine::getLowPositionLimit(){
	//if the machine motion is inverted relative to the axis motion, we also need to invert the limits
	if(b_invertDirection) return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
}

double PositionControlledMachine::getHighPositionLimit(){
	//if the machine motion is inverted relative to the axis motion, we also need to invert the limits
	if(b_invertDirection) return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
}

double PositionControlledMachine::getPositionNormalized(){
	double lowLimit = getLowPositionLimit();
	double highLimit = getHighPositionLimit();
	double position;
	if(isSimulating()) position = motionProfile.getPosition();
	else position = axisPositionToMachinePosition(getAxis()->getActualPosition());
	return (position - lowLimit) / (highLimit - lowLimit);
}

double PositionControlledMachine::getVelocityNormalized(){
	double velocityLimit = getAxis()->getVelocityLimit();
	double velocity;
	if(isSimulating()) velocity = motionProfile.getVelocity();
	else velocity = axisVelocityToMachineVelocity(getAxis()->getActualVelocity());
	return velocity / velocityLimit;
}


bool PositionControlledMachine::hasManualPositionTarget(){
	return controlMode == ControlMode::POSITION_TARGET;
}

double PositionControlledMachine::getManualPositionTarget(){
	return motionProfile.getInterpolationTarget();
}
