#pragma once

#include "Environnement/Node.h"

class GroupNode : public Node {
public:																	
	
	DEFINE_NODE(GroupNode, "Group", "Group", Node::Type::CONTAINER , "Utility");
	
};

void GroupNode::initialize(){}
