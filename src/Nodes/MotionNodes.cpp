#include <pch.h>

#include "NodeFactory.h"

#include "Machine/Machines/Basic/PositionControlledMachine.h"

//#include "Machine/Machines/StateMachines/FlipStateMachine.h"
//#include "Machine/Machines/StateMachines/AxisStateMachine.h"

#include "Motion/Axis/AxisNode.h"

#include "Motion/Safety/DeadMansSwitch.h"
#include "Motion/Safety/SafetySignal.h"
#include "Motion/Safety/Brake.h"

namespace NodeFactory{

	std::vector<std::shared_ptr<Node>> allAxisNodes;
	std::vector<std::shared_ptr<Node>> allMachineNodes;
	std::vector<std::shared_ptr<Node>> allSafetyNodes;

	std::vector<NodeGroup> machinesByCategory;

	void loadMotionNodes(std::vector<std::shared_ptr<Node>>& nodeList){
		
		allAxisNodes = {
			AxisNode::createInstance()
		};
		
		allMachineNodes = {
			//new PositionControlledMachine(),
			//new FlipStateMachine(),
			PositionControlledMachine::createInstance()
		};
		
		allSafetyNodes = {
			DeadMansSwitch::createInstance(),
			SafetySignal::createInstance(),
			//new Brake()
		};
		
		//sort machine nodes by category
		for (auto node : allMachineNodes) {
		   bool categoryExists = false;
		   for (NodeGroup& group : machinesByCategory) {
			   if (group.name == node->getNodeCategory()) {
				   categoryExists = true;
				   group.nodes.push_back(node);
				   break;
			   }
		   }
		   if (!categoryExists) {
			   machinesByCategory.push_back(NodeGroup());
			   machinesByCategory.back().name = node->getNodeCategory();
			   machinesByCategory.back().nodes.push_back(node);
		   }
	   }
		
		nodeList.insert(nodeList.end(), allMachineNodes.begin(), allMachineNodes.end());
		nodeList.insert(nodeList.end(), allAxisNodes.begin(), allAxisNodes.end());
		nodeList.insert(nodeList.end(), allSafetyNodes.begin(), allSafetyNodes.end());
		
	}

	std::vector<NodeGroup>& getMachinesByCategory() { return machinesByCategory; }
	std::vector<std::shared_ptr<Node>>& getAllAxisNodes() { return allAxisNodes; }
	std::vector<std::shared_ptr<Node>>& getAllSafetyNodes(){ return allSafetyNodes; }

}
