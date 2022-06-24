#include <pch.h>

#include "NodeFactory.h"
#include "Environnement/NodeGraph/Node.h"

namespace NodeFactory {

	std::vector<Node*> allNodes;

	void load() {
		loadEtherCatNodes(allNodes);
		loadMotionNodes(allNodes);
		loadNetworkNodes(allNodes);
		loadProcessorNodes(allNodes);
	}

	std::shared_ptr<Node> getNodeBySaveName(const char* saveName){
		for (Node* node : allNodes) {
			if (strcmp(saveName, node->getSaveName()) == 0) return node->getNewInstance();
		}
		return nullptr;
	}

}
