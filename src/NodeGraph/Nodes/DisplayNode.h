#pragma once

#include "NodeGraph/ioNode.h"

class DisplayNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Display", DisplayNode, "Utility")
	

	virtual void assignIoData() {
		addIoData(displayInput);
	}

	std::shared_ptr<ioData> displayInput = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "value: ", ioDataFlags_DisableDataField | ioDataFlags_ForceDataField);

	virtual void process(bool inputDataValid) {
		if (displayInput->isConnected()) displayInput->set(displayInput->getLinks().front()->getInputData()->getReal());
		else displayInput->set(0.0);
	}
};