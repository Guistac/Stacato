#include <pch.h>

#include "Oscillator3x.h"

#include <pch.h>

#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

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

	double profileTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
	double profileDeltaTime_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();

	if (b_enabled) {
		if (!isEnabled()) disable();
		else if (!isReady()) disable();
	}

	if (b_startOscillator && !b_oscillatorActive) {
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
			double velocity_axisUnits = (position_axisUnits - axis->getProfileVelocity_axisUnitsPerSecond()) / profileDeltaTime_seconds;

			//send commands to axis
			axis->profilePosition_axisUnits = position_axisUnits;
			axis->profileVelocity_axisUnitsPerSecond = velocity_axisUnits;
			axis->sendActuatorCommands();
		}

	}

}

//======================= STATE CONTROL ========================

bool Oscillator3x::isEnabled() {
	return b_enabled;
}

bool Oscillator3x::isReady() {
	//the machine is ready when a minimum of 1 axis is ready
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i) && getAxis(i)->isReady()) return true;
	}
	return false;
}

void Oscillator3x::enable() {
	if (isReady()) {
		std::thread axisEnabler([this]() {	
			for (int i = 0; i < 3; i++) {
				if (isAxisConnected(i)) {
					getAxis(i)->enable();
				}
			}
			using namespace std::chrono;
			system_clock::time_point enableTime = system_clock::now();
			do {
				bool allReadyEnabled = true;
				for (int i = 0; i < 3; i++) {
					if (!isAxisConnected(i)) continue;
					std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
					if (axis->isReady() && !axis->isEnabled()) {
						allReadyEnabled = false;
					}
				}
				if (allReadyEnabled) {
					b_enabled = true;
					return Logger::info("Enabled Machine {}", getName());
				}
				std::this_thread::sleep_for(milliseconds(15));
			} while (system_clock::now() - enableTime < milliseconds(100));
			Logger::warn("Could not enable Machine {}, all axes were not enabled on time", getName());
			b_enabled = false;
		});
		axisEnabler.detach();
	}
}

void Oscillator3x::disable() {
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) getAxis(i)->disable();
	}
	b_startOscillator = false;
	b_oscillatorActive = false;
	b_enabled = false;
}

bool Oscillator3x::isMoving() {
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i) && getAxis(i)->isMoving()) return true;
	}
	return false;
}

//======================== OSCILLATOR CONTROL ============================

void Oscillator3x::startOscillator() {
	b_startOscillator = true;
}

void Oscillator3x::stopOscillator() {
	b_stopOscillator = true;
}


//====================== MANUAL CONTROLS ==========================

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
		axis->moveToPositionWithVelocity(position_axisUnits, velocity_axisUnits, acceleration_axisUnits);
	}
}

void Oscillator3x::moveAllToPosition(double position_normalized) {
	for (int i = 0; i < 3; i++) {
		moveToPosition(i, position_normalized);
	}
}

bool Oscillator3x::isOscillatorReadyToStart() {
	bool b_ready = true;
	bool noAxisEnabled = true;
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) {
			std::shared_ptr<PositionControlledAxis> axis = getAxis(i);
			if (axis->isEnabled()) {
				noAxisEnabled = false;
				double rangedPosition_normalized = (axis->getProfilePosition_axisUnits() - axis->getLowPositionLimit()) / axis->getRange_axisUnits();
				if (b_startAtLowerLimit) {
					if(std::abs(rangedPosition_normalized - oscillatorLowerAmplitude_normalized) > 0.001) b_ready = false;
				}
				else {
					if(std::abs(rangedPosition_normalized - oscillatorUpperAmplitude_normalized) > 0.001) b_ready = false;
				}
			}
		}
	}
	if (noAxisEnabled) return false;
	return b_ready;
}

void Oscillator3x::moveToOscillatorStart() {
	if (b_startAtLowerLimit) moveAllToPosition(oscillatorLowerAmplitude_normalized);
	else moveAllToPosition(oscillatorUpperAmplitude_normalized);
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


//=========================== AXES & DEVICES ==============================

void Oscillator3x::getDevices(std::vector<std::shared_ptr<Device>>& output) {
	for (int i = 0; i < 3; i++) {
		if (isAxisConnected(i)) getAxis(i)->getDevices(output);
	}
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
	return axisConnected;
}





//========================================================================
//============================ PLOT INTERFACE ============================
//========================================================================



void Oscillator3x::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
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
}

float Oscillator3x::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {

		

	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	return 0.0;
}


bool Oscillator3x::isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup) {


	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	return false;
}

void Oscillator3x::onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) {
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
}

void Oscillator3x::onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter) {
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
}

void Oscillator3x::getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
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
}

void Oscillator3x::cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) {
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
}


bool Oscillator3x::validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) {
	std::shared_ptr<AnimatableParameter> parameter = parameterTrack->parameter;
	if (parameter == frequencyParameter ||
		parameter == minAmplitudeParameter ||
		parameter == maxAmplitudeParameter ||
		parameter == phaseOffsetParameter ||
		parameter == oscillatorParameterGroup){

		std::shared_ptr<ParameterTrack> oscillatorParameterTrack;
		if (parameter == oscillatorParameterGroup) oscillatorParameterTrack = parameterTrack;
		else oscillatorParameterTrack = parameterTrack->parentParameterTrack;
		//validate all oscillator parameter tracks


	}
	else if (parameter == axis1PositionParameter) {

	}
	else if (parameter == axis2PositionParameter) {

	}
	else if (parameter == axis3PositionParameter) {

	}
	return false;
}

bool Oscillator3x::getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) {
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

	return true;
}

void Oscillator3x::getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) {
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
}



void Oscillator3x::enterSimulationMode() {}
void Oscillator3x::exitSimulationMode() {}
bool Oscillator3x::isInSimulationMode() {
	return false;
}

