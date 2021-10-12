#pragma once

#include "NodeGraph/Node.h"

namespace NodeFactory {

	struct NodeGroup {
		char name[128];
		std::vector<Node*> nodes;
	};

	void loadNodes();

	std::shared_ptr<Node> getNodeByName(const char* name);
	
	std::vector<NodeGroup>& getNodesByCategory();

	std::shared_ptr<Node> getMachineByName(const char* name);
	std::vector<Node*>& getMachineTypes();
}