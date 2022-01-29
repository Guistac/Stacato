#pragma once

#include "NodeGraph/Node.h"

class GroupNode : public Node {
public:																	
	
	DEFINE_NODE(GroupNode, "Group", "Group", Node::Type::CONTAINER , "Utility");

	virtual void process(){}
	
};

void GroupNode::initialize(){}
