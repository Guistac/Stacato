#include <pch.h>

#include "NodeFactory.h"

#include "Motion/Axis/VelocityControlledAxis.h"
#include "Motion/Axis/PositionControlledAxis.h"
#include "Machine/Machines/Basic/PositionControlledSingleAxisMachine.h"
#include "Machine/Machines/StateMachines/HoodedLiftStateMachine.h"
//#include "Motion/Machine/Machines/Animated/Oscillator3x.h"
//#include "Motion/Machine/Machines/Animated/BinaryOscillator6x.h"
#include "Machine/Machines/Utility/PositionFeedbackMachine.h"
#include "Machine/Machines/Special/LinearMecanumClaw.h"

#include "Machine/Machines/Safety/DeadMansSwitch.h"

#include "Motion/Adapters/GpioActuator.h"
#include "Motion/Adapters/ActuatorToServoActuator.h"

namespace NodeFactory{

	std::vector<Node*> allAxisNodes;
	std::vector<Node*> allMachineNodes;
	std::vector<NodeGroup> machinesByCategory;
	std::vector<Node*> allSafetyNodes;
	std::vector<Node*> allMotionAdapterNodes;

	void loadMotionNodes(std::vector<Node*>& nodeList){
		
		allAxisNodes = {
			new VelocityControlledAxis(),
			new PositionControlledAxis()
		};
		
		allMachineNodes = {
			new PositionControlledSingleAxisMachine(),
			new HoodedLiftStateMachine(),
			//new Oscillator3x(),
			//new BinaryOscillator6x(),
			new PositionFeedbackMachine(),
			new LinearMecanumClaw()
		};
		
		allSafetyNodes = {
			new DeadMansSwitch()
		};
		
		allMotionAdapterNodes = {
			new GpioActuator(),
			new ActuatorToServoActuator()
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
		nodeList.insert(nodeList.end(), allMotionAdapterNodes.begin(), allMotionAdapterNodes.end());
		
	}

	std::vector<Node*>& getAllAxisNodes() { return allAxisNodes; }
	std::vector<NodeGroup>& getMachinesByCategory() { return machinesByCategory; }
	std::vector<Node*>& getAllSafetyNodes(){ return allSafetyNodes; }
	std::vector<Node*>& getAllMotionAdapterNodes(){ return allMotionAdapterNodes; }

}
