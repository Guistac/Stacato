#pragma once

#include "Motion/Machine/Machine.h"

class BinaryOscillator6x : public Machine {
	
	DEFINE_MACHINE_NODE(BinaryOscillator6x, "Hex Binary Oscillator", "HexBinaryOscillator", "Oscillators")

	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodeData::Type::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Gpio Devices", NodePinFlags_AcceptMultipleInputs);

	std::vector<std::shared_ptr<NodePin>> outputPins = {
		std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 1"),
		std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 2"),
		std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 3"),
		std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 4"),
		std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 5"),
		std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Output 6")
	};

	std::shared_ptr<AnimatableParameter> minOffTimeParameter = std::make_shared<AnimatableParameter>("Min Off Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> maxOffTimeParameter = std::make_shared<AnimatableParameter>("Max Off Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> minOnTimeParameter = std::make_shared<AnimatableParameter>("Min On Time", ParameterDataType::Type::REAL_PARAMETER, "s");
	std::shared_ptr<AnimatableParameter> maxOnTimeParameter = std::make_shared<AnimatableParameter>("Max On Time", ParameterDataType::Type::REAL_PARAMETER, "s");
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
	std::vector<double> nextStateChangeDelay_seconds = std::vector<double>(6, 0.0);
	float minOffTime_seconds = 0.0;
	float maxOffTime_seconds = 0.0;
	float minOnTime_seconds = 0.0;
	float maxOnTime_seconds = 0.0;
	void updateOscillatorParametersFromTracks();
	void stopOscillatorParameterPlayback();

	float maxTime_seconds = 60.0;

	//=== manual control ===
	void manuallySetOutput(int i, bool s);
	void setOutput(int i, bool s);

	void startOscillator();
	void stopOscillator();
	bool isOscillatorActive();
	bool b_oscillatorActive = false;
	bool b_startOscillator = false;
	bool b_stopOscillator = false;


	//=== Axes ===
	int getGpioDeviceCount();
	std::shared_ptr<GpioDevice> getGpioDevice(int i);

	//=== Saving & Loading ===
	bool saveMachine(tinyxml2::XMLElement* xml);
	bool loadMachine(tinyxml2::XMLElement* xml);
};
