#pragma once

#include "Environnement/NodeGraph/Node.h"

class ClockNode : public Node {
public:

	DEFINE_NODE(ClockNode, Node::Type::CLOCK, "Time")

	std::shared_ptr<NodePin> output_seconds;
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override {
		*outputPinValue = Timing::getProgramTime_seconds();
	}
	
	virtual bool onSerialization() override { Component::onSerialization(); }
	virtual bool onDeserialization() override { Component::onDeserialization(); }
	virtual void onConstruction() override {
		Node::onConstruction();
		setName("Clock");
		output_seconds = NodePin::createInstance(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "output", "output", NodePin::Flags::DisableDataField);
		output_seconds->assignData(outputPinValue);
		addNodePin(output_seconds);
	}
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override { Component::onCopyFrom(source); }
	
};

