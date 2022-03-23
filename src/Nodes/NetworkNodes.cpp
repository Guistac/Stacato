#include <pch.h>

#include "NodeFactory.h"

#include "Networking/Osc/OscDevice.h"

namespace NodeFactory{

	std::vector<Node*> allNetworkNodes;

	void loadNetworkNodes(std::vector<Node*>& nodeList){
		allNetworkNodes = {
			new OscDevice()
		};
		nodeList.insert(nodeList.end(), allNetworkNodes.begin(), allNetworkNodes.end());
	}

	std::vector<Node*>& getAllNetworkNodes(){ return allNetworkNodes; }

}
