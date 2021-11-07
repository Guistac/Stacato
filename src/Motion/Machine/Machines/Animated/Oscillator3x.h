#pragma once

#include "Motion/Machine/Machine.h"

class Oscillator3x : public Machine {
	
	DEFINE_MACHINE_NODE(Oscillator3x, "Triple Oscillator", "TripleOscillator")

	std::shared_ptr<NodePin> linearAxis1Pin = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis 1");
	std::shared_ptr<NodePin> linearAxis2Pin = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis 2");
	std::shared_ptr<NodePin> linearAxis3Pin = std::make_shared<NodePin>(NodeData::Type::AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis 3");

	std::shared_ptr<AnimatableParameter> frequencyParameter;
	std::shared_ptr<AnimatableParameter> amplitudeParameter;
	std::shared_ptr<AnimatableParameter> phaseOffsetParameter;

};