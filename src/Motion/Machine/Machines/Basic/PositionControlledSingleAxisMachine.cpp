#include <pch.h>

#include "PositionControlledSingleAxisMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

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
					Logger::info("Enabled Machine {}", getName());
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

void PositionControlledSingleAxisMachine::process() {
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//Get Realtime values from axis
	double actualPosition_machineUnits = axis->getActualPosition_axisUnits();
	double actualVelocity_machineUnits = axis->getActualVelocity_axisUnitsPerSecond();

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
		axis->profilePosition_axisUnits = playbackPosition.realValue;
		axis->profileVelocity_axisUnitsPerSecond = (playbackPosition.realValue - previousProfilePosition_machineUnits) / profileDeltaTime_seconds;
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
		
		Logger::warn("playback: {}", playbackPosition.realValue);
		
		simulationMotionProfile.setPosition(playbackPosition.realValue);
		simulationMotionProfile.setVelocity((playbackPosition.realValue - previousProfilePosition_machineUnits) / profileDeltaTime_seconds);
	}else if(controlMode == SimulationControlMode::VELOCITY_TARGET){
		
		simulationMotionProfile.matchVelocity(profileDeltaTime_seconds, manualVelocityTarget_machineUnitsPerSecond, rapidAcceleration_machineUnitsPerSecond);
		
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
	if(!isSimulating()) getAxis()->setVelocityTarget(velocityTarget_machineUnitsPerSecond);
	else{
		//SIMULATION TEST
		controlMode = SimulationControlMode::VELOCITY_TARGET;
		simulationTargetInterpolation->resetValues();
	}
}

void PositionControlledSingleAxisMachine::moveToPosition(double target_machineUnits) {
	if(!isSimulating()) getAxis()->moveToPositionWithVelocity(target_machineUnits, rapidVelocity_machineUnitsPerSecond, rapidAcceleration_machineUnitsPerSecond);
	else{
		//SIMULATION TEST
		
		double highLimit = getAxis()->getHighPositionLimit();
		double lowLimit = getAxis()->getLowPositionLimit();
		
		double simulationTime = Environnement::getTime_seconds();
		double profilePosition = simulationMotionProfile.getPosition();
		double profileAcceleration = simulationMotionProfile.getAcceleration();
		double profileVelocity = simulationMotionProfile.getVelocity();
		
		if (target_machineUnits > highLimit) target_machineUnits = highLimit;
		else if (target_machineUnits < lowLimit) target_machineUnits = lowLimit;
		
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
			//axis->moveToPositionWithVelocity(value.realValue, rapidVelocity_machineUnitsPerSecond, rapidAcceleration_machineUnitsPerSecond);
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
				return axis->getProfilePosition_axisUnits() == value.realValue && axis->getProfileVelocity_axisUnitsPerSecond() == 0.0;
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

void PositionControlledSingleAxisMachine::onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		if(!isSimulating()) getAxis()->setVelocityTarget(0.0);
		else{
			//SIMULATION TEST
			setVelocityTarget(0.0);
		}
	}
}

void PositionControlledSingleAxisMachine::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter == positionParameter) {
		if(!isSimulating()) value.realValue = getAxis()->getActualPosition_axisUnits();
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


	if (parameterTrack->parameter == positionParameter && parameterTrack->curves.size() == 1) {

		using namespace Motion;
		std::shared_ptr<Curve> curve = parameterTrack->curves.front();

		for (auto& controlPoint : curve->points) {
			controlPoint->validationError = ValidationError::Error::NO_VALIDATION_ERROR;

			if (controlPoint->position < axis->getLowPositionLimit() || controlPoint->position > axis->getHighPositionLimit())
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
				if (point.position > axis->getHighPositionLimit() || point.position < axis->getLowPositionLimit()) {
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
		lowLimit = axis->getLowPositionLimit();
		highLimit = axis->getHighPositionLimit();
		return true;
	}
	return false;
}


void PositionControlledSingleAxisMachine::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
	if (parameter == positionParameter && outputCurves.size() == 1 && targetPoints.size() == 1) {
	
		std::shared_ptr<PositionControlledAxis> axis = getAxis();

		std::shared_ptr<Motion::ControlPoint> startPoint = std::make_shared<Motion::ControlPoint>();
		startPoint->position = axis->getActualPosition_axisUnits();
		startPoint->velocity = axis->getActualVelocity_axisUnitsPerSecond();
		startPoint->velocityOut = axis->getActualVelocity_axisUnitsPerSecond();
		startPoint->rampOut = rampIn;

		std::shared_ptr<Motion::ControlPoint> endPoint = targetPoints.front();
		std::shared_ptr<Motion::Interpolation> timedInterpolation = std::make_shared<Motion::Interpolation>();

		Motion::TrapezoidalInterpolation::getClosestTimeAndVelocityConstrainedInterpolation(startPoint, endPoint, axis->getVelocityLimit_axisUnitsPerSecond(), timedInterpolation);
	}
	//movement from current position to the target position arriving at 0 velocity
}


void PositionControlledSingleAxisMachine::onEnableHardware() {
}

void PositionControlledSingleAxisMachine::onDisableHardware() {
}

bool PositionControlledSingleAxisMachine::isSimulationReady(){
	return isAxisConnected();
}

void PositionControlledSingleAxisMachine::onEnableSimulation() {
	//nothing to do here really
}

void PositionControlledSingleAxisMachine::onDisableSimulation() {
	//nothing to here either
}









void PositionControlledSingleAxisMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}

bool PositionControlledSingleAxisMachine::loadMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if (rapidsXML == nullptr) return Logger::warn("Could not find Rapids attribute");
	if (rapidsXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &rapidVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid velocity attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &rapidAcceleration_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid acceleration attribute");
	
	return true;
}

bool PositionControlledSingleAxisMachine::saveMachine(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", rapidVelocity_machineUnitsPerSecond);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", rapidAcceleration_machineUnitsPerSecond);
	
	return true;
}





double PositionControlledSingleAxisMachine::axisPositionToMachinePosition(double axisPosition){
	if(b_invertDirection) return 1.0f * (axisPosition - axisUnitOffset);
	else return axisPosition - axisUnitOffset;
}

double PositionControlledSingleAxisMachine::axisVelocityToMachineVelocity(double axisVelocity){
	if(b_invertDirection) return axisVelocity * -1.0;
	else return axisVelocity;
}

double PositionControlledSingleAxisMachine::machinePositionToAxisPosition(double machinePosition){
	if(b_invertDirection) return 1.0f * (machinePosition + axisUnitOffset);
	else return machinePosition - axisUnitOffset;
}

double PositionControlledSingleAxisMachine::machineVelocityToAxisVelocity(double machineVelocity){
	if(b_invertDirection) return machineVelocity * -1.0;
	else return machineVelocity;
}

