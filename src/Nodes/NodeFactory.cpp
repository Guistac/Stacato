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

	std::shared_ptr<Node> getNodeByClassName(std::string className){
		for (auto node : allNodes) {
			if (className == node->getClassName()) return node->duplicate();
		}
		return nullptr;
	}

}
