#include <pch.h>

#include "NodeFactory.h"

#include "Nodes/Processors/ClockNode.h"
#include "Nodes/Processors/DisplayNode.h"
#include "Nodes/Processors/MathNodes.h"
#include "Nodes/Processors/GroupNode.h"
#include "Nodes/Processors/PlotterNode.h"

namespace NodeFactory{

	std::vector<Node*> allProcessorNodes;
	std::vector<NodeGroup> processorNodesByCategory;

	void loadProcessorNodes(std::vector<Node*>& nodeList){
		allProcessorNodes = {
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
			new ModuloNode(),
			new JoystickNode(),
			
			new SinusNode(),
			new CosinusNode(),
			new TangentNode(),
			new CotangentNode(),

			new BoolNode(),
			new NotNode(),
			new AndNode(),
			new OrNode()
		};
		
		//sort processor nodes by category
		for (Node* node : allProcessorNodes) {
			const char* category = node->getNodeCategory();
			bool categoryExists = false;
			for (NodeGroup& group : processorNodesByCategory) {
				if (strcmp(category, group.name) == 0) {
					categoryExists = true;
					group.nodes.push_back(node);
					break;
				}
			}
			if (!categoryExists) {
				processorNodesByCategory.push_back(NodeGroup());
				strcpy(processorNodesByCategory.back().name, node->getNodeCategory());
				processorNodesByCategory.back().nodes.push_back(node);
			}
		}

		nodeList.insert(nodeList.end(), allProcessorNodes.begin(), allProcessorNodes.end());
	}

	std::vector<NodeGroup>& getProcessorNodesByCategory() { return processorNodesByCategory; }

}
