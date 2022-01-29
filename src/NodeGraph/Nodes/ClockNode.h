#pragma once

#include "NodeGraph/Node.h"

class ClockNode : public Node {
public:

	DEFINE_NODE(ClockNode, "Clock", "Clock", Node::Type::PROCESSOR, "Time")

	std::shared_ptr<NodePin> output_seconds = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "output", NodePin::Flags::DisableDataField);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void process();
	
};

void ClockNode::initialize(){
	output_seconds->assignData(outputPinValue);
	addNodePin(output_seconds);
}

void ClockNode::process() {
	*outputPinValue = Timing::getProgramTime_seconds();
}
