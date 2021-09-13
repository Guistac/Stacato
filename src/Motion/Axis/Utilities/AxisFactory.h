#pragma once

#include "NodeGraph/ioNode.h"

namespace ioNodeFactory {

	struct ioNodeGroup {
		char name[128];
		std::vector<ioNode*> nodes;
	};

	void loadAxis();

	std::shared_ptr<ioNode> getAxisByName(const char* name);

	std::vector<ioNodeGroup>& getAxisTypes();
}