#pragma once

#include "NodeGraph/Node.h"

class ClockNode : public Node {
public:

	DEFINE_CLOCK_NODE(ClockNode, "Clock", "Clock", "Time")

	std::shared_ptr<NodePin> output_seconds = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "output", NodePin::Flags::DisableDataField);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void assignIoData() {
		output_seconds->assignData(outputPinValue);
		addIoData(output_seconds);
	}

	virtual void process() {
		*outputPinValue = Timing::getProgramTime_seconds();
	}
};
