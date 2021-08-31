#include <pch.h>
#include "NodeGraph.h"

#include "ioNode.h"
#include "ioData.h"
#include "ioLink.h"

void NodeGraph::addIoNode(ioNode* newIoNode) {
	newIoNode->parentNodeGraph = this;
	newIoNode->uniqueID = uniqueID++;
	ioNodeList.push_back(newIoNode);
	for (ioData* data : newIoNode->nodeInputData) {
		data->uniqueID = uniqueID++;
		ioDataList.push_back(data);
	}
	for (ioData* data : newIoNode->nodeOutputData) {
		data->uniqueID = uniqueID++;
		ioDataList.push_back(data);
	}
}

void NodeGraph::removeIoNode(ioNode* removedIoNode) {
	for (ioData* data : removedIoNode->nodeInputData) {
		for (ioLink* link : data->ioLinks) disconnect(link);
	}
	for (ioData* data : removedIoNode->nodeOutputData) {
		for (ioLink* link : data->ioLinks) disconnect(link);
	}
	for (int i = (int)removedIoNode->nodeInputData.size() - 1; i >= 0; i--) {
		removedIoNode->removeIoData(removedIoNode->nodeInputData[i]);
	}
	for (int i = (int)removedIoNode->nodeOutputData.size() - 1; i >= 0; i--) {
		removedIoNode->removeIoData(removedIoNode->nodeOutputData[i]);
	}
	for (int i = (int)ioNodeList.size() - 1; i >= 0; i--) {
		if (ioNodeList[i] == removedIoNode) {
			ioNodeList.erase(ioNodeList.begin() + i);
			break;
		}
	}
	delete removedIoNode;
}


bool NodeGraph::isConnectionValid(ioData* data1, ioData* data2) {
	//only allow connection between an input and an output
	if (data1->isInput() && data2->isInput()) return false;
	else if (data1->isOutput() && data2->isOutput()) return false;

	//check if the link already exists, don't allow duplicates
	for (ioLink* link : data1->ioLinks) {
		if (link->outputData == data2 || link->inputData == data2) return false;
	}

	//don't allow connecting two ioData pins on the same node
	if (data1->parentNode == data2->parentNode) return false;

	//if all checks pass, allow the connection
	return true;
}

ioLink* NodeGraph::connect(ioData* data1, ioData* data2) {
	if (!isConnectionValid(data1, data2)) return nullptr;
	ioLink* newIoLink = new ioLink();
	newIoLink->uniqueID = uniqueID++;
	newIoLink->inputData = data1->isInput() ? data1 : data2;
	newIoLink->outputData = data2->isOutput() ? data2 : data1;
	data1->ioLinks.push_back(newIoLink);
	data2->ioLinks.push_back(newIoLink);
	ioLinkList.push_back(newIoLink);
	return newIoLink;
}

void NodeGraph::disconnect(ioLink* removedIoLink) {
	std::vector<ioLink*>& inputDataLinks = removedIoLink->inputData->ioLinks;
	std::vector<ioLink*>& outputDataLinks = removedIoLink->outputData->ioLinks;
	for (int i = (int)inputDataLinks.size() - 1; i >= 0; i--) {
		if (inputDataLinks[i] == removedIoLink) {
			inputDataLinks.erase(inputDataLinks.begin() + i);
			break;
		}
	}
	for (int i = (int)outputDataLinks.size() - 1; i >= 0; i--) {
		if (outputDataLinks[i] == removedIoLink) {
			outputDataLinks.erase(outputDataLinks.begin() + i);
			break;
		}
	}
	for (int i = (int)ioLinkList.size() - 1; i >= 0; i--) {
		if (ioLinkList[i] == removedIoLink) {
			ioLinkList.erase(ioLinkList.begin() + i);
			break;
		}
	}
	delete removedIoLink;
}

ioNode* NodeGraph::getIoNode(int Id) {
	for (ioNode* node : ioNodeList) {
		if (Id == node->uniqueID) return node;
	}
	return nullptr;
}

ioData* NodeGraph::getIoData(int Id) {
	for (ioData* pin : ioDataList) {
		if (Id == pin->uniqueID) return pin;
	}
	return nullptr;
}

ioLink* NodeGraph::getIoLink(int Id) {
	for (ioLink* link : ioLinkList) {
		if (Id == link->uniqueID) return link;
	}
	return nullptr;
}