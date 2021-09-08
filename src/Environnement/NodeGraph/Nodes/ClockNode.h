#pragma once

#include "Environnement/NodeGraph/ioNode.h"

class ClockNode : public ioNode {
public:

	DEFINE_DEVICE_NODE("Clock", ClockNode, DeviceType::CLOCK)

	std::shared_ptr<ioData> output_seconds = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "output");

	virtual void assignIoData() {
		addIoData(output_seconds);
	}

	virtual void process(bool inputDataValid) {
		output_seconds->set(Timing::getTime_seconds());
	}
};
