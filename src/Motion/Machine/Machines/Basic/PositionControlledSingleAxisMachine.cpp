#include <pch.h>

#include "PositionControlledSingleAxisMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

#include "Motion/Manoeuvre/Manoeuvre.h"

#include <tinyxml2.h>

#include "Project/Environnement.h"

void PositionControlledSingleAxisMachine::assignIoData() {
	addIoData(positionControlledAxisPin);
	addIoData(positionPin);
	addIoData(velocityPin);

	addAnimatableParameter(positionParameter);
}

bool PositionControlledSingleAxisMachine::isHardwareReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	if (!axis->isReady()) return false;
	return true;
}

bool PositionControlledSingleAxisMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void PositionControlledSingleAxisMachine::enableHardware() {
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

void PositionControlledSingleAxisMachine::disableHardware() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
	onDisableHardware();
}

void PositionControlledSingleAxisMachine::onEnableHardware() {
	Logger::info("Enabled Machine {}", getName());
}

void PositionControlledSingleAxisMachine::onDisableHardware() {
	Logger::info("Disabled Machine {}", getName());
}

bool PositionControlledSingleAxisMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledSingleAxisMachine::onEnableSimulation() {
	simulationTargetInterpolation->resetValues();
	simulationMotionProfile.setVelocity(0.0);
	simulationMotionProfile.setAcceleration(0.0);
	setVelocityTarget(0.0);
}

void PositionControlledSingleAxisMachine::onDisableSimulation() {
	simulationMotionProfile.setVelocity(0.0);
	simulationMotionProfile.setAcceleration(0.0);
}

void PositionControlledSingleAxisMachine::process() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//Get Realtime values from axis
	
	//MACHINE ZERO TEST
	double actualPosition_machineUnits = axisPositionToMachinePosition(axis->getActualPosition_axisUnits());
	double actualVelocity_machineUnits = axisVelocityToMachineVelocity(axis->getActualVelocity_axisUnitsPerSecond());
	//double actualPosition_machineUnits = axis->getActualPosition_axisUnits();
	//double actualVelocity_machineUnits = axis->getActualVelocity_axisUnitsPerSecond();
	
	positionPin->set(actualPosition_machineUnits);
	velocityPin->set(actualVelocity_machineUnits);

	//Update Time
	double profileTime_seconds = axis->profileTime_seconds;
	double profileDeltaTime_seconds = axis->profileTimeDelta_seconds;

	//Handle state changes
	if (b_enabled && !axis->isEnabled()) disable();

	//Handle parameter track playback
	if (positionParameter->hasParameterTrack()) {
		double previousProfilePosition_machineUnits = axis->getProfilePosition_axisUnits();
		AnimatableParameterValue playbackPosition;
		positionParameter->getActiveTrackParameterValue(playbackPosition);
		
		//MACHINE ZERO TEST
		axis->profilePosition_axisUnits = machinePositionToAxisPosition(playbackPosition.realValue);
		axis->profileVelocity_axisUnitsPerSecond = machineVelocityToAxisVelocity((playbackPosition.realValue - previousProfilePosition_machineUnits) / profileDeltaTime_seconds);
		//axis->profilePosition_axisUnits = playbackPosition.realValue;
		//axis->profileVelocity_axisUnitsPerSecond = (playbackPosition.realValue - previousProfilePosition_machineUnits) / profileDeltaTime_seconds;
		
		axis->sendActuatorCommands();
	}
}

void PositionControlledSingleAxisMachine::simulateProcess() {
	
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	positionPin->set(simulationMotionProfile.getPosition());
	velocityPin->set(simulationMotionProfile.getVelocity());

	//Update Time
	double profileTime_seconds = Environnement::getTime_seconds();
	double profileDeltaTime_seconds = Environnement::getDeltaTime_seconds();

	//Handle parameter track playback
	if (positionParameter->hasParameterTrack()) {
		
		double previousProfilePosition_machineUnits = simulationMotionProfile.getPosition();
		AnimatableParameterValue playbackPosition;
		positionParameter->getActiveTrackParameterValue(playbackPosition);
		simulationMotionProfile.setPosition(playbackPosition.realValue);
		simulationMotionProfile.setVelocity((playbackPosition.realValue - previousProfilePosition_machineUnits) / profileDeltaTime_seconds);
		
	}else if(controlMode == SimulationControlMode::VELOCITY_TARGET){
		
		//MACHINE ZERO TEST
		double lowLimit_machineUnits = getLowPositionLimit();
		double highLimit_machineUnits = getHighPositionLimit();
		double accelerationLimit = getAxis()->getAccelerationLimit_axisUnitsPerSecondSquared();
		
		simulationMotionProfile.matchVelocityAndRespectPositionLimits(profileDeltaTime_seconds,
																	  manualVelocityTarget_machineUnitsPerSecond,
																	  rapidAcceleration_machineUnitsPerSecond,
																	  lowLimit_machineUnits,
																	  highLimit_machineUnits,
																	  accelerationLimit);
		
	}else if(controlMode == SimulationControlMode::POSITION_TARGET){
		
		if (simulationTargetInterpolation->isTimeInside(profileTime_seconds)) {
			Motion::CurvePoint curvePoint = simulationTargetInterpolation->getPointAtTime(profileTime_seconds);
			simulationMotionProfile.setPosition(curvePoint.position);
			simulationMotionProfile.setVelocity(curvePoint.velocity);
			simulationMotionProfile.setAcceleration(curvePoint.acceleration);
		}
		else if (simulationTargetInterpolation->getProgressAtTime(profileTime_seconds) >= 1.0) {
			simulationMotionProfile.setPosition(simulationTargetInterpolation->outPosition);
			simulationMotionProfile.setVelocity(0.0);
			simulationMotionProfile.setAcceleration(0.0);
			setVelocityTarget(0.0);
		}else{
			simulationMotionProfile.setVelocity(0.0);
			simulationMotionProfile.setAcceleration(0.0);
			setVelocityTarget(0.0);
		}
		
	}
	
}






bool PositionControlledSingleAxisMachine::isAxisConnected() {
	return positionControlledAxisPin->isConnected();
}

std::shared_ptr<PositionControlledAxis> PositionControlledSingleAxisMachine::getAxis() {
	return positionControlledAxisPin->getConnectedPins().front()->getPositionControlledAxis();
}



//===================== MANUAL CONTROLS =========================

void PositionControlledSingleAxisMachine::setVelocityTarget(double velocityTarget_machineUnitsPerSecond) {
	manualVelocityTarget_machineUnitsPerSecond = velocityTarget_machineUnitsPerSecond;
	if(!isSimulating()) {
		//MACHINE ZERO TEST
		double velocityTarget_axisUnits = machineVelocityToAxisVelocity(velocityTarget_machineUnitsPerSecond);
		getAxis()->setVelocityTarget(velocityTarget_axisUnits);
		//getAxis()->setVelocityTarget(velocityTarget_machineUnitsPerSecond);
	}
	else{
		//SIMULATION TEST
		controlMode = SimulationControlMode::VELOCITY_TARGET;
		simulationTargetInterpolation->resetValues();
	}
}

void PositionControlledSingleAxisMachine::moveToPosition(double target_machineUnits) {
	if(!isSimulating()) {
		
		//MACHINE ZERO TEST
		double target_axisUnits = machinePositionToAxisPosition(target_machineUnits);
		getAxis()->moveToPositionWithVelocity(target_axisUnits, rapidVelocity_machineUnitsPerSecond, rapidAcceleration_machineUnitsPerSecond);
		//getAxis()->moveToPositionWithVelocity(target_machineUnits, rapidVelocity_machineUnitsPerSecond, rapidAcceleration_machineUnitsPerSecond);
	}
	else{
		//SIMULATION TEST
		//MACHINE ZERO TEST
		double highLimit_machineUnits = getHighPositionLimit();
		double lowLimit_machineUnits = getLowPositionLimit();
		
		double simulationTime = Environnement::getTime_seconds();
		double profilePosition = simulationMotionProfile.getPosition();
		double profileAcceleration = simulationMotionProfile.getAcceleration();
		double profileVelocity = simulationMotionProfile.getVelocity();
		
		if (target_machineUnits > highLimit_machineUnits) target_machineUnits = highLimit_machineUnits;
		else if (target_machineUnits < lowLimit_machineUnits) target_machineUnits = lowLimit_machineUnits;
		
		auto startPoint = std::make_shared<Motion::ControlPoint>(simulationTime, profilePosition, rapidAcceleration_machineUnitsPerSecond, profileVelocity);
		auto endPoint = std::make_shared<Motion::ControlPoint>(0.0, target_machineUnits, rapidAcceleration_machineUnitsPerSecond, 0.0);
		
		if (Motion::TrapezoidalInterpolation::getFastestVelocityConstrainedInterpolation(startPoint, endPoint, rapidVelocity_machineUnitsPerSecond, simulationTargetInterpolation)) {
			controlMode = SimulationControlMode::POSITION_TARGET;
		}
		else {
			setVelocityTarget(0.0);
		}
	}
}






void PositionControlledSingleAxisMachine::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter->dataType == value.type) {
		if (parameter == positionParameter) {
			if (!isAxisConnected()) return;
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			moveToPosition(value.realValue);
		}
	}
}

float PositionControlledSingleAxisMachine::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		if (!isAxisConnected()) return 0.0;
		if(!isSimulating()){
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			return axis->targetInterpolation->getProgressAtTime(axis->profileTime_seconds);
		}else{
			//SIMULATION TEST
			return simulationTargetInterpolation->getProgressAtTime(Environnement::getTime_seconds());
		}
	}
	return 0.0;
}

bool PositionControlledSingleAxisMachine::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter->dataType == value.type) {
		if (parameter == positionParameter) {
			if (!isAxisConnected()) return false;
			if(!isSimulating()){
				std::shared_ptr<PositionControlledAxis> axis = getAxis();
				//MACHINE ZERO TEST
				double profilePosition_machineUnits = axisPositionToMachinePosition(axis->getProfilePosition_axisUnits());
				return profilePosition_machineUnits == value.realValue && axis->getProfileVelocity_axisUnitsPerSecond() == 0.0;
				//return axis->getProfilePosition_axisUnits() == value.realValue && axis->getProfileVelocity_axisUnitsPerSecond() == 0.0;
			}else{
				//SIMULATION TEST
				return simulationMotionProfile.getPosition() == value.realValue && simulationMotionProfile.getVelocity() == 0.0;
			}
		}
	}
	return false;
}

void PositionControlledSingleAxisMachine::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		if(!isSimulating()) getAxis()->controlMode = ControlMode::Mode::MACHINE_CONTROL;
		else {
			
			//SIMULATION TEST
			controlMode = SimulationControlMode::PLOT;
		}
	}
}

void PositionControlledSingleAxisMachine::onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter) {
	//here we just set the velocity target to 0 regardless of where we are at in the manoeuvre
	if (parameter == positionParameter) {
		if(!isSimulating()) setVelocityTarget(0.0);
		else{
			//SIMULATION TEST
			setVelocityTarget(0.0);
		}
	}
}

void PositionControlledSingleAxisMachine::onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter) {
	//here we have to make sure that the last position of the manoeuvre stays in the motion profile on the next loop
	//to make sure of this we manually set the profile velocity to 0.0, and the target velocity to 0.0 to make sure nothing moves after the manoeuvre is done playing
	if (parameter == positionParameter) {
		if(!isSimulating()) {
			getAxis()->profileVelocity_axisUnitsPerSecond = 0.0;
			setVelocityTarget(0.0);
		}
		else{
			Logger::critical("END PARAMETER");
			simulationMotionProfile.setVelocity(0.0);
			//SIMULATION TEST
			setVelocityTarget(0.0);
		}
	}
}

void PositionControlledSingleAxisMachine::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter == positionParameter) {
		if(!isSimulating()) {
			//MACHINE ZERO TEST
			double actualPosition_machineUnits = axisPositionToMachinePosition(getAxis()->getActualPosition_axisUnits());
			//value.realValue = getAxis()->getActualPosition_axisUnits();
			value.realValue = actualPosition_machineUnits;
		}
		else{
			//SIMULATION TEST
			value.realValue = simulationMotionProfile.getPosition();
		}
	}
}






void PositionControlledSingleAxisMachine::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		if(!isSimulating()){
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			axis->setVelocityTarget(0.0);
		}else{
			//SIMULATION TEST
			setVelocityTarget(0.0);
		}
	}
}

bool PositionControlledSingleAxisMachine::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
	bool b_curveValid = true;

	if (!isAxisConnected()) return false;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	
	//MACHINE ZERO TEST
	double lowLimit_machineUnits = getLowPositionLimit();
	double highLimit_machineUnits = getHighPositionLimit();

	if (parameterTrack->parameter == positionParameter && parameterTrack->curves.size() == 1) {

		using namespace Motion;
		std::shared_ptr<Curve> curve = parameterTrack->curves.front();

		for (auto& controlPoint : curve->points) {
			controlPoint->validationError = ValidationError::Error::NO_VALIDATION_ERROR;

			//MACHINE ZERO TEST
			if (controlPoint->position < lowLimit_machineUnits || controlPoint->position > highLimit_machineUnits)
				controlPoint->validationError = ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			else if (std::abs(controlPoint->velocity) > axis->getVelocityLimit_axisUnitsPerSecond())
				controlPoint->validationError = ValidationError::Error::CONTROL_POINT_VELOCITY_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->rampIn) > axis->getAccelerationLimit_axisUnitsPerSecondSquared())
				controlPoint->validationError = ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (std::abs(controlPoint->rampOut) > axis->getAccelerationLimit_axisUnitsPerSecondSquared())
				controlPoint->validationError = ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_LIMIT_EXCEEDED;
			else if (controlPoint->rampIn == 0.0)
				controlPoint->validationError = ValidationError::Error::CONTROL_POINT_INPUT_ACCELERATION_IS_ZERO;
			else if (controlPoint->rampOut == 0.0)
				controlPoint->validationError = ValidationError::Error::CONTROL_POINT_OUTPUT_ACCELERATION_IS_ZERO;

			if (controlPoint->validationError == ValidationError::Error::NO_VALIDATION_ERROR)
				controlPoint->b_valid = true;
			else {
				controlPoint->b_valid = false;
				b_curveValid = false;
			}

		}

		for (auto& interpolation : curve->interpolations) {
			if (!interpolation->b_valid) {
				//if the interpolation is already marked invalid
				//no solution was found to begin with
				//an validation error type was already set by the interpolation engine
				b_curveValid = false;
				continue;
			}
			else if (std::abs(interpolation->interpolationVelocity) > axis->getVelocityLimit_axisUnitsPerSecond()) {
				interpolation->validationError = ValidationError::Error::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED;
				interpolation->b_valid = false;
				b_curveValid = false;
				continue;
			}
			for (auto& point : interpolation->displayPoints) {
				//MACHINE ZERO TEST
				if (point.position > highLimit_machineUnits || point.position < lowLimit_machineUnits) {
					interpolation->validationError = ValidationError::Error::INTERPOLATION_POSITION_OUT_OF_RANGE;
					interpolation->b_valid = false;
					b_curveValid = false;
					break;
				}
			}
		}
		curve->b_valid = b_curveValid;
	}

	return b_curveValid;
}

bool PositionControlledSingleAxisMachine::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	if (parameter == positionParameter && parameterCurves.size() == 1) {
		std::shared_ptr<PositionControlledAxis> axis = getAxis();
		//MACHINE ZERO TEST
		lowLimit = getLowPositionLimit();
		highLimit = getHighPositionLimit();
		//lowLimit = axis->getLowPositionLimit();
		//highLimit = axis->getHighPositionLimit();
		return true;
	}
	return false;
}


void PositionControlledSingleAxisMachine::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
	if (parameter == positionParameter && outputCurves.size() == 1 && targetPoints.size() == 1) {
	
		std::shared_ptr<PositionControlledAxis> axis = getAxis();

		std::shared_ptr<Motion::ControlPoint> startPoint = std::make_shared<Motion::ControlPoint>();
		//MACHINE ZERO TEST
		startPoint->position =  axisPositionToMachinePosition(axis->getActualPosition_axisUnits());
		startPoint->velocity = axisVelocityToMachineVelocity(axis->getActualVelocity_axisUnitsPerSecond());
		startPoint->velocityOut = axisVelocityToMachineVelocity(axis->getActualVelocity_axisUnitsPerSecond());
		//startPoint->position = axis->getActualPosition_axisUnits();
		//startPoint->velocity = axis->getActualVelocity_axisUnitsPerSecond();
		//startPoint->velocityOut = axis->getActualVelocity_axisUnitsPerSecond();
		startPoint->rampOut = rampIn;

		std::shared_ptr<Motion::ControlPoint> endPoint = targetPoints.front();
		//MACHINE ZERO TEST
		endPoint->position = machinePositionToAxisPosition(endPoint->position);
		std::shared_ptr<Motion::Interpolation> timedInterpolation = std::make_shared<Motion::Interpolation>();

		Motion::TrapezoidalInterpolation::getClosestTimeAndVelocityConstrainedInterpolation(startPoint, endPoint, axis->getVelocityLimit_axisUnitsPerSecond(), timedInterpolation);
	}
	//movement from current position to the target position arriving at 0 velocity
}









void PositionControlledSingleAxisMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}



bool PositionControlledSingleAxisMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", rapidVelocity_machineUnitsPerSecond);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", rapidAcceleration_machineUnitsPerSecond);
	
	XMLElement* machineZeroXML = xml->InsertNewChildElement("MachineZero");
	machineZeroXML->SetAttribute("MachineZero_AxisUnits", machineZero_axisUnits);
	machineZeroXML->SetAttribute("InvertAxisDirection", b_invertDirection);
	
	return true;
}


bool PositionControlledSingleAxisMachine::loadMachine(tinyxml2::XMLElement* xml) {
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




void PositionControlledSingleAxisMachine::captureMachineZero(){
	if(!isSimulating()){
		machineZero_axisUnits = getAxis()->getActualPosition_axisUnits();
	}else{
		//SIMULATION TEST
		machineZero_axisUnits = simulationMotionProfile.getPosition();
		
	}
}

//TODO: Integrate this

double PositionControlledSingleAxisMachine::axisPositionToMachinePosition(double axisPosition){
	if(b_invertDirection) return -1.0f * (axisPosition - machineZero_axisUnits);
	return axisPosition - machineZero_axisUnits;
}

double PositionControlledSingleAxisMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(b_invertDirection) return axisVelocity * -1.0;
	return axisVelocity;
}

double PositionControlledSingleAxisMachine::machinePositionToAxisPosition(double machinePosition){
	if(b_invertDirection) return (-1.0f * machinePosition) + machineZero_axisUnits;
	return machinePosition + machineZero_axisUnits;
}

double PositionControlledSingleAxisMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(b_invertDirection) return machineVelocity * -1.0;
	return machineVelocity;
}


double PositionControlledSingleAxisMachine::getLowPositionLimit(){
	//if the machine motion is inverted relative to the axis motion, we also need to invert the limits
	if(b_invertDirection) return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
}

double PositionControlledSingleAxisMachine::getHighPositionLimit(){
	//if the machine motion is inverted relative to the axis motion, we also need to invert the limits
	if(b_invertDirection) return axisPositionToMachinePosition(getAxis()->getLowPositionLimit());
	return axisPositionToMachinePosition(getAxis()->getHighPositionLimit());
}

double PositionControlledSingleAxisMachine::getPositionNormalized(){
	double lowLimit = getLowPositionLimit();
	double highLimit = getHighPositionLimit();
	if(isSimulating()) return (simulationMotionProfile.getPosition() - lowLimit) / (highLimit - lowLimit);
	return getAxis()->getActualPosition_normalized();
}

double PositionControlledSingleAxisMachine::getVelocityNormalized(){
	double velocityLimit = getAxis()->getVelocityLimit_axisUnitsPerSecond();
	if(isSimulating()) return simulationMotionProfile.getVelocity() / velocityLimit;
	return getAxis()->getActualVelocityNormalized();
}


bool PositionControlledSingleAxisMachine::hasManualPositionTarget(){
	if(!isAxisConnected()) return false;
	auto axis = getAxis();
	if(!isSimulating()) return axis->controlMode == ControlMode::Mode::POSITION_TARGET;
	return controlMode == SimulationControlMode::POSITION_TARGET;
}

double PositionControlledSingleAxisMachine::getManualPositionTarget(){
	if(!isAxisConnected()) return false;
	auto axis = getAxis();
	if(!isSimulating()) return axisPositionToMachinePosition(axis->targetInterpolation->outPosition);
	return simulationTargetInterpolation->outPosition;
}
