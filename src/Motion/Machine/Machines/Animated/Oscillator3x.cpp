#include <pch.h>

#include "Oscillator3x.h"

#include <pch.h>

#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"

#include "Motion/Axis/Axis.h"
#include "Fieldbus/EtherCatFieldbus.h"

void Oscillator3x::assignIoData() {
	addIoData(axis1Pin);
	addIoData(axis2Pin);
	addIoData(axis3Pin);

	std::shared_ptr<Machine> thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());

	frequencyParameter = std::make_shared<AnimatableParameter>("Frequency", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "Hz");
	minAmplitudeParameter = std::make_shared<AnimatableParameter>("Min Amplitude", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "mm");
	maxAmplitudeParameter = std::make_shared<AnimatableParameter>("Max Amplitude", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "mm");
	phaseOffsetParameter = std::make_shared<AnimatableParameter>("Phase Offset", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "\x25"); //'°' == \xC2\xB0

	axis1Position = std::make_shared<AnimatableParameter>("Axis 1 Position", thisMachine, ParameterDataType::Type::KINEMATIC_POSITION_CURVE, "mm");
	axis2Position = std::make_shared<AnimatableParameter>("Axis 2 Position", thisMachine, ParameterDataType::Type::KINEMATIC_POSITION_CURVE, "mm");
	axis3Position = std::make_shared<AnimatableParameter>("Axis 3 Position", thisMachine, ParameterDataType::Type::KINEMATIC_POSITION_CURVE, "mm");

	animatableParameters.push_back(frequencyParameter);
	animatableParameters.push_back(minAmplitudeParameter);
	animatableParameters.push_back(maxAmplitudeParameter);
	animatableParameters.push_back(phaseOffsetParameter);

	animatableParameters.push_back(axis1Position);
	animatableParameters.push_back(axis2Position);
	animatableParameters.push_back(axis3Position);
}

void Oscillator3x::process() {

	double cycleTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	double deltaTime_seconds = cycleTime_seconds - previousCycleTime_seconds;
	previousCycleTime_seconds = cycleTime_seconds;

	if (!isEnabled()) return;
	if (!isReady()) disable();


	if (!b_oscillatorActive
		&& frequencyParameter->hasParameterTrack()
		&& phaseOffsetParameter->hasParameterTrack()
		&& minAmplitudeParameter->hasParameterTrack()
		&& maxAmplitudeParameter->hasParameterTrack()) {
		b_oscillatorActive = true;
		oscillatorXOffset_radians = 0.0;
		axis1NormalizedPosition = 0.0;
		axis2NormalizedPosition = 0.0;
		axis3NormalizedPosition = 0.0;
	}

	if (b_oscillatorActive) {
	
		AnimatableParameterValue frequencyValue;
		frequencyParameter->getActiveTrackParameterValue(frequencyValue);
		AnimatableParameterValue phaseOffsetValue;
		frequencyParameter->getActiveTrackParameterValue(phaseOffsetValue);
		AnimatableParameterValue minAmplitudeValue;
		minAmplitudeParameter->getActiveTrackParameterValue(minAmplitudeValue);
		AnimatableParameterValue maxAmplitudeValue;
		maxAmplitudeParameter->getActiveTrackParameterValue(maxAmplitudeValue);

		oscillatorXOffset_radians += deltaTime_seconds * frequencyValue.realValue * 2.0 * M_PI;
		double phaseOffsetRadians = (phaseOffsetValue.realValue / 100.0) * 2.0 * M_PI;

		int axisCount = 3;

		for (int i = 0; i < axisCount; i++) {
			double axisXOffset_radians = oscillatorXOffset_radians - i * phaseOffsetRadians;

			std::shared_ptr<Axis> axis = getAxis(i+1);
			double low = axis->getLowPositionLimitWithClearance();
			double high = axis->getHighPositionLimitWithClearance();

			double axisPosition = ((high - low) * (1.0 + std::cos(axisXOffset_radians)) / 2.0) + low;
			double axisVelocity = low + M_PI * frequencyValue.realValue * (high - low) * std::sin(axisXOffset_radians);

			axis->profilePosition_axisUnits = axisPosition;
			axis->profileVelocity_axisUnitsPerSecond = axisVelocity;
			axis->setActuatorCommands();
		}

	}




}

bool Oscillator3x::isEnabled() {
	return false;
}

bool Oscillator3x::isReady() {
	if (!isAxisConnected(1) || !getAxis(1)->isReady()) return false;
	else if (!isAxisConnected(2) || !getAxis(2)->isReady()) return false;
	else if (!isAxisConnected(3) || !getAxis(3)->isReady()) return false;
	return true;
}

void Oscillator3x::enable() {
	if (isReady()) {
		getAxis(1)->enable();
		getAxis(2)->enable();
		getAxis(3)->enable();
		b_enabled = true;
	}
}

void Oscillator3x::disable() {
	getAxis(1)->disable();
	getAxis(2)->disable();
	getAxis(3)->disable();
	b_enabled = false;
}

bool Oscillator3x::isMoving() {
	return false;
}

void Oscillator3x::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter) {
		
		if (!b_primingToOscillatorStart) {
			b_primingToOscillatorStart = true;

			if (b_startAtLowerLimit) {
				//move all axis to their lowest position
			}
			else {
				//move all axis to their highest position
			}
		}
	}
	else if (parameter == axis1Position) {
		
	}
	else if (parameter == axis2Position) {
	
	}
	else if (parameter == axis3Position) {
		
	}
}

float Oscillator3x::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	return 0.0;
}


bool Oscillator3x::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	return false;
}

void Oscillator3x::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {}

void Oscillator3x::onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter) {}

void Oscillator3x::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {}



void Oscillator3x::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {}

void Oscillator3x::startParameterPlayback(std::shared_ptr<AnimatableParameter> parameters) {}
void Oscillator3x::stopParameterPlayback(std::shared_ptr<AnimatableParameter> parameters) {}


bool Oscillator3x::validateParameterCurve(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& curves) {
	return false;
}

bool Oscillator3x::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
	return true;
}

void Oscillator3x::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {}


void Oscillator3x::getDevices(std::vector<std::shared_ptr<Device>>& output) {
}


void Oscillator3x::enterSimulationMode() {}
void Oscillator3x::exitSimulationMode() {}
bool Oscillator3x::isInSimulationMode() {
	return false;
}



bool Oscillator3x::isAxisConnected(int idx) {
	if (idx == 1) return axis1Pin->isConnected();
	else if (idx == 2) return axis2Pin->isConnected();
	else if (idx == 3) return axis3Pin->isConnected();
	else return false;
}
std::shared_ptr<Axis> Oscillator3x::getAxis(int idx) {
	if (idx == 1) return axis1Pin->getConnectedPins().front()->getAxis();
	else if (idx == 2) return axis2Pin->getConnectedPins().front()->getAxis();
	else if (idx == 3) return axis3Pin->getConnectedPins().front()->getAxis();
	else return nullptr;
}