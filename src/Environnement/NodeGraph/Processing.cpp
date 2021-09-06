#include <pch.h>

#include "NodeGraph.h"

void NodeGraph::evaluate() {
	std::vector<std::shared_ptr<ioNode>> dummyNodeList;
	evaluate(dummyNodeList);
}

void NodeGraph::evaluate(DeviceNode::DeviceType deviceType) {
	std::vector<std::shared_ptr<ioNode>> deviceNodes;
	//get all the nodes that will be processed
	for (auto node : getIoNodes()) {
		if (node->getType() == ioNode::NodeType::IODEVICE) {
			auto device = std::dynamic_pointer_cast<DeviceNode>(node);
			if (device->getDeviceType() == deviceType) {
				deviceNodes.push_back(device);
			}
		}
	}
	evaluate(deviceNodes);
}

void NodeGraph::evaluate(std::shared_ptr<ioNode> node) {
	std::vector<std::shared_ptr<ioNode>> nodeList;
	nodeList.push_back(node);
	evaluate(nodeList);
}

void NodeGraph::evaluate(std::vector<std::shared_ptr<ioNode>> nodes) {

	//regardless of the type of node thats being processed
	//clock nodes always update their value, so they must always be processed
	for (auto node : getIoNodes()) {
		if (node->getType() == ioNode::NodeType::IODEVICE) {
			std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(node);
			if (device->getDeviceType() == DeviceNode::CLOCK) {
				nodes.push_back(device);
			}
		}
	}

	std::vector<std::shared_ptr<ioNode>> nodesToProcess = nodes;
	std::vector<std::shared_ptr<ioNode>> linkedNodes;
	std::vector<std::shared_ptr<ioNode>> processedNodes;

	mutex.lock();

	// do the tree evalution here
	while (!nodesToProcess.empty()) {
		for (auto node : nodesToProcess) {
			if (node->wasProcessed()) continue;
			node->process(true);
			node->b_wasProcessed = true;
			processedNodes.push_back(node);
			for (auto outputData : node->getNodeOutputData()) {
				outputData->copyToLinked();
				for (auto linkedNode : outputData->getNodesLinkedAtOutputs()) {
					linkedNodes.push_back(linkedNode);
				}
			}
		}
		nodesToProcess.clear();
		nodesToProcess.swap(linkedNodes);	
	}

	for (auto node : processedNodes) {
		node->b_wasProcessed = false;
	}

	mutex.unlock();
}