#pragma once

#include "Environnement/NodeGraph/Node.h"

class DisplayNode : public Node {
public:

	DEFINE_NODE(DisplayNode, "Display", "Display", Node::Type::PROCESSOR, "Utility")

	std::shared_ptr<NodePin> displayInput = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_INPUT, "value: ", NodePin::Flags::DisableDataField | NodePin::Flags::ForceDataField);
	std::shared_ptr<double> inputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override {
		if (displayInput->isConnected()) displayInput->copyConnectedPinValue();
 }
	
	virtual bool onSerialization() override { Component::onSerialization(); }
	virtual bool onDeserialization() override { Component::onDeserialization(); }
	virtual void onConstruction() override {
		Component::onConstruction();
		displayInput->assignData(inputPinValue);
		addNodePin(displayInput);
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override { Component::onCopyFrom(source); }
	
	
};
