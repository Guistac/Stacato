#pragma once

#include "NodeGraph/DeviceNode.h"

class ClockNode : public ioNode {
public:

	DEFINE_CLOCK_NODE("Clock", ClockNode)

	std::shared_ptr<ioData> output_seconds = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "output", ioDataFlags_DisableDataField);

	virtual void assignIoData() {
		addIoData(output_seconds);
	}

	virtual void process() {
		output_seconds->set(Timing::getTime_seconds());
	}
};
