#include <pch.h>

#include "NodeFactory.h"

#include "Networking/Osc/OscDevice.h"
#include "Networking/Psn/PsnServer.h"
#include "Networking/ArtNet/ArtnetNode.h"

namespace NodeFactory{

	std::vector<Node*> allNetworkNodes;

	void loadNetworkNodes(std::vector<Node*>& nodeList){
		allNetworkNodes = {
			new OscDevice(),
			new PsnServer(),
			new ArtNetNode()
		};
		nodeList.insert(nodeList.end(), allNetworkNodes.begin(), allNetworkNodes.end());
	}

	std::vector<Node*>& getAllNetworkNodes(){ return allNetworkNodes; }

}
