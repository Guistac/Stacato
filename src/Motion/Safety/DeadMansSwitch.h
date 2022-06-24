#pragma once

#include "Environnement/NodeGraph/Node.h"

class DeadMansSwitch : public Node {
public:
	
	DEFINE_NODE(DeadMansSwitch, "Dead Man's Switch", "DeadMansSwitch", Node::Type::PROCESSOR, "Safety")
	
	std::shared_ptr<bool> b_switchPressed = std::make_shared<bool>(false);
	std::shared_ptr<bool> b_switchConnected = std::make_shared<bool>(false);
	std::shared_ptr<bool> b_switchLed = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Gpio Device", "GpioDevicePin");
	std::shared_ptr<NodePin> switchPressedPin = std::make_shared<NodePin>(b_switchPressed, NodePin::Direction::NODE_INPUT, "Switch Pressed", "SwitchPressedPin");
	std::shared_ptr<NodePin> switchConnectedPin = std::make_shared<NodePin>(b_switchLed, NodePin::Direction::NODE_INPUT, "Switch Connected", "SwitchConnectedPin");
	
	std::shared_ptr<NodePin> switchLedPin = std::make_shared<NodePin>(b_switchLed, NodePin::Direction::NODE_OUTPUT, "Switch LED Signal", "SwitchLEDSignalPin");
	std::shared_ptr<NodePin> deadMansSwitchLink = std::make_shared<NodePin>(NodePin::DataType::DEAD_MANS_SWITCH, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Dead Man's Switch");
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	
	virtual void nodeSpecificGui() override;
	void controlsGui();
	void settingsGui();
};
