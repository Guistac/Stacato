#pragma once

#include "NodeGraph/Node.h"

class GroupNode : public Node {
public:																	
	
	DEFINE_CONTAINER_NODE(GroupNode, "Group", "Group", "Utility");

	virtual void process() {}
	virtual void assignIoData() {}

};
