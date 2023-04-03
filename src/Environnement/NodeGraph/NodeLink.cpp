#include <pch.h>

#include "NodeLink.h"
#include "NodePin.h"
#include "NodeGraph.h"

void NodeLink::disconnect(){
	std::vector<std::shared_ptr<NodeLink>>& inputDataLinks = inputData->nodeLinks;
	std::vector<std::shared_ptr<NodeLink>>& outputDataLinks = outputData->nodeLinks;

	auto thisLink = std::static_pointer_cast<NodeLink>(shared_from_this());
	
	for (int i = 0; i < inputDataLinks.size(); i++) {
		if (inputDataLinks[i] == thisLink) {
			inputDataLinks.erase(inputDataLinks.begin() + i);
			break;
		}
	}
	for (int i = 0; i < outputDataLinks.size(); i++) {
		if (outputDataLinks[i] == thisLink) {
			outputDataLinks.erase(outputDataLinks.begin() + i);
			break;
		}
	}
	
	std::vector<std::shared_ptr<NodeLink>>& links = nodeGraph->getLinks();
	for (int i = 0; i < links.size(); i++) {
		if (links[i] == thisLink) {
			links.erase(links.begin() + i);
			break;
		}
	}
		
	inputData->parentNode->onPinDisconnection(inputData);
	outputData->parentNode->onPinDisconnection(outputData);
	
	nodeGraph = nullptr;
	inputData = nullptr;
	outputData = nullptr;
	//ideally the link is referenced nowhere else and should free itself at the end of this function
};
