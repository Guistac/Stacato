#pragma once

#include "Environnement/NodeGraph/ioNode.h"

class DisplayNode : public ioNode {
public:

	DEFINE_PROCESSOR_NODE("Display", DisplayNode)
	

	virtual void assignIoData() {
		addIoData(displayInput);
	}

	std::shared_ptr<ioData> displayInput = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "input");

	virtual void process(bool inputDataValid) {}
};