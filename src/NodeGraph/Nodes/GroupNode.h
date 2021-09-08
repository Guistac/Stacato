#pragma once

#include "Environnement/NodeGraph/ioNode.h"

class GroupNode : public ioNode {
public:																	
	
	DEFINE_GROUP_NODE("Group", GroupNode);

	virtual void process(bool) {}
	virtual void assignIoData() {}

};
