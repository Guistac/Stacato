#pragma once

#include "NodeGraph/Node.h"

class DisplayNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(DisplayNode, "Display", "Display", "Utility")
	
	virtual void assignIoData() {
		displayInput->assignData(inputPinValue);
		addIoData(displayInput);
	}

	std::shared_ptr<NodePin> displayInput = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "value: ", NodePin::Flags::DisableDataField | NodePin::Flags::ForceDataField);
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);
	
	
	virtual void process() {
		if (displayInput->isConnected()) displayInput->copyConnectedPinValue();
	}
};
