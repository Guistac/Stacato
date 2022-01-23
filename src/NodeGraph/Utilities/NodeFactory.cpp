#include <pch.h>

#include "NodeFactory.h"

#include "NodeGraph/Nodes/ClockNode.h"
#include "NodeGraph/Nodes/DisplayNode.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/GroupNode.h"
#include "NodeGraph/Nodes/PlotterNode.h"

#include "Motion/Axis/VelocityControlledAxis.h"
#include "Motion/Axis/PositionControlledAxis.h"

#include "Motion/Machine/Machines/Basic/PositionControlledSingleAxisMachine.h"
#include "Motion/Machine/Machines/StateMachines/HoodedLiftStateMachine.h"
//#include "Motion/Machine/Machines/Animated/Oscillator3x.h"
//#include "Motion/Machine/Machines/Animated/BinaryOscillator6x.h"
#include "Motion/Machine/Machines/Utility/PositionFeedbackMachine.h"
#include "Motion/Machine/Machines/Special/LinearMecanumClaw.h"

#include "Motion/Machine/DeadMansSwitch.h"

#include "Networking/Osc/OscDevice.h"

#include "Motion/Adapters/GpioActuator.h"
#include "Motion/Adapters/GpioServoActuator.h"

namespace NodeFactory {

	std::vector<Node*> allNodes;
	std::vector<NodeGroup> nodesByCategory;

	std::vector<Node*> allAxisNodes;
	std::vector<Node*> allMachineNodes;
	std::vector<NodeGroup> machinesByCategory;
	std::vector<Node*> allSafetyNodes;

	std::vector<Node*> allNetworkIoNodes;

	void loadNodes() {

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

		allNodes = {
			new ClockNode(),

			new DisplayNode(),
			new PlotterNode(),
			new GroupNode(),
			new ConstantNode(),

			new AdditionNode(),
			new SubtractionNode(),
			new MultiplicationNode(),
			new DivisionNode(),
			new ExponentNode(),
			new AbsoluteNode(),
			new SinusNode(),
			new CosinusNode(),
			new TangentNode(),
			new CotangentNode(),

			new BoolNode(),
			new NotNode(),
			new AndNode(),
			new OrNode(),
			
			new GpioActuator(),
			new GpioServoActuator()
		};
		
		allSafetyNodes = {
			new DeadMansSwitch()
		};
		
		allNetworkIoNodes = {
			new OscDevice()
		};

		//sort devices by manufacturer
		for (Node* node : allNodes) {
			const char* category = node->getNodeCategory();
			bool categoryExists = false;
			for (NodeGroup& group : nodesByCategory) {
				if (strcmp(category, group.name) == 0) {
					categoryExists = true;
					group.nodes.push_back(node);
					break;
				}
			}
			if (!categoryExists) {
				nodesByCategory.push_back(NodeGroup());
				strcpy(nodesByCategory.back().name, node->getNodeCategory());
				nodesByCategory.back().nodes.push_back(node);
			}
		}

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


	}

	std::shared_ptr<Node> getNodeBySaveName(const char* saveName) {
		for (Node* device : allNodes) {
			if (strcmp(saveName, device->getSaveName()) == 0) return device->getNewInstance();
		}
		return nullptr;
	}

	std::vector<NodeGroup>& getNodesByCategory() {
		return nodesByCategory;
	}

	std::shared_ptr<Node> getAxisBySaveName(const char* saveName) {
		for (Node* axis : allAxisNodes) {
			if (strcmp(saveName, axis->getSaveName()) == 0) return axis->getNewInstance();
		}
		return nullptr;
	}
	std::vector<Node*>& getAllAxisTypes() {
		return allAxisNodes;
	}



	std::shared_ptr<Node> getMachineBySaveName(const char* saveName) {
		for (Node* machine : allMachineNodes) {
			if (strcmp(saveName, machine->getSaveName()) == 0) return machine->getNewInstance();
		}
		return nullptr;
	}
	std::vector<NodeGroup>& getMachinesByCategory() {
		return machinesByCategory;
	}

	std::shared_ptr<Node> getSafetyNodeBySaveName(const char* saveName){
		for(Node* safetyNode : allSafetyNodes){
			if(strcmp(saveName, safetyNode->getSaveName()) == 0) return safetyNode->getNewInstance();
		}
		return nullptr;
	}

	std::vector<Node*>& getAllSafetyNodes(){
		return allSafetyNodes;
	}

	std::shared_ptr<Node> getNetworkIoNodeBySaveName(const char* saveName){
		for(Node* networkIoNode : allNetworkIoNodes){
			if(strcmp(saveName, networkIoNode->getSaveName()) == 0) return networkIoNode->getNewInstance();
		}
		return nullptr;
	}

	std::vector<Node*>& getAllNetworkIoNodes(){
		return allNetworkIoNodes;
	}

}
