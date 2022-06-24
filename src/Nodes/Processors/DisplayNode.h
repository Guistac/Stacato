#pragma once

#include "Environnement/Node.h"

class DisplayNode : public Node {
public:

	DEFINE_NODE(DisplayNode, "Display", "Display", Node::Type::PROCESSOR, "Utility")

	std::shared_ptr<NodePin> displayInput = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "value: ", NodePin::Flags::DisableDataField | NodePin::Flags::ForceDataField);
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess();
	
};

void DisplayNode::initialize(){
	displayInput->assignData(inputPinValue);
	addNodePin(displayInput);
}

void DisplayNode::inputProcess() {
	if (displayInput->isConnected()) displayInput->copyConnectedPinValue();
}
