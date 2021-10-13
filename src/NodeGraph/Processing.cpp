#include <pch.h>

#include "NodeGraph.h"
#include "Device.h"

void NodeGraph::evaluate() {
	std::vector<std::shared_ptr<Node>> dummyNodeList;
	evaluate(dummyNodeList);
}

void NodeGraph::evaluate(Device::Type deviceType) {
	std::vector<std::shared_ptr<Node>> devices;
	//get all the nodes that will be processed
	for (auto node : getNodes()) {
		if (node->getType() == Node::Type::IODEVICE) {
			std::shared_ptr<Device> device = std::dynamic_pointer_cast<Device>(node);
			if(device->getDeviceType() == deviceType) devices.push_back(node);
		}
	}
	evaluate(devices);
}

void NodeGraph::evaluate(std::shared_ptr<Node> node) {
	std::vector<std::shared_ptr<Node>> nodeList;
	nodeList.push_back(node);
	evaluate(nodeList);
}

void NodeGraph::evaluate(std::vector<std::shared_ptr<Node>> startNodes) {

	//regardless of the type of node thats being processed
	//clock nodes always update their value, so they must always be processed
	for (auto node : getNodes()) {
		std::shared_ptr<Node> currentNode = node;
		if (node->getType() == Node::Type::CLOCK) {
			startNodes.push_back(node);
		}
	}

	//FIRST PHASE:
	//find all nodes that need processing when the start nodes update
	//start from the start nodes and go down the tree adding each node to the nodesToProcess vector
	//nodes can be added to the list more than one time, this is not important

	std::vector<std::shared_ptr<Node>> nodesToProcess;
	std::vector<std::shared_ptr<Node>> linkedNodes = startNodes;
	std::vector<std::shared_ptr<Node>> nextLinkedNodes;

	mutex.lock();

	std::chrono::time_point start = std::chrono::high_resolution_clock::now();

	//set all nodes to unprocessed, so they will all be searched exactly once
	for (auto node : nodes) node->b_wasProcessed = false;

	while (!linkedNodes.empty()) {
		//linked nodes are all nodes that were found on the outputs of the previous linked nodes
		//linked nodes is initialized with the start nodes
		for (auto node : linkedNodes) {
			if (node->wasProcessed()) continue;
			//add the linked nodes to the list of nodes that will need to be processed
			nodesToProcess.push_back(node);
			//set the processed flag, so branches from this node's outputs won't be searched again
			//this prevents an infinite loop
			node->b_wasProcessed = true;
			//get each node that is connected to an output of the current node
			//and add it to the list of next linked nodes
			for (auto outputData : node->getNodeOutputData()) {
				for (auto outputLink : outputData->getLinks()) {
					std::shared_ptr<Node> linkedOutputNode = outputLink->getOutputData()->getNode();
					nextLinkedNodes.push_back(linkedOutputNode);
				}
			}
		}
		//take the next linked nodes are now the current linked nodes
		//the tree search goes deeper until no more linked nodes can be found on any branches
		//this will cause an issue with circular node connections !!!!!
		linkedNodes.clear();
		linkedNodes.swap(nextLinkedNodes);
	}

	//TODO: don't update device nodes other than the specified type
	//TODO: precompile a list of instructions rather than scanning the tree each time

	//SECOND PHASE:
	//all nodesToProcess will need to be processed in a certain order
	//going from outputs to inputs
	//each node can only be processed if all the nodes connected to its inputs have been processed
	//each node can only be processed once

	//set all nodes to processed except the ones that need to be processed
	//this ensures only nodeToProcess will be processed
	for (auto node : nodes) node->b_wasProcessed = true;
	for (auto node : nodes) node->b_circularDependencyFlag = false;
	for (auto node : nodesToProcess) node->b_wasProcessed = false;

	std::vector<std::shared_ptr<Node>> nextNodesToProcess;

	while (!nodesToProcess.empty()) {
		//for each node to process, check if all the nodes connected to the current nodes inputs have been processed
		//if yes process the node
		for (auto node : nodesToProcess) {
			//don't process a node twice
			if (node->wasProcessed()) continue;
			//check if all nodes linked to inputs of the current node were processed
			if (node->areAllLinkedInputNodesProcessed() || node->b_circularDependencyFlag) {
				//if yes, process the node
				node->process();
				node->b_wasProcessed = true;
			}
			else {
				//add to a vector to try again on the next loop
				nextNodesToProcess.push_back(node);
			}
		}

		if (nodesToProcess == nextNodesToProcess) {
			nextNodesToProcess.front()->b_circularDependencyFlag = true;
		}

		nodesToProcess.clear();
		nodesToProcess.swap(nextNodesToProcess);
	}

	std::chrono::time_point end = std::chrono::high_resolution_clock::now();

	long long timeNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
	//Logger::warn("Process Time: {} nanoseconds", timeNanos);

	mutex.unlock();
}



//each node that needs processing on a specific cycle only gets processed once
//we need to find all the nodes that need processing
//starting from newly generated output data on devices and clocks
//going from inputs to outputs, up to the last nodes without outputs

//we start processing the new data of the device nodes
//from there we find the nodes connected to the outputs of the device nodes
//then we need to run a check for which of those nodes can be processed
//the check only passes when all nodes connected to the input of the curent node did process if they needed processing
//if the check passes, we process the node and mark it as processed
//on the next cycle this node will have its outputs ready