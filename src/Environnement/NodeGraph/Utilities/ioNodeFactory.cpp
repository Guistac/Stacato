#include <pch.h>

#include "ioNodeFactory.h"

#include "Environnement/NodeGraph/Nodes/ClockNode.h"
#include "Environnement/NodeGraph/Nodes/DisplayNode.h"
#include "Environnement/NodeGraph/Nodes/MathNodes.h"
#include "Environnement/NodeGraph/Nodes/GroupNode.h"

namespace ioNodeFactory {

	std::shared_ptr<ioNode> getIoNodeByName(const char* name) {
		RETURN_NODE_IF_MATCHING(name, ClockNode);
		RETURN_NODE_IF_MATCHING(name, DisplayNode);
		RETURN_NODE_IF_MATCHING(name, AdditionNode);
		RETURN_NODE_IF_MATCHING(name, SubtractionNode);
		RETURN_NODE_IF_MATCHING(name, MultiplicationNode);
		RETURN_NODE_IF_MATCHING(name, DivisionNode);
		RETURN_NODE_IF_MATCHING(name, SinusNode);
		RETURN_NODE_IF_MATCHING(name, CosinusNode);
		RETURN_NODE_IF_MATCHING(name, GroupNode);
		return nullptr;
	}

	std::vector<ioNode*> ioNodeList = {
		new ClockNode(),
		new DisplayNode(),
		new AdditionNode(),
		new SubtractionNode(),
		new MultiplicationNode(),
		new DivisionNode(),
		new SinusNode(),
		new CosinusNode(),
		new GroupNode()
	};

	std::vector<ioNode*>& getIoNodeList() {
		return ioNodeList;
	}
}