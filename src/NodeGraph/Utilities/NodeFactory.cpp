#include <pch.h>

#include "NodeFactory.h"

#include "NodeGraph/Nodes/ClockNode.h"
#include "NodeGraph/Nodes/DisplayNode.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/GroupNode.h"
#include "NodeGraph/Nodes/PlotterNode.h"

#include "Motion/Machine/SingleAxisMachine.h"
#include "Motion/Machine/StateMachine.h"

namespace NodeFactory {

	std::vector<Node*> allNodes;
	std::vector<NodeGroup> nodesByCategory;

	std::vector<Node*> allMachineNodes;

	void loadNodes() {

		allMachineNodes = {
			new SingleAxisMachine(),
			new StateMachine()
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

	std::shared_ptr<Node> getMachineBySaveName(const char* saveName) {
		for (Node* machine : allMachineNodes) {
			if (strcmp(saveName, machine->getSaveName()) == 0) return machine->getNewNodeInstance();
		}
	}

	std::vector<Node*>& getMachineTypes() {
		return allMachineNodes;
	}

}