#include <pch.h>

#include "NodeGraph.h"
#include "Environnement/DeviceNode.h"

namespace Environnement::NodeGraph{

	void evaluate() {
		std::vector<std::shared_ptr<Node>> dummyNodeList;
		evaluate(dummyNodeList);
	}

	void evaluate(Device::Type deviceType) {
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

	void evaluate(std::shared_ptr<Node> node) {
		std::vector<std::shared_ptr<Node>> nodeList;
		nodeList.push_back(node);
		evaluate(nodeList);
	}

	void evaluate(std::vector<std::shared_ptr<Node>> startNodes) {

		//regardless of the type of node thats being processed
		//clock nodes always update their value, so they must always be processed
		for (auto node : getNodes()) {
			std::shared_ptr<Node> currentNode = node;
			if (node->getType() == Node::Type::CLOCK) {
				startNodes.push_back(node);
			}
		}
		
		//PHASE I:
		//find all nodes that will be processed on this cycle and add them in 'nodesToProcess'
		//find them by starting from the initial nodes, and go down the connection tree
		//this way they will more or less be in the right update order
		//each time they are added, set the processed flag so they don't get added twice
		std::vector<std::shared_ptr<Node>> nodesToProcess;
		
		//set all nodes to unprocessed, so they can all be added exactly once
		for (auto node : getNodes()) node->b_wasProcessed = false;
		
		//linked nodes are all nodes that were found on the outputs of the previous linked nodes
		//linked nodes is initialized with the start nodes
		std::vector<std::shared_ptr<Node>> linkedNodes = startNodes;
		
		//next linked nodes are candidates for the next round of processing
		std::vector<std::shared_ptr<Node>> nextLinkedNodes;

		while (!linkedNodes.empty()) {
			for (auto node : linkedNodes) {
				if (node->wasProcessed()) continue;
				//add the linked nodes to the list of nodes that will need to be processed
				nodesToProcess.push_back(node);
				//set the processed flag, so branches from this node's outputs won't be searched again
				//this prevents an infinite loop
				node->b_wasProcessed = true;
				//get each node that is connected to an output of the current node
				//and add it to the list of next linked nodes
				for (auto outputData : node->getOutputPins()) {
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

		//PHASE 2:
		//all nodes will be attempt processing in the order of the nodeToProcess list
		//certain conditions apply:
		//-a node can only process once
		//-a node can only process if all nodes connected to its inputs are already processed
		//-if a circular dependency causes no nodes to process, we force processing of the first one left in the list

		//set all nodes to processed except the ones that need to be processed and clear circular dependency flags
		for (auto node : getNodes()) node->b_wasProcessed = true;
		for (auto node : getNodes()) node->b_circularDependencyFlag = false;
		for (auto node : nodesToProcess) node->b_wasProcessed = false;

		//keep a list of candidates for the next round of processing
		std::vector<std::shared_ptr<Node>> nextNodesToProcess;

		while (!nodesToProcess.empty()) {
			
			for (auto node : nodesToProcess) {
				//skip processing if:
				//-the node was already processed
				//-it is an IOdevice (which cannot process, only read inputs and write outputs)
				if (node->wasProcessed() || node->getType() == Node::Type::IODEVICE) {
					node->b_wasProcessed = true;
					continue;
				}
				//processing takes place if:
				//-all nodes connected to the current nodes inputs are already processed
				//-a circular dependency condition forces the current node to process
				if (node->areAllLinkedInputNodesProcessed() || node->b_circularDependencyFlag) {
					node->process();
					node->b_wasProcessed = true;
				}
				//if the node can't process, add it as a canditate for the next processing round
				else nextNodesToProcess.push_back(node);
			}

			//if the canditate list didn't change, we are stuck in a cyclic dependency loop.
			//force update of the first node in the list on the next cycle to try and resolve the loop
			if (nodesToProcess == nextNodesToProcess) nextNodesToProcess.front()->b_circularDependencyFlag = true;

			//swap the list of canditates with the nodes to be processed on the next round
			nodesToProcess.clear();
			nodesToProcess.swap(nextNodesToProcess);
		}
	}


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
