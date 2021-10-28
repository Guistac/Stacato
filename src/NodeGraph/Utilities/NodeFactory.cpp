#include <pch.h>

#include "NodeFactory.h"

#include "NodeGraph/Nodes/ClockNode.h"
#include "NodeGraph/Nodes/DisplayNode.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/GroupNode.h"
#include "NodeGraph/Nodes/PlotterNode.h"

#include "Motion/Axis/PositionControlledAxis.h"

#include "Motion/Machine/Machines/Basic/SingleAxisMachine.h"
#include "Motion/Machine/Machines/StateMachines/HoodedLiftStateMachine.h"

namespace NodeFactory {

	std::vector<Node*> allNodes;
	std::vector<NodeGroup> nodesByCategory;

	std::vector<Node*> allAxisNodes;
	std::vector<Node*> allMachineNodes;

	void loadNodes() {

		allAxisNodes = {
			new PositionControlledAxis()
		};

		allMachineNodes = {
			new SingleAxisMachine(),
			new HoodedLiftStateMachine()
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
			new OrNode()
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
	}

	std::shared_ptr<Node> getNodeBySaveName(const char* saveName) {
		for (Node* device : allNodes) {
			if (strcmp(saveName, device->getSaveName()) == 0) return device->getNewNodeInstance();
		}
		return nullptr;
	}

	std::vector<NodeGroup>& getNodesByCategory() { return nodesByCategory; }

	std::shared_ptr<Node> getAxisBySaveName(const char* saveName) {
		for (Node* axis : allAxisNodes) {
			if (strcmp(saveName, axis->getSaveName()) == 0) return axis->getNewNodeInstance();
		}
		return nullptr;
	}
	
	std::vector<Node*>& getAxisTypes() {
		return allAxisNodes;
	}


	std::shared_ptr<Node> getMachineBySaveName(const char* saveName) {
		for (Node* machine : allMachineNodes) {
			if (strcmp(saveName, machine->getSaveName()) == 0) return machine->getNewNodeInstance();
		}
		return nullptr;
	}
	std::vector<Node*>& getMachineTypes() {
		return allMachineNodes;
	}

}