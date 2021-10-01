#include <pch.h>

#include "NodeFactory.h"

#include "NodeGraph/Nodes/ClockNode.h"
#include "NodeGraph/Nodes/DisplayNode.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/GroupNode.h"
#include "NodeGraph/Nodes/PlotterNode.h"

#include "Motion/Axis/Axis.h"

namespace NodeFactory {

	std::vector<Node*> allNodes;
	std::vector<NodeGroup> nodesByCategory;

	std::vector<Node*> axisTypes;

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

		axisTypes = {
			new Axis(),
			new StateMachineAxis()
		};
	}

	std::shared_ptr<Node> getNodeByName(const char* name) {
		for (Node* device : allNodes) {
			if (strcmp(name, device->getNodeName()) == 0) return device->getNewNodeInstance();
		}
		return nullptr;
	}

	std::vector<NodeGroup>& getNodesByCategory() { return nodesByCategory; }

	std::shared_ptr<Node> getAxisByName(const char* name) {
		for (Node* axis : axisTypes) {
			if (strcmp(name, axis->getNodeName()) == 0) return axis->getNewNodeInstance();
		}
	}

	std::vector<Node*>& getAxisTypes() {
		return axisTypes;
	}

}