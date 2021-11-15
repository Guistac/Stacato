#include <pch.h>

#include "Oscillator3x.h"

#include <pch.h>

#include "Motion/Subdevice.h"
#include "Motion/AnimatableParameter.h"


void Oscillator3x::assignIoData() {
	addIoData(linearAxis1Pin);
	addIoData(linearAxis2Pin);
	addIoData(linearAxis3Pin);

	std::shared_ptr<Machine> thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());

	frequencyParameter = std::make_shared<AnimatableParameter>("Frequency", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "Hz");
	amplitudeParameter = std::make_shared<AnimatableParameter>("Amplitude", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "mm");
	phaseOffsetParameter = std::make_shared<AnimatableParameter>("Phase Offset", thisMachine, ParameterDataType::Type::REAL_PARAMETER, "\xC2\xB0");

	animatableParameters.push_back(frequencyParameter);
	animatableParameters.push_back(amplitudeParameter);
	animatableParameters.push_back(phaseOffsetParameter);
}

void Oscillator3x::process() {
}

bool Oscillator3x::isEnabled() {
	return false;
}

bool Oscillator3x::isReady() {
	return false;
}

void Oscillator3x::enable() {
}

void Oscillator3x::disable() {
}

bool Oscillator3x::isMoving() {
	return false;
}

void Oscillator3x::rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
}

float Oscillator3x::getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) {
	return 0.0;
}

bool Oscillator3x::isParameterAtValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) {
	return false;
}

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

