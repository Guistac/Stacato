#include <pch.h>

#include "ioNode.h"

#include "NodeGraph.h"
#include "ioData.h"

void ioNode::addIoData(std::shared_ptr<ioData> ioData) {
	if (ioData->isInput()) nodeInputData.push_back(ioData);
	else if (ioData->isOutput()) nodeOutputData.push_back(ioData);

	//if a pins gets added after if the node is already on the editor, this handles everything
	if (parentNodeGraph) {
		ioData->uniqueID = parentNodeGraph->uniqueID++;
		parentNodeGraph->ioDataList.push_back(ioData);
	}
}

void ioNode::removeIoData(std::shared_ptr<ioData> removedIoData) {
	if (removedIoData->isInput()) {
		for (int i = (int)nodeInputData.size() - 1; i >= 0; i--) {
			if (nodeInputData[i] == removedIoData) {
				nodeInputData.erase(nodeInputData.begin() + i);
				break;
			}
		}
	}
	else if (removedIoData->isOutput()) {
		for (int i = (int)nodeOutputData.size() - 1; i >= 0; i--) {
			if (nodeOutputData[i] == removedIoData) {
				nodeOutputData.erase(nodeOutputData.begin() + i);
				break;
			}
		}
	}

	if (parentNodeGraph) {
		std::vector<std::shared_ptr<ioData>>& ioDataList = parentNodeGraph->ioDataList;
		for (int i = (int)ioDataList.size() - 1; i >= 0; i--) {
			if (ioDataList[i] == removedIoData) {
				ioDataList.erase(ioDataList.begin() + i);
			}
		}
	}
}

//check if all nodes linked to the inputs of this node were processed
bool ioNode::areAllLinkedInputNodesProcessed() {
	for (auto inputData : nodeInputData) {
		for (auto inputDataLink : inputData->getLinks()) {
			if (!inputDataLink->getInputData()->getNode()->wasProcessed()) return false;
		}
	}
	return true;
}