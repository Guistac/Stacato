#pragma once

#include "Motion/Machine/Machine.h"

class Oscillator3x : public Machine {
	
	DEFINE_MACHINE_NODE(Oscillator3x, "Triple Oscillator", "TripleOscillator")

	std::shared_ptr<NodePin> axis1Pin = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Axis 1");
	std::shared_ptr<NodePin> axis2Pin = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Axis 2");
	std::shared_ptr<NodePin> axis3Pin = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Axis 3");

	std::shared_ptr<AnimatableParameter> frequencyParameter;
	std::shared_ptr<AnimatableParameter> minAmplitudeParameter;
	std::shared_ptr<AnimatableParameter> maxAmplitudeParameter;
	std::shared_ptr<AnimatableParameter> phaseOffsetParameter;

	std::shared_ptr<AnimatableParameter> axis1Position;
	std::shared_ptr<AnimatableParameter> axis2Position;
	std::shared_ptr<AnimatableParameter> axis3Position;

	bool b_enabled = false;
	bool b_startAtLowerLimit = true;
	bool b_oscillatorActive = false;

	double previousCycleTime_seconds = 0.0;
	double oscillatorXOffset_radians = 0.0;
	double axis1NormalizedPosition = 0.0;
	double axis2NormalizedPosition = 0.0;
	double axis3NormalizedPosition = 0.0;

	bool b_primingToOscillatorStart = false;

	bool isAxisConnected(int);
	std::shared_ptr<Axis> getAxis(int);

};