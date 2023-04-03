#include <pch.h>

#include "NodeFactory.h"
#include "Environnement/NodeGraph/Node.h"

namespace NodeFactory {

	std::vector<std::shared_ptr<Node>> allNodes;

	void load() {
		loadEtherCatNodes(allNodes);
		loadMotionNodes(allNodes);
		loadNetworkNodes(allNodes);
		loadProcessorNodes(allNodes);
	}

	std::shared_ptr<Node> getNodeBySaveName(const char* saveName){
		for (auto node : allNodes) {
			if (strcmp(saveName, node->getSaveName()) == 0) return node->duplicate();
		}
		return nullptr;
	}

}
