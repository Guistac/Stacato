#include <pch.h>

#include "Oscillator3x.h"

#include <pch.h>

#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"

#include "Motion/Axis/PositionControlledAxis.h"
#include "Fieldbus/EtherCatFieldbus.h"

void Oscillator3x::assignIoData() {
	addIoData(linearAxis1Pin);
	addIoData(linearAxis2Pin);
	addIoData(linearAxis3Pin);

	addAnimatableParameter(oscillatorParameterGroup);
	addAnimatableParameter(axis1PositionParameter);
	addAnimatableParameter(axis2PositionParameter);
	addAnimatableParameter(axis3PositionParameter);
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

			std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
			double low = axis->getLowPositionLimit();
			double high = axis->getHighPositionLimit();

			double axisPosition = ((high - low) * (1.0 + std::cos(axisXOffset_radians)) / 2.0) + low;
			double axisVelocity = low + M_PI * frequencyValue.realValue * (high - low) * std::sin(axisXOffset_radians);

			axis->profilePosition_axisUnits = axisPosition;
			axis->profileVelocity_axisUnitsPerSecond = axisVelocity;
			axis->sendActuatorCommands();
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
	if (parameter == oscillatorParameterGroup) {
	
		
	}
	else if (parameter == axis1PositionParameter) {
		
	}
	else if (parameter == axis2PositionParameter) {
	
	}
	else if (parameter == axis3PositionParameter) {
		
	}
	else if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter) {

		Logger::critical("Oscillator should not receive an oscillator child parameter query, only query the group");
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
	if (idx == 0) return linearAxis1Pin->isConnected();
	else if (idx == 1) return linearAxis2Pin->isConnected();
	else if (idx == 2) return linearAxis3Pin->isConnected();
	else return false;
}
std::shared_ptr<PositionControlledAxis> Oscillator3x::getAxis(int idx) {
	if (idx == 0) return linearAxis1Pin->getConnectedPins().front()->getPositionControlledAxis();
	else if (idx == 1) return linearAxis2Pin->getConnectedPins().front()->getPositionControlledAxis();
	else if (idx == 2) return linearAxis3Pin->getConnectedPins().front()->getPositionControlledAxis();
	else return nullptr;
}
bool Oscillator3x::getAxes(std::vector<std::shared_ptr<PositionControlledAxis>>& output) {
	output.resize(3, nullptr);
	bool axisConnected = false;
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) {
			output[i] = getAxis(i);
			axisConnected = true;
		}
	}
	return !axisConnected;
}


/*
bool Oscillator3x::isOscillatorReadyToStartFromNormalizedPosition(double startPosition_normalized) {
	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	if (!getAxes(axes)) return false;
	for (auto& axis : axes) {
		if (axis) {
			
			

		}
	}
}
*/

void Oscillator3x::startOscillator() {
	
}

void Oscillator3x::stopOscillator() {
	b_oscillatorActive = false;
	
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) getAxis(i)->setVelocityTarget(0.0);
	}

}




//======= MANUAL CONTROLS ========

void Oscillator3x::setVelocityTarget(int axisIndex, double velocityTarget_normalized) {
	if (isAxisConnected(axisIndex)) {
		std::shared_ptr<PositionControlledAxis> axis = getAxis(axisIndex);
		double velocity_axisUnits = velocityTarget_normalized * axis->getRange_axisUnits();
		stopOscillator();
		axis->setVelocityTarget(velocity_axisUnits);
	}
}

void Oscillator3x::moveToPosition(int axisIndex, double position_normalized) {
	if (isAxisConnected(axisIndex)) {
		std::shared_ptr<PositionControlledAxis> axis = getAxis(axisIndex);
		double position_axisUnits = position_normalized * axis->getRange_axisUnits() + axis->getLowPositionLimit();
		double velocity_axisUnits = maxVelocity_normalized * axis->getRange_axisUnits();
		double acceleration_axisUnits = maxAcceleration_normalized * axis->getRange_axisUnits();
		stopOscillator();
		axis->moveToPositionWithVelocity(position_axisUnits, velocity_axisUnits, acceleration_axisUnits);
	}
}

void Oscillator3x::moveAllToPosition(double position_normalized) {
	stopOscillator();
	for (int i = 0; i < 3; i++) {
		moveToPosition(i, position_normalized);
	}
}


//================== MACHINE LIMITS ==================

void Oscillator3x::updateMachineLimits() {
	std::vector<std::shared_ptr<PositionControlledAxis>> axes;
	if (!getAxes(axes)) {
		maxOscillationFrequency = 0.0;
		maxVelocity_normalized = 0.0;
		maxAcceleration_normalized = 0.0;
		return;
	}

	double lowestNormalizedVelocity = std::numeric_limits<double>::infinity();
	double lowestNormalizedAcceleration = std::numeric_limits<double>::infinity();

	for (auto& axis : axes) {
		if (axis) {
			double normalizedVelocity = axis->getVelocityLimit_axisUnitsPerSecond() / axis->getRange_axisUnits();
			double normalizedAcceleration = axis->getAccelerationLimit_axisUnitsPerSecondSquared() / axis->getRange_axisUnits();
			lowestNormalizedVelocity = std::min(normalizedVelocity, lowestNormalizedVelocity);
			lowestNormalizedAcceleration = std::min(normalizedAcceleration, lowestNormalizedAcceleration);
		}
	}

	double maxFrequencyByVelocity = lowestNormalizedVelocity / (2.0 * M_PI);
	double maxFrequencyByAcceleration = std::sqrt(lowestNormalizedAcceleration / (4.0 * std::pow(M_PI, 2.0)));

	maxOscillationFrequency = std::min(maxFrequencyByVelocity, maxFrequencyByAcceleration);
	maxVelocity_normalized = lowestNormalizedVelocity;
	maxAcceleration_normalized = lowestNormalizedAcceleration;
}