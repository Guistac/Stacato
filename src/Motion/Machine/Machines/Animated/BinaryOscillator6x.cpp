#include <pch.h>

#include "BinaryOscillator6x.h"

#include <pch.h>

#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Fieldbus/EtherCatFieldbus.h"

void BinaryOscillator6x::assignIoData() {

	addIoData(gpioDevicePin);
	addIoData(output1Pin);
	addIoData(output2Pin);
	addIoData(output3Pin);
	addIoData(output4Pin);
	addIoData(output5Pin);
	addIoData(output6Pin);

	addAnimatableParameter(oscillatorParameterGroup);
}

void BinaryOscillator6x::process() {

	double profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	double profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();

	/*

	if (b_enabled) {
		if (!isEnabled()) disable();
		else if (!isReady()) disable();
		bool noneEnabled = true;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) {
				if (getAxis(i)->isEnabled()) {
					noneEnabled = false;
					break;
				}
			}
		}
		if (noneEnabled) disable();
	}

	if (b_startHoming) {
		b_startHoming = false;
		b_homing = true;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) {
				getAxis(i)->startHoming();
			}
		}
	}
	if (b_homing) {
		bool homingFinished = true;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) {
				std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
				if (axis->homingStep != Homing::Step::FINISHED) {
					homingFinished = false;
				}
				if (axis->homingError != Homing::Error::NONE) {
					homingFinished = false;
					b_stopHoming = true;
				}
			}
		}
		if (homingFinished) {
			b_homing = false;
		}
	}
	if (b_stopHoming) {
		b_stopHoming = false;
		b_homing = false;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) {
				getAxis(i)->cancelHoming();
			}
		}
		Logger::warn("Cancel Homing");
	}

	if (b_startOscillator && !b_oscillatorActive) {
		updateOscillatorParametersFromTracks();
		b_startOscillator = false;
		b_oscillatorActive = true;
		oscillatorXOffset_radians = 0.0;
		axis1NormalizedPosition = 0.0;
		axis2NormalizedPosition = 0.0;
		axis3NormalizedPosition = 0.0;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) getAxis(i)->controlMode = ControlMode::Mode::MACHINE_CONTROL;
		}
	}
	else if (b_stopOscillator && b_oscillatorActive) {
		b_stopOscillator = false;
		b_oscillatorActive = false;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) getAxis(i)->setVelocityTarget(0.0);
		}
	}

	if (b_oscillatorActive) {
		updateOscillatorParametersFromTracks();
		//increment xOffset and get Phase Offset in radians
		oscillatorXOffset_radians += profileDeltaTime_seconds * oscillatorFrequency_hertz * 2.0 * M_PI;
		double phaseOffsetRadians = (oscillatorPhaseOffset_percent / 100.0) * 2.0 * M_PI;

		for (int i = 0; i < 3; i++) {
			if (!isAxisConnected(i)) continue;
			std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
			if (!axis->isEnabled()) continue;

			//adjust local xOffset for phase offset of each axis
			double axisXOffset_radians = oscillatorXOffset_radians - i * phaseOffsetRadians;
			axisXOffset_radians = std::max(axisXOffset_radians, 0.0); //clamp negative xOffset value to 0.0

			double positionNormalized;
			//get oscillator position using frequency and phase offset
			if (b_startAtLowerLimit) positionNormalized = (1.0 - std::cos(axisXOffset_radians)) / 2.0;
			else positionNormalized = (1.0 + std::cos(axisXOffset_radians)) / 2.0;

			//adjust for amplitude parameters
			positionNormalized = oscillatorLowerAmplitude_normalized + positionNormalized * (oscillatorUpperAmplitude_normalized - oscillatorLowerAmplitude_normalized);

			//convert normalized position to axis units
			double position_axisUnits = axis->getLowPositionLimit() + axis->getRange_axisUnits() * positionNormalized;

			//calculate current axis velocity
			double velocity_axisUnits = (position_axisUnits - axis->getProfilePosition_axisUnits()) / profileDeltaTime_seconds;

			//send commands to axis
			axis->profilePosition_axisUnits = position_axisUnits;
			axis->profileVelocity_axisUnitsPerSecond = velocity_axisUnits;
			axis->sendActuatorCommands();
		}

	}
*/
}

void BinaryOscillator6x::updateOscillatorParametersFromTracks() {
	AnimatableParameterValue value;
	/*
	if (frequencyParameter->hasParameterTrack()) {
		frequencyParameter->getActiveTrackParameterValue(value);
		oscillatorFrequency_hertz = value.realValue;
	}
	if (minAmplitudeParameter->hasParameterTrack()) {
		minAmplitudeParameter->getActiveTrackParameterValue(value);
		oscillatorLowerAmplitude_normalized = value.realValue;
	}
	if (maxAmplitudeParameter->hasParameterTrack()) {
		maxAmplitudeParameter->getActiveTrackParameterValue(value);
		oscillatorUpperAmplitude_normalized = value.realValue;
	}
	if (phaseOffsetParameter->hasParameterTrack()) {
		phaseOffsetParameter->getActiveTrackParameterValue(value);
		oscillatorPhaseOffset_percent = value.realValue;
	}
	*/
}

//======================= STATE CONTROL ========================

bool BinaryOscillator6x::isEnabled() {
	return b_enabled;
}

bool BinaryOscillator6x::isReady() {
	return false;
}

void BinaryOscillator6x::enable() {
	if (isReady()) {
		b_enabled = true;
	}
}

void BinaryOscillator6x::disable() {
	b_oscillatorEnabled = false;
	b_enabled = false;
}

bool BinaryOscillator6x::isMoving() {
	return false;
}

//======================== OSCILLATOR CONTROL ============================

void BinaryOscillator6x::startOscillator() {
	b_startOscillator = true;
}

void BinaryOscillator6x::stopOscillator() {
	b_stopOscillator = true;
}



//=========================== AXES & DEVICES ==============================

int BinaryOscillator6x::getGpioDeviceCount() {
	return 0;
}

std::shared_ptr<GpioDevice> BinaryOscillator6x::getGpioDevice(int i) {
	return nullptr;
}




//========================================================================
//============================ PLOT INTERFACE ============================
//========================================================================



void BinaryOscillator6x::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	/*
	if (parameter == minAmplitudeParameter && b_startAtLowerLimit){
		moveAllToPosition(value.realValue);
	}
	else if (parameter == maxAmplitudeParameter && !b_startAtLowerLimit) {
		moveAllToPosition(value.realValue);
	}
	else if (parameter == frequencyParameter || parameter == phaseOffsetParameter || parameter == oscillatorParameterGroup) {
		//do nothing here since we don't know the target position from these parameters
	}
	else if (parameter == axis1PositionParameter) {
		moveToPosition(0, value.realValue);
	}
	else if (parameter == axis2PositionParameter) {
		moveToPosition(1, value.realValue);
	}
	else if (parameter == axis3PositionParameter) {
		moveToPosition(2, value.realValue);
	}
	*/
}

float BinaryOscillator6x::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	/*
	if (parameter == minAmplitudeParameter || parameter == maxAmplitudeParameter) {
		float lowestRapidProgress = 1.0;
		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) {
				std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
				if (axis->isEnabled()) {
					float progress = axis->targetInterpolation->getProgressAtTime(EtherCatFieldbus::getCycleProgramTime_seconds());
					lowestRapidProgress = std::min(lowestRapidProgress, lowestRapidProgress);
				}
			}
		}
		return lowestRapidProgress;
	}else if (parameter == frequencyParameter || parameter == phaseOffsetParameter || parameter == oscillatorParameterGroup) {
		return 1.0;
	}
	else if (parameter == axis1PositionParameter) {
		return 0.0;
	}
	else if (parameter == axis2PositionParameter) {
		return 0.0;
	}
	else if (parameter == axis3PositionParameter) {
		return 0.0;
	}
	*/
	return 0.0;
}


bool BinaryOscillator6x::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	/*
	if (parameter == frequencyParameter || parameter == phaseOffsetParameter || parameter == oscillatorParameterGroup) {
		return true;
	}
	else if (parameter == minAmplitudeParameter) {
		if (b_startAtLowerLimit) {
			return isOscillatorReadyToStart();
		}
		else return true;
	}else if(parameter == maxAmplitudeParameter) {
		if (b_startAtLowerLimit) return true;
		else {
			return isOscillatorReadyToStart();
		}
	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
	return false;
}

void BinaryOscillator6x::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {
	/*
	if (parameter == frequencyParameter || parameter == minAmplitudeParameter || parameter == maxAmplitudeParameter || parameter == phaseOffsetParameter) {
		//do nothing here
	}
	else if(parameter == oscillatorParameterGroup) {
		//start oscillator
		b_startOscillator = true;
	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
}

void BinaryOscillator6x::onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter) {
	/*
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {
		//stop Oscillator
		b_stopOscillator = true;
	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
}

void BinaryOscillator6x::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	/*
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {

		//move to start or end

	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
}

void BinaryOscillator6x::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
	/*
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {

		for (int i = 0; i < 3; i++) {
			if (isAxisConnected(i)) setVelocityTarget(i, 0.0);
		}

	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
}

//validate curves, interpolations and points
bool BinaryOscillator6x::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
	bool trackValid = true;
	/*
	std::shared_ptr<AnimatableParameter> parameter = parameterTrack->parameter;
	if (parameter == frequencyParameter) {
		std::shared_ptr<Motion::Curve> curve = parameterTrack->curves.front();
		curve->b_valid = true;
		for (auto& point : curve->points) {
			if (point->position < 0.0 || point->position > maxOscillationFrequency) { 
				trackValid = false;
				curve->b_valid = false;
				point->validationError = Motion::ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			}
			else {
				point->b_valid = true;
				point->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
			}
		}
		for (auto& interpolation : curve->interpolations) {
			interpolation->b_valid = true;
			interpolation->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
			for (auto& point : interpolation->displayPoints) {
				if (point.position < 0.0 || point.position > maxOscillationFrequency) {
					trackValid = false;
					curve->b_valid = false;
					interpolation->validationError = Motion::ValidationError::Error::INTERPOLATION_POSITION_OUT_OF_RANGE;
					break;
				}
			}
		}
	}
	else if (parameter == minAmplitudeParameter || parameter == maxAmplitudeParameter) {
		std::shared_ptr<Motion::Curve> curve = parameterTrack->curves.front();
		curve->b_valid = true;
		for (auto& point : curve->points) {
			if (point->position < 0.0 || point->position > 1.0) {
				trackValid = false;
				curve->b_valid = false;
				point->validationError = Motion::ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			}
			else {
				point->b_valid = true;
				point->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
			}
		}
		for (auto& interpolation : curve->interpolations) {
			interpolation->b_valid = true;
			interpolation->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
			for (auto& point : interpolation->displayPoints) {
				if (point.position < 0.0 || point.position > 1.0) {
					trackValid = false;
					curve->b_valid = false;
					interpolation->validationError = Motion::ValidationError::Error::INTERPOLATION_POSITION_OUT_OF_RANGE;
					break;
				}
			}
		}
	}
	else if (parameter == phaseOffsetParameter) {
		std::shared_ptr<Motion::Curve> curve = parameterTrack->curves.front();
		for (auto& point : curve->points) {
			if (point->position < 0.0 || point->position > 100.0) {
				trackValid = false;
				curve->b_valid = false;
				point->validationError = Motion::ValidationError::Error::CONTROL_POINT_POSITION_OUT_OF_RANGE;
			}
			else {
				point->b_valid = true;
				point->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
			}
		}
		for (auto& interpolation : curve->interpolations) {
			interpolation->b_valid = true;
			interpolation->validationError = Motion::ValidationError::Error::NO_VALIDATION_ERROR;
			for (auto& point : interpolation->displayPoints) {
				if (point.position < 0.0 || point.position > 100.0) {
					trackValid = false;
					curve->b_valid = false;
					interpolation->validationError = Motion::ValidationError::Error::INTERPOLATION_POSITION_OUT_OF_RANGE;
					break;
				}
			}
		}
	}else if(parameter == oscillatorParameterGroup){
		trackValid = true;
	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
	return trackValid;
}

bool BinaryOscillator6x::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	/*
	if (parameter == frequencyParameter) {
		lowLimit = 0.0;
		highLimit = maxOscillationFrequency;
	}
	else if (parameter == minAmplitudeParameter || parameter == maxAmplitudeParameter) {
		lowLimit = 0.0;
		highLimit = 1.0;
	}
	else if (parameter == phaseOffsetParameter) {
		lowLimit = 0.0;
		highLimit = 100.0;
	}	
	else if(parameter == oscillatorParameterGroup) {
		lowLimit = 0.0;
		highLimit = 0.0;
	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
	return true;
}

void BinaryOscillator6x::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
	/*
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {

		//move to start or end

	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	*/
}



void BinaryOscillator6x::enterSimulationMode() {}
void BinaryOscillator6x::exitSimulationMode() {}
bool BinaryOscillator6x::isInSimulationMode() {
	return false;
}




void BinaryOscillator6x::getDevices(std::vector<std::shared_ptr<Device>>& output) {}

bool BinaryOscillator6x::save(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	/*
	XMLElement* limitsXML = xml->InsertNewChildElement("Limits");
	limitsXML->SetAttribute("MaxFrequency", maxOscillationFrequency);
	limitsXML->SetAttribute("MaxVelocity", maxVelocity_normalized);
	limitsXML->SetAttribute("MaxAcceleration", maxAcceleration_normalized);
	XMLElement* rapidsXML = xml->InsertNewChildElement("Rapids");
	rapidsXML->SetAttribute("Velocity", rapidVelocity_normalized);
	rapidsXML->SetAttribute("Acceleration", rapidAcceleration_normalized);
	XMLElement* oscillatorXML = xml->InsertNewChildElement("Oscillator");
	oscillatorXML->SetAttribute("StartFromLowerLimit", b_startAtLowerLimit);
	*/
	return true;
}

bool BinaryOscillator6x::load(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	/*
	XMLElement* limitsXML = xml->FirstChildElement("Limits");
	if (limitsXML == nullptr) return Logger::warn("Could not find limits attribute");
	if (limitsXML->QueryDoubleAttribute("MaxFrequency", &maxOscillationFrequency) != XML_SUCCESS) return Logger::warn("Could not find max frequency attribute");
	if (limitsXML->QueryDoubleAttribute("MaxVelocity", &maxVelocity_normalized) != XML_SUCCESS) return Logger::warn("Could not find max velocity attribute");
	if (limitsXML->QueryDoubleAttribute("MaxAcceleration", &maxAcceleration_normalized) != XML_SUCCESS) return Logger::warn("Could not find max acceleration attribute");

	XMLElement* rapidsXML = xml->FirstChildElement("Rapids");
	if (rapidsXML == nullptr) return Logger::warn("Could not finds rapids attribute");
	if (rapidsXML->QueryDoubleAttribute("Velocity", &rapidVelocity_normalized) != XML_SUCCESS) return Logger::warn("Could not find rapid velocity Attribute");
	if (rapidsXML->QueryDoubleAttribute("Acceleration", &rapidAcceleration_normalized) != XML_SUCCESS) return Logger::warn("Could not find rapid acceleration Attribute");

	XMLElement* oscillatorXML = xml->FirstChildElement("Oscillator");
	if (oscillatorXML == nullptr) return Logger::warn("Could not find Oscillator Attribute");
	if (oscillatorXML->QueryBoolAttribute("StartFromLowerLimit", &b_startAtLowerLimit) != XML_SUCCESS) return Logger::warn("Could not find start from lower limit attribute");
	*/
	return true;
}