#include <pch.h>

#include "NodeFactory.h"

#include "Machine/Machines/Basic/PositionControlledMachine.h"
#include "Machine/Machines/Basic/VelocityControlledMachine.h"
#include "Machine/Machines/Basic/MultiAxisMachine.h"

#include "Machine/Machines/StateMachines/FlipStateMachine.h"
#include "Machine/Machines/StateMachines/AxisStateMachine.h"
#include "Machine/Machines/Special/MecanumMachine.h"

#include "Motion/Axis/AxisNode.h"

#include "Motion/Safety/DeadMansSwitch.h"
#include "Motion/Safety/SafetySignal.h"
#include "Motion/Safety/Brake.h"

namespace NodeFactory{

	std::vector<Node*> allAxisNodes;

	std::vector<Node*> allMachineNodes;
	std::vector<NodeGroup> machinesByCategory;

	std::vector<Node*> allSafetyNodes;

	void loadMotionNodes(std::vector<Node*>& nodeList){
		
		allAxisNodes = {
			new AxisNode()
		};
		
		allMachineNodes = {
			new PositionControlledMachine(),
			new VelocityControlledMachine(),
			new MultiAxisMachine(),
			new FlipStateMachine(),
			new AxisStateMachine(),
			new MecanumMachine()
		};
		
		allSafetyNodes = {
			new DeadMansSwitch(),
			new SafetySignal(),
			new Brake()
		};
		
		//sort machine nodes by category
		for (Node* node : allMachineNodes) {
		   const char* category = node->getNodeCategory();
		   bool categoryExists = false;
		   for (NodeGroup& group : machinesByCategory) {
			   if (strcmp(category, group.name) == 0) {
				   categoryExists = true;
				   group.nodes.push_back(node);
				   break;
			   }
		   }
		   if (!categoryExists) {
			   machinesByCategory.push_back(NodeGroup());
			   strcpy(machinesByCategory.back().name, node->getNodeCategory());
			   machinesByCategory.back().nodes.push_back(node);
		   }
	   }
		
		nodeList.insert(nodeList.end(), allMachineNodes.begin(), allMachineNodes.end());
		nodeList.insert(nodeList.end(), allAxisNodes.begin(), allAxisNodes.end());
		nodeList.insert(nodeList.end(), allSafetyNodes.begin(), allSafetyNodes.end());
		
	}

	std::vector<Node*>& getAllAxisNodes() { return allAxisNodes; }
	std::vector<NodeGroup>& getMachinesByCategory() { return machinesByCategory; }
	std::vector<Node*>& getAllSafetyNodes(){ return allSafetyNodes; }
}
