#pragma once

#include "Environnement/NodeGraph/ProcessorNode.h"

class DisplayNode : public ProcessorNode {

	DEFINE_PROCESSOR_NODE("Display", DisplayNode)

public:

	virtual void assignIoData() {
		addIoData(displayInput);
	}

	std::shared_ptr<ioData> displayInput = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input");

	virtual void process(bool inputDataValid) {}
};