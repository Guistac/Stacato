#pragma once

#include "NodeGraph/DeviceNode.h"

class ClockNode : public Node {
public:

	DEFINE_CLOCK_NODE("Clock", ClockNode)

	std::shared_ptr<NodePin> output_seconds = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "output", NodePinFlags_DisableDataField);

	virtual void assignIoData() {
		addIoData(output_seconds);
	}

	virtual void process() {
		output_seconds->set(Timing::getTime_seconds());
	}
};
