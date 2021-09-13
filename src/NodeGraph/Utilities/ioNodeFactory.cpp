#include <pch.h>

#include "ioNodeFactory.h"

#include "NodeGraph/Nodes/ClockNode.h"
#include "NodeGraph/Nodes/DisplayNode.h"
#include "NodeGraph/Nodes/MathNodes.h"
#include "NodeGraph/Nodes/GroupNode.h"
#include "NodeGraph/Nodes/PlotterNode.h"

#include "Motion/Axis/Axis.h"

namespace ioNodeFactory {

	std::vector<ioNode*> allNodes;
	std::vector<ioNodeGroup> nodesByCategory;

	std::vector<ioNode*> axisTypes;

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
		for (ioNode* node : allNodes) {
			const char* category = node->getNodeCategory();
			bool categoryExists = false;
			for (ioNodeGroup& group : nodesByCategory) {
				if (strcmp(category, group.name) == 0) {
					categoryExists = true;
					group.nodes.push_back(node);
					break;
				}
			}
			if (!categoryExists) {
				nodesByCategory.push_back(ioNodeGroup());
				strcpy(nodesByCategory.back().name, node->getNodeCategory());
				nodesByCategory.back().nodes.push_back(node);
			}
		}

		axisTypes = {
			new RotatingAxis(),
			new LinearAxis(),
			new StateMachineAxis()
		};
	}

	std::shared_ptr<ioNode> getIoNodeByName(const char* name) {
		for (ioNode* device : allNodes) {
			if (strcmp(name, device->getNodeName()) == 0) return device->getNewNodeInstance();
		}
		return nullptr;
	}

	std::vector<ioNodeGroup>& getNodesByCategory() { return nodesByCategory; }

	std::shared_ptr<ioNode> getAxisByName(const char* name) {
		for (ioNode* axis : axisTypes) {
			if (strcmp(name, axis->getNodeName()) == 0) return axis->getNewNodeInstance();
		}
	}

	std::vector<ioNode*>& getAxisTypes() {
		return axisTypes;
	}

}