#pragma once

#include "NodeGraph/Node.h"

class DisplayNode : public Node {
public:

	DEFINE_PROCESSOR_NODE(DisplayNode, "Display", "Display", "Utility")
	

	virtual void assignIoData() {
		addIoData(displayInput);
	}

	std::shared_ptr<NodePin> displayInput = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "value: ", NodePinFlags_DisableDataField | NodePinFlags_ForceDataField);

	virtual void process() {
		if (displayInput->isConnected()) displayInput->set(displayInput->getLinks().front()->getInputData()->getReal());
		else displayInput->set(0.0);
	}
};