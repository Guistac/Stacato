#pragma once

#include "Environnement/NodeGraph/ProcessorNode.h"

class ClockNode : public ProcessorNode {

	DEFINE_PROCESSOR_NODE("Clock", ClockNode)

public:
	
	virtual void assignIoData() {
		addIoData(output_seconds);
	}

	std::shared_ptr<ioData> output_seconds = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "output");

	virtual void process(bool inputDataValid) {
		output_seconds->set(Timing::getTime_seconds());
	}
};
