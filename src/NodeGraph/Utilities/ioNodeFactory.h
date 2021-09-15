#pragma once

#include "NodeGraph/ioNode.h"

namespace ioNodeFactory {

	struct ioNodeGroup {
		char name[128];
		std::vector<ioNode*> nodes;
	};

	void loadNodes();

	std::shared_ptr<ioNode> getIoNodeByName(const char* name);
	
	std::vector<ioNodeGroup>& getNodesByCategory();

	std::shared_ptr<ioNode> getAxisByName(const char* name);
	std::vector<ioNode*>& getAxisTypes();
}