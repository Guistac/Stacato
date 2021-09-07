#pragma once

#include "Environnement/NodeGraph/Nodes/ClockNode.h"
#include "Environnement/NodeGraph/Nodes/DisplayNode.h"
#include "Environnement/NodeGraph/Nodes/MathNodes.h"

#define RETURN_NODE_IF_MATCHING(name, className) if(strcmp(name, className::getNodeTypeNameStatic()) == 0) return std::make_shared<className>()

namespace ioNodeFactory {

	std::shared_ptr<ioNode> getIoNodeByName(const char* name);

	std::vector<ioNode*>& getIoNodeList();

}