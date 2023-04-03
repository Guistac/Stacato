#pragma once

#include "Environnement/NodeGraph/Node.h"

class ClockNode : public Node {
public:

	DEFINE_NODE(ClockNode, "Clock", "Clock", Node::Type::CLOCK, "Time")

	std::shared_ptr<NodePin> output_seconds = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "output", NodePin::Flags::DisableDataField);
	std::shared_ptr<double> outputPinValue = std::make_shared<double>(0.0);

	virtual void inputProcess() override {
		*outputPinValue = Timing::getProgramTime_seconds();
	}
	
	virtual bool onSerialization() override { Component::onSerialization(); }
	virtual bool onDeserialization() override { Component::onDeserialization(); }
	virtual void onConstruction() override {
		Component::onConstruction();
		output_seconds->assignData(outputPinValue);
		addNodePin(output_seconds);
	}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override { Component::onCopyFrom(source); }
	
};

