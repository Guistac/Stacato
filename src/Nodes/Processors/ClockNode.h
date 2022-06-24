#pragma once

#include "Environnement/Node.h"

class ClockNode : public Node {
public:

	DEFINE_NODE(ClockNode, "Clock", "Clock", Node::Type::CLOCK, "Time")

	std::shared_ptr<NodePin> output_seconds = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "output", NodePin::Flags::DisableDataField);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess();
	
};

void ClockNode::initialize(){
	output_seconds->assignData(outputPinValue);
	addNodePin(output_seconds);
}

void ClockNode::inputProcess() {
	*outputPinValue = Timing::getProgramTime_seconds();
}
