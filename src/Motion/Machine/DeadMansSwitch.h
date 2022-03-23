#pragma once

#include "Environnement/Node.h"
#include "Environnement/NodePin.h"


class DeadMansSwitch : public Node {
public:
	
	DEFINE_NODE(DeadMansSwitch, "Dead Man's Switch", "DeadMansSwitch", Node::Type::PROCESSOR, "Safety")
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Gpio Device");
	std::shared_ptr<NodePin> switchPressedPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Switch Pressed");
	std::shared_ptr<NodePin> switchConnectedPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_INPUT, "Switch Connected");
	
	std::shared_ptr<bool> switchPressedValue = std::make_shared<bool>(false);
	std::shared_ptr<bool> switchConnectedValue = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> switchLedPin = std::make_shared<NodePin>(NodePin::DataType::BOOLEAN, NodePin::Direction::NODE_OUTPUT, "Switch LED Signal");
	std::shared_ptr<NodePin> deadMansSwitchLink = std::make_shared<NodePin>(NodePin::DataType::DEAD_MANS_SWITCH, NodePin::Direction::NODE_OUTPUT, "Dead Man's Switch");
	
	std::shared_ptr<bool> switchLedValue = std::make_shared<bool>(false);
	
	virtual void process();
	
};
