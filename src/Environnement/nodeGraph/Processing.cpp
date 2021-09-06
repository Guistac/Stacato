#include <pch.h>

#include "NodeGraph.h"


void NodeGraph::process(DeviceNode::DeviceType deviceType) {
	std::vector<std::shared_ptr<DeviceNode>> deviceNodes;

	//get all the nodes that will be processed
	for (auto node : getIoNodes()) {
		if (node->getType() == ioNode::NodeType::IODEVICE) {
			auto device = std::dynamic_pointer_cast<DeviceNode>(node);
			if (device->getDeviceType() == deviceType) {
				deviceNodes.push_back(device);
			}
		}
	}
}