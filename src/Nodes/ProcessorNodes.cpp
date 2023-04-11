#include <pch.h>

#include "NodeFactory.h"

#include "Nodes/Processors/ClockNode.h"
#include "Nodes/Processors/DisplayNode.h"
#include "Nodes/Processors/MathNodes.h"
#include "Nodes/Processors/GroupNode.h"
#include "Nodes/Processors/PlotterNode.h"

namespace NodeFactory{

	std::vector<std::shared_ptr<Node>> allProcessorNodes;
	std::vector<NodeGroup> processorNodesByCategory;

	void loadProcessorNodes(std::vector<std::shared_ptr<Node>>& nodeList){
		allProcessorNodes = {
			ClockNode::createInstance(),

			DisplayNode::createInstance(),
			PlotterNode::createInstance(),
			GroupNode::createInstance(),
			ConstantNode::createInstance(),

			AdditionNode::createInstance(),
			SubtractionNode::createInstance(),
			MultiplicationNode::createInstance(),
			DivisionNode::createInstance(),
			ExponentNode::createInstance(),
			AbsoluteNode::createInstance(),
			SinusNode::createInstance(),
			CosinusNode::createInstance(),
			TangentNode::createInstance(),
			CotangentNode::createInstance(),

			BoolNode::createInstance(),
			NotNode::createInstance(),
			AndNode::createInstance(),
			OrNode::createInstance()
		};
		
		//sort processor nodes by category
		for (auto node : allProcessorNodes) {
			bool categoryExists = false;
			for (NodeGroup& group : processorNodesByCategory) {
				if (group.name == node->getNodeCategory()) {
					categoryExists = true;
					group.nodes.push_back(node);
					break;
				}
			}
			if (!categoryExists) {
				processorNodesByCategory.push_back(NodeGroup());
				processorNodesByCategory.back().name = node->getNodeCategory();
				processorNodesByCategory.back().nodes.push_back(node);
			}
		}

		nodeList.insert(nodeList.end(), allProcessorNodes.begin(), allProcessorNodes.end());
	}

	std::vector<NodeGroup>& getProcessorNodesByCategory() { return processorNodesByCategory; }

}
