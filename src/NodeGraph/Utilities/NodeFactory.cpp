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

	std::vector<Node*> machineTypes;

	void loadNodes() {
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

		machineTypes = {
			new SingleAxisMachine(),
			new StateMachine()
		};
	}

	std::shared_ptr<Node> getNodeByName(const char* name) {
		for (Node* device : allNodes) {
			if (strcmp(name, device->getNodeName()) == 0) return device->getNewNodeInstance();
		}
		return nullptr;
	}

	std::vector<NodeGroup>& getNodesByCategory() { return nodesByCategory; }

	std::shared_ptr<Node> getMachineByName(const char* name) {
		for (Node* machine : machineTypes) {
			if (strcmp(name, machine->getNodeName()) == 0) return machine->getNewNodeInstance();
		}
	}

	std::vector<Node*>& getMachineTypes() {
		return machineTypes;
	}

}