#include <pch.h>

#include "PositionControlledSingleAxisMachine.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Motion/AnimatableParameter.h"

#include <tinyxml2.h>


void PositionControlledSingleAxisMachine::assignIoData() {
	addIoData(positionControlledAxisLink);
	std::shared_ptr<Machine> thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	positionParameter = std::make_shared<AnimatableParameter>("Position", thisMachine, ParameterDataType::Type::KINEMATIC_POSITION_CURVE, "units");
	animatableParameters.push_back(positionParameter);
	//TODO: initialize animatable parameter
}

bool PositionControlledSingleAxisMachine::isEnabled() {
	return b_enabled;
}

bool PositionControlledSingleAxisMachine::isReady() {
	if (!isAxisConnected()) return false;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	//if (axis->hasManualControlsEnabled()) return false;
	if (!axis->isReady()) return false;
	//if (axis->getCommandType() != MotionCommand::Type::POSITION_COMMAND) return false;
	//if (axis->axisPositionUnitType != PositionUnit::Type::LINEAR) return false;
	return true;
}

bool PositionControlledSingleAxisMachine::isMoving() {
	if (isAxisConnected()) return getAxis()->isMoving();
	return false;
}

void PositionControlledSingleAxisMachine::enable() {
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
					break;
				}
			}
			});
		machineEnabler.detach();
	}
}

void PositionControlledSingleAxisMachine::disable() {
	b_enabled = false;
	if (isAxisConnected()) getAxis()->disable();
}

void PositionControlledSingleAxisMachine::process() {
	/*
	if (!isAxisConnected()) return;
	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	//Get Realtime values from axis
	actualPosition_machineUnits = axis->actualPosition_axisUnits;
	actualVelocity_machineUnits = axis->actualVelocity_axisUnitsPerSecond;

	//update parameter data
	positionParameter->actualValue.realValue = actualPosition_machineUnits;

	//Update Time
	profileTime_seconds = axis->profileTime_seconds;
	profileDeltaTime_seconds = axis->profileTimeDelta_seconds;

	//Handle state changes
	if (b_enabled) {
		if (!axis->isEnabled()) disable();
	}

	if (b_enabled) {
		//if the machine is enabled
		//its drives the profile generator values of the axis
		//it also sends commands to the actuators of the axis

		if (positionParameter->hasParameterTrack()) {
			double previousProfilePosition = profilePosition_machineUnits;
			AnimatableParameterValue playbackPosition;
			positionParameter->getActiveTrackParameterValue(playbackPosition);
			profilePosition_machineUnits = playbackPosition.realValue;
			profileVelocity_machineUnitsPerSecond = (profilePosition_machineUnits - previousProfilePosition) / profileDeltaTime_seconds;
		}
		else {
			switch (controlMode) {
				case ControlMode::Mode::MANUAL_VELOCITY_TARGET:
					velocityTargetControl();
					break;
				case ControlMode::Mode::MANUAL_POSITION_TARGET:
					positionTargetControl();
					break;
			}
		}
		axis->profilePosition_axisUnits = profilePosition_machineUnits;
		axis->profileVelocity_axisUnitsPerSecond = profileVelocity_machineUnitsPerSecond;
		axis->sendActuatorCommands();
	}
	else {
		//if the machine is disabled
		//we only update the profile generator by copying axis values
		profilePosition_machineUnits = axis->profilePosition_axisUnits;
		profileVelocity_machineUnitsPerSecond = axis->profileVelocity_axisUnitsPerSecond;
	}
	*/
}





bool PositionControlledSingleAxisMachine::isAxisConnected() {
	return positionControlledAxisLink->isConnected();
}

std::shared_ptr<PositionControlledAxis> PositionControlledSingleAxisMachine::getAxis() {
	return positionControlledAxisLink->getConnectedPins().front()->getPositionControlledAxis();
}






void PositionControlledSingleAxisMachine::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter->dataType == value.type) {
		if (parameter == positionParameter) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			axis->moveToPositionWithVelocity(value.realValue, rapidVelocity_machineUnitsPerSecond, rapidAcceleration_machineUnitsPerSecond);
		}
	}
}

float PositionControlledSingleAxisMachine::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		std::shared_ptr<PositionControlledAxis> axis = getAxis();
		axis->targetInterpolation->getProgressAtTime(axis->profileTime_seconds);
	}
	return 0.0;
}

bool PositionControlledSingleAxisMachine::isParameterAtValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter->dataType == value.type) {
		if (parameter == positionParameter) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis();
			return axis->targetInterpolation->getProgressAtTime(axis->profileTime_seconds) == 1.0;
		}
	}
	return false;
}

void PositionControlledSingleAxisMachine::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == positionParameter) {
		std::shared_ptr<PositionControlledAxis> axis = getAxis();
		axis->setVelocityTarget(0.0);
	}
}

bool PositionControlledSingleAxisMachine::validateParameterCurve(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& curves) {
	bool b_curveValid = true;

	std::shared_ptr<PositionControlledAxis> axis = getAxis();

	if (parameter == positionParameter && curves.size() == 1) {

		using namespace Motion;
		std::shared_ptr<Curve> curve = curves.front();

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
			}
			else if (std::abs(interpolation->interpolationVelocity) > axis->getVelocityLimit_axisUnitsPerSecond()) {
				interpolation->validationError = ValidationError::Error::INTERPOLATION_VELOCITY_LIMIT_EXCEEDED;
				interpolation->b_valid = false;
				b_curveValid = false;
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















void PositionControlledSingleAxisMachine::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	if (isAxisConnected()) getAxis()->getDevices(output);
}






bool PositionControlledSingleAxisMachine::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if (rapidsXML == nullptr) return Logger::warn("Could not find Rapids attribute");
	if (rapidsXML->QueryDoubleAttribute("Velocity_machineUnitsPerSecond", &rapidVelocity_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid velocity attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration_machineUnitsPerSecondSquared", &rapidAcceleration_machineUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could find rapid acceleration attribute");
	
	return true;
}

bool PositionControlledSingleAxisMachine::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;

	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity_machineUnitsPerSecond", rapidVelocity_machineUnitsPerSecond);
	rapidsXML->SetAttribute("Acceleration_machineUnitsPerSecondSquared", rapidAcceleration_machineUnitsPerSecond);
	
	return true;
}