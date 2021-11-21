#pragma once

#include "Motion/Machine/Machine.h"

class Oscillator3x : public Machine {
	
	DEFINE_MACHINE_NODE(Oscillator3x, "Triple Oscillator", "TripleOscillator", "Oscillators")

	std::shared_ptr<NodePin> linearAxis1Pin = std::make_shared<NodePin>(NodeData::Type::POSITION_CONTROLLED_AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis 1");
	std::shared_ptr<NodePin> linearAxis2Pin = std::make_shared<NodePin>(NodeData::Type::POSITION_CONTROLLED_AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis 2");
	std::shared_ptr<NodePin> linearAxis3Pin = std::make_shared<NodePin>(NodeData::Type::POSITION_CONTROLLED_AXIS_LINK, DataDirection::NODE_INPUT, "Linear Axis 3");

	std::shared_ptr<AnimatableParameter> frequencyParameter;
	std::shared_ptr<AnimatableParameter> minAmplitudeParameter;
	std::shared_ptr<AnimatableParameter> maxAmplitudeParameter;
	std::shared_ptr<AnimatableParameter> phaseOffsetParameter;

	std::shared_ptr<AnimatableParameter> axis1Position;
	std::shared_ptr<AnimatableParameter> axis2Position;
	std::shared_ptr<AnimatableParameter> axis3Position;

	bool b_enabled = false;
	bool b_startAtLowerLimit = true;

	double previousCycleTime_seconds = 0.0;
	double oscillatorXOffset_radians = 0.0;
	double axis1NormalizedPosition = 0.0;
	double axis2NormalizedPosition = 0.0;
	double axis3NormalizedPosition = 0.0;

	bool b_primingToOscillatorStart = false;

	//=== Machine Limits ===
	double maxOscillationFrequency = 0.0;
	double maxVelocity_normalized = 0.0;
	double maxAcceleration_normalized = 0.0;
	void updateMachineLimits();


	void setVelocityTarget(int axis, double velocityTarget_normalized);
	void moveToPosition(int axis, double position_normalized);
	void moveAllToPosition(double position_normalized);

	bool isOscillatorReadyToStartFromPosition(double startPosition_normalized);
	void startOscillator();
	void stopOscillator();
	bool b_oscillatorActive = false;

	bool isAxisConnected(int);
	std::shared_ptr<PositionControlledAxis> getAxis(int);
	bool getAxes(std::vector<std::shared_ptr<PositionControlledAxis>>& output);
};