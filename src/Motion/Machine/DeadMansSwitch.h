#pragma once

#include "NodeGraph/Node.h"
#include "NodeGraph/NodePin.h"


class DeadMansSwitch : public Node {
	DEFINE_PROCESSOR_NODE(DeadMansSwitch, "Dead Man's Switch", "DeadMansSwitch", "Safety")
	
	std::shared_ptr<NodePin> gpioDevicePin = std::make_shared<NodePin>(NodeData::Type::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Gpio Device");
	std::shared_ptr<NodePin> switchPressedPin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Switch Pressed");
	std::shared_ptr<NodePin> switchConnectedPin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Switch Connected");
	
	std::shared_ptr<NodePin> switchLedPin = std::make_shared<NodePin>(NodeData::Type::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Switch LED Signal");
	std::shared_ptr<NodePin> deadMansSwitchLink = std::make_shared<NodePin>(NodeData::Type::DEAD_MANS_SWITCH_LINK, DataDirection::NODE_OUTPUT, "Dead Man's Switch");
	
	virtual void assignIoData();
	virtual void process();
};
