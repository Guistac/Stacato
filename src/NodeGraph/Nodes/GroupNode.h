#pragma once

#include "NodeGraph/ioNode.h"

class GroupNode : public ioNode {
public:																	
	
	DEFINE_CONTAINER_NODE("Group", GroupNode, "Utility");

	virtual void process(bool) {}
	virtual void assignIoData() {}

};
