#pragma once

#include "Environnement/NodeGraph/Node.h"

class GroupNode : public Node {
public:																	
	
	DEFINE_NODE(GroupNode, "Group", "Group", Node::Type::CONTAINER , "Utility");
	
};

void GroupNode::initialize(){}
