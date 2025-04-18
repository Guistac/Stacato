#pragma once
/*
#include "Motion/Machine/Machine.h"

class Oscillator3x : public Machine {
	
	DEFINE_MACHINE_NODE(Oscillator3x, "Triple Oscillator", "TripleOscillator", "Oscillators")

	std::shared_ptr<NodePin> linearAxis1Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT, "Linear Axis 1");
	std::shared_ptr<NodePin> linearAxis2Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT, "Linear Axis 2");
	std::shared_ptr<NodePin> linearAxis3Pin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_INPUT, "Linear Axis 3");

	std::shared_ptr<AnimatableParameter> frequencyParameter = std::make_shared<AnimatableParameter>("Frequency", ParameterDataType::REAL_PARAMETER, "Hz");
	std::shared_ptr<AnimatableParameter> minAmplitudeParameter = std::make_shared<AnimatableParameter>("Min Amplitude", ParameterDataType::REAL_PARAMETER, "mm");
	std::shared_ptr<AnimatableParameter> maxAmplitudeParameter = std::make_shared<AnimatableParameter>("Max Amplitude", ParameterDataType::REAL_PARAMETER, "mm");
	std::shared_ptr<AnimatableParameter> phaseOffsetParameter = std::make_shared<AnimatableParameter>("Phase Offset", ParameterDataType::REAL_PARAMETER, "\x25"); //'�' == \xC2\xB0
	std::shared_ptr<AnimatableParameter> oscillatorParameterGroup = std::make_shared<AnimatableParameter>("Oscillator", std::vector<std::shared_ptr<AnimatableParameter>>({
		frequencyParameter,
		minAmplitudeParameter,
		maxAmplitudeParameter,
		phaseOffsetParameter
	}));
	
	std::shared_ptr<AnimatableParameter> axis1PositionParameter = std::make_shared<AnimatableParameter>("Axis 1 Position", ParameterDataType::POSITION, "mm");
	std::shared_ptr<AnimatableParameter> axis2PositionParameter = std::make_shared<AnimatableParameter>("Axis 2 Position", ParameterDataType::POSITION, "mm");
	std::shared_ptr<AnimatableParameter> axis3PositionParameter = std::make_shared<AnimatableParameter>("Axis 3 Position", ParameterDataType::POSITION, "mm");
	

	//=== Machine Limits ===
	double maxOscillationFrequency = 0.0;
	double maxVelocity_normalized = 0.0;
	double maxAcceleration_normalized = 0.0;
	double rapidVelocity_normalized = 0.0;
	double rapidAcceleration_normalized = 0.0;
	void updateMachineLimits();

	//=== Machine Settings ===
	bool b_startAtLowerLimit = true;

	//=== Machine State ===
	bool b_startOscillator = false;
	bool b_stopOscillator = false;
	bool b_oscillatorActive = false;
	bool isOscillatorReadyToStart();
	void moveToOscillatorStart();
	void startOscillator();
	void stopOscillator();

	//=== Control Variables ===
	double oscillatorXOffset_radians = 0.0;
	double axis1NormalizedPosition = 0.0;
	double axis2NormalizedPosition = 0.0;
	double axis3NormalizedPosition = 0.0;
	float oscillatorFrequency_hertz = 0.01;
	float oscillatorPhaseOffset_percent = 0.0;
	float oscillatorLowerAmplitude_normalized = 0.0;
	float oscillatorUpperAmplitude_normalized = 1.0;
	void updateOscillatorParametersFromTracks();
	void stopOscillatorParameterPlayback();

	//=== Manual Controls ===
	void setVelocityTarget(int axis, double velocityTarget_normalized);
	void moveToPosition(int axis, double position_normalized);
	void moveAllToPosition(double position_normalized);

	//=== Homing ===
	void startHoming();
	void stopHoming();
	bool isHoming();
	bool b_startHoming = false;
	bool b_stopHoming = false;
	bool b_homing = false;

	//=== Axes ===
	bool isAxisConnected(int);
	std::shared_ptr<PositionControlledAxis> getAxis(int);
	bool getAxes(std::vector<std::shared_ptr<PositionControlledAxis>>& output);
};
*/
