#include <pch.h>

#include "ioNodeFactory.h"

namespace ioNodeFactory {

	std::shared_ptr<ioNode> getIoNodeByName(const char* name) {
		RETURN_NODE_IF_MATCHING(name, ClockNode);
		RETURN_NODE_IF_MATCHING(name, DisplayNode);
		RETURN_NODE_IF_MATCHING(name, AdditionNode);
		RETURN_NODE_IF_MATCHING(name, SubtractionNode);
		RETURN_NODE_IF_MATCHING(name, MultiplicationNode);
		RETURN_NODE_IF_MATCHING(name, DivisionNode);
		return nullptr;
	}

	std::vector<ioNode*> ioNodeList = {
		new ClockNode(),
		new DisplayNode(),
		new AdditionNode(),
		new SubtractionNode(),
		new MultiplicationNode(),
		new DivisionNode()
	};

	std::vector<ioNode*>& getIoNodeList() {
		return ioNodeList;
	}
}