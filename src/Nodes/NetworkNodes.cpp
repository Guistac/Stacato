#include <pch.h>

#include "NodeFactory.h"

#include "Networking/Osc/OscDevice.h"
#include "Networking/Psn/PsnServer.h"
#include "Networking/ArtNet/ArtnetNode.h"

namespace NodeFactory{

	std::vector<std::shared_ptr<Node>> allNetworkNodes;

	void loadNetworkNodes(std::vector<std::shared_ptr<Node>>& nodeList){
		allNetworkNodes = {
			OscDevice::createInstance(),
			PsnServer::createInstance(),
			ArtNetNode::createInstance()
		};
		nodeList.insert(nodeList.end(), allNetworkNodes.begin(), allNetworkNodes.end());
	}

	std::vector<std::shared_ptr<Node>>& getAllNetworkNodes(){ return allNetworkNodes; }

}
