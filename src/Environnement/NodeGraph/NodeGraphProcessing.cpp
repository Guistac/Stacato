#include <pch.h>

#include "NodeGraph.h"
#include "Environnement/NodeGraph/DeviceNode.h"

#include <iostream>


//this compiles a single direction of execution, starting from a set of starting nodes
std::vector<std::shared_ptr<NodeGraphProcess::Instruction>> NodeGraph::compileOrderedInstructions(std::vector<std::shared_ptr<Node>>& startNodes,
																								  NodeGraphProcess::Direction processDirection){
		
	if(processDirection == NodeGraphProcess::Direction::INPUT_PROCESS){
		//in the input process, regardless of the nodes being processed, clock nodes always update their value, so they must always be processed
		for (auto node : getNodes()) {
			if (node->getType() == Node::Type::CLOCK) startNodes.push_back(node);
		}
	}
	
	std::vector<std::shared_ptr<NodeGraphProcess::Instruction>> instructions;
	for(auto node : getNodes()) node->b_wasProcessed = false;
	
	std::vector<std::shared_ptr<NodePin>> triggeringPins;
	std::vector<std::shared_ptr<NodePin>> nextTriggeringPins;
	
	for(auto& startNode : startNodes){
		instructions.push_back(NodeGraphProcess::Instruction::make(startNode, processDirection));
		startNode->b_wasProcessed = true;
		std::vector<std::shared_ptr<NodePin>> thisNodeTriggeringPins;
		switch(processDirection){
			case NodeGraphProcess::Direction::INPUT_PROCESS: thisNodeTriggeringPins = startNode->getUpdatedPinsAfterInputProcess(); break;
			case NodeGraphProcess::Direction::OUTPUT_PROCESS: thisNodeTriggeringPins = startNode->getUpdatedPinsAfterOutputProcess(); break;
		}
		triggeringPins.insert(triggeringPins.end(), thisNodeTriggeringPins.begin(), thisNodeTriggeringPins.end());
	}
	
	while(!triggeringPins.empty()){
		for(auto& triggeringPin : triggeringPins){
			
			for(auto& processStarterPin : triggeringPin->getConnectedPins()){
					
				std::shared_ptr<Node> processedNode = processStarterPin->parentNode;
				NodeGraphProcess::Direction processDirection;
				
				//skip triggering the node connected to the pin if:
				//-the node was already processed on this cycle
				//-the node is an io device (those can't process, they can only read inputs and write outputs)
				//-the pin is an output but is not bidirectional
				if(processedNode->b_wasProcessed) continue;
				else if(processedNode->getType() == Node::Type::IODEVICE) continue;
				else if(processStarterPin->isInput()) processDirection = NodeGraphProcess::Direction::INPUT_PROCESS;
				else if(processStarterPin->isOutput() && processStarterPin->isBidirectional()) processDirection = NodeGraphProcess::Direction::OUTPUT_PROCESS;
				else continue;
				
				instructions.push_back(NodeGraphProcess::Instruction::make(processedNode, processDirection));
				processedNode->b_wasProcessed = true;
				
				std::vector<std::shared_ptr<NodePin>> thisNodeTriggeringPins;
				switch(processDirection){
					case NodeGraphProcess::Direction::INPUT_PROCESS: thisNodeTriggeringPins = processedNode->getUpdatedPinsAfterInputProcess(); break;
					case NodeGraphProcess::Direction::OUTPUT_PROCESS: thisNodeTriggeringPins = processedNode->getUpdatedPinsAfterOutputProcess(); break;
				}
				
				nextTriggeringPins.insert(nextTriggeringPins.end(), thisNodeTriggeringPins.begin(), thisNodeTriggeringPins.end());
				
			}
			
		}
		
		triggeringPins.clear();
		triggeringPins.swap(nextTriggeringPins);
	}
	
	
	for (auto node : getNodes()) node->b_wasProcessed = true;
	for (auto node : getNodes()) node->b_circularDependencyFlag = false;
	for (auto& instruction : instructions) instruction->processedNode->b_wasProcessed = false;
	
	std::vector<std::shared_ptr<NodeGraphProcess::Instruction>> instructionsOrdered;
	std::vector<std::shared_ptr<NodeGraphProcess::Instruction>> nextInstructionCanditates;
	
	while(!instructions.empty()){
		
		for (auto& instruction : instructions) {
			
			auto& processedNode = instruction->processedNode;
			
			//skip processing if:
			//-the node was already processed
			//-it is an IOdevice (which cannot process, only read inputs and write outputs)
			if (processedNode->b_wasProcessed || processedNode->getType() == Node::Type::IODEVICE) {
				processedNode->b_wasProcessed = true;
				continue;
			}
			
			//processing can only happen if all triggering nodes have been processed first
			//or it can be forced through the ciruclar dependency flag
			bool b_nodeCanProcess;
			if(processedNode->b_circularDependencyFlag) b_nodeCanProcess = true;
			else if(instruction->processType == NodeGraphProcess::Direction::INPUT_PROCESS && processedNode->areAllLinkedInputNodesProcessed()) b_nodeCanProcess = true;
			else if(instruction->processType == NodeGraphProcess::Direction::OUTPUT_PROCESS && processedNode->areAllLinkedBidirectionalOutputNodesProcessed()) b_nodeCanProcess = true;
			else b_nodeCanProcess = false;
			
			//processed nodes get added to the ordered instruction list
			//else they get added to a candidate list to try again on the next round
			if(b_nodeCanProcess){
				instructionsOrdered.push_back(instruction);
				processedNode->b_wasProcessed = true;
			}else nextInstructionCanditates.push_back(instruction);
		}

		//if the canditate list didn't change, we are stuck in a cyclic dependency loop.
		//force update of the first node in the list on the next cycle to try and resolve the loop
		if (instructions == nextInstructionCanditates) nextInstructionCanditates.front()->processedNode->b_circularDependencyFlag = true;

		//swap the list of canditates with the nodes to be processed on the next round
		instructions.clear();
		instructions.swap(nextInstructionCanditates);
	}
	
	return instructionsOrdered;
}




//compiles a input process instructions and output process instructions, from a given list of start nodes
std::shared_ptr<NodeGraphProcess> NodeGraph::compileProcess(std::vector<std::shared_ptr<Node>>& startNodes){
	std::shared_ptr<NodeGraphProcess> program = std::make_shared<NodeGraphProcess>();
	
	program->inputProcessInstructions = compileOrderedInstructions(startNodes, NodeGraphProcess::Direction::INPUT_PROCESS);
	
	std::vector<std::shared_ptr<Node>> outputProcesStartedNodes;
	for(auto& instruction : program->inputProcessInstructions){
		if(instruction->processedNode->needsOutputProcess()) {
			outputProcesStartedNodes.push_back(instruction->processedNode);
		}
	}
	
	program->outputProcessInstructions = compileOrderedInstructions(outputProcesStartedNodes, NodeGraphProcess::Direction::OUTPUT_PROCESS);
		
	return program;
}




void NodeGraphProcess::log(){
	Logger::debug("Input Process: {} instructions", inputProcessInstructions.size());
	for(int i = 0; i < inputProcessInstructions.size(); i++){
		auto& instruction = inputProcessInstructions[i];
		switch(instruction->processType){
			case NodeGraphProcess::Direction::INPUT_PROCESS: Logger::trace("[{}] [Input Process] {}", i, instruction->processedNode->getName()); break;
			case NodeGraphProcess::Direction::OUTPUT_PROCESS: Logger::trace("[{}] [Output Process] {}", i, instruction->processedNode->getName()); break;
		}
	}
	Logger::debug("Output Process: {} instructions", outputProcessInstructions.size());
	for(int i = 0; i < outputProcessInstructions.size(); i++){
		auto& instruction = outputProcessInstructions[i];
		switch(instruction->processType){
			case NodeGraphProcess::Direction::INPUT_PROCESS: Logger::trace("[{}] [Input Process] {}", i, instruction->processedNode->getName()); break;
			case NodeGraphProcess::Direction::OUTPUT_PROCESS: Logger::trace("[{}] [Output Process] {}", i, instruction->processedNode->getName()); break;
		}
	}
}



void NodeGraph::executeInstructions(std::vector<std::shared_ptr<NodeGraphProcess::Instruction>>& instructions){
	for(auto& instruction : instructions){
		switch(instruction->processType){
			case NodeGraphProcess::Direction::INPUT_PROCESS: instruction->processedNode->inputProcess(); break;
			case NodeGraphProcess::Direction::OUTPUT_PROCESS: instruction->processedNode->outputProcess(); break;
		}
	}
}

void NodeGraph::executeInputProcess(std::shared_ptr<NodeGraphProcess> processProgram){
	executeInstructions(processProgram->inputProcessInstructions);
}
void NodeGraph::executeOutputProcess(std::shared_ptr<NodeGraphProcess> processProgram){
	executeInstructions(processProgram->outputProcessInstructions);
}

