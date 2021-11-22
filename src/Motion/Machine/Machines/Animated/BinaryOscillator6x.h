#pragma once

#include "Motion/Machine/Machine.h"

class BinaryOscillator6x : public Machine {
	
	DEFINE_MACHINE_NODE(BinaryOscillator6x, "Hex Binary Oscillator", "HexBinaryOscillator", "Oscillators")

	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodeData::Type::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Gpio Devices", NodePinFlags_AcceptMultipleInputs);

	std::shared_ptr<NodePin> output1Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 1");
	std::shared_ptr<NodePin> output2Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 2");
	std::shared_ptr<NodePin> output3Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 3");
	std::shared_ptr<NodePin> output4Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 4");
	std::shared_ptr<NodePin> output5Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 5");
	std::shared_ptr<NodePin> output6Pin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 6");

	std::shared_ptr<AnimatableParameter> minOffTimeParameter = std::make_shared<AnimatableParameter>("Min Off Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> maxOffTimeParameter = std::make_shared<AnimatableParameter>("Min Off Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> minOnTimeParameter = std::make_shared<AnimatableParameter>("Min Off Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> maxOnTimeParameter = std::make_shared<AnimatableParameter>("Min Off Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> oscillatorParameterGroup = std::make_shared<AnimatableParameter>("Oscillator", std::vector<std::shared_ptr<AnimatableParameter>>({
		minOffTimeParameter,
		maxOffTimeParameter,
		minOnTimeParameter,
		maxOnTimeParameter
	}));

	//=== Control Variables ===
	bool b_enabled = false;
	std::vector<bool> outputSignals = std::vector<bool>(6, false);
	std::vector<double> currentStateLengths_seconds = std::vector<double>(6, 0.0);
	double minOffTime_seconds = 0.0;
	double maxOffTime_seconds = 0.0;
	double minOnTime_seconds = 0.0;
	double maxOnTime_seconds = 0.0;
	void updateOscillatorParametersFromTracks();

	void startOscillator();
	void stopOscillator();
	bool b_oscillatorEnabled = false;
	bool b_startOscillator = false;
	bool b_stopOscillator = false;

	//=== Axes ===
	int getGpioDeviceCount();
	std::shared_ptr<GpioDevice> getGpioDevice(int i);

	//=== Saving & Loading ===
	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
};
