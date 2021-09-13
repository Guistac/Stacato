#include <pch.h>
#include "NodeGraph.h"

#include "ioNode.h"
#include "ioData.h"
#include "ioLink.h"

void NodeGraph::addIoNode(std::shared_ptr<ioNode> newIoNode) {
	newIoNode->uniqueID = uniqueID;
	uniqueID++;
	ioNodeList.push_back(newIoNode);
	newIoNode->assignIoData(); //this tries to generate an ID and adds all data to the nodelist if a parent was specified, we don't want this so we add the parent afterwards
	for (std::shared_ptr<ioData> data : newIoNode->nodeInputData) {
		data->uniqueID = uniqueID;
		uniqueID++;
		data->parentNode = newIoNode;
		ioDataList.push_back(data);
	}
	for (std::shared_ptr<ioData> data : newIoNode->nodeOutputData) {
		data->uniqueID = uniqueID;
		uniqueID++;
		data->parentNode = newIoNode;
		ioDataList.push_back(data);
	}
	if (newIoNode->getType() == IODEVICE) {
		std::shared_ptr<DeviceNode> deviceNode = std::dynamic_pointer_cast<DeviceNode>(newIoNode);
		std::shared_ptr<ioData> devicePin = deviceNode->deviceLink;
		devicePin->uniqueID = uniqueID;
		uniqueID++;
		devicePin->parentNode = newIoNode;
		ioDataList.push_back(deviceNode->deviceLink);
	}
	newIoNode->parentNodeGraph = this;
	newIoNode->b_isInNodeGraph = true;
}

void NodeGraph::removeIoNode(std::shared_ptr<ioNode> removedIoNode) {
	for (auto data : removedIoNode->nodeInputData) {
		for (std::shared_ptr<ioLink> link : data->ioLinks) disconnect(link);
	}
	for (auto data : removedIoNode->nodeOutputData) {
		for (std::shared_ptr<ioLink> link : data->ioLinks) disconnect(link);
	}
	for (int i = (int)ioNodeList.size() - 1; i >= 0; i--) {
		if (ioNodeList[i] == removedIoNode) {
			ioNodeList.erase(ioNodeList.begin() + i);
			break;
		}
	}
	removedIoNode->b_isInNodeGraph = false;
}


bool NodeGraph::isConnectionValid(std::shared_ptr<ioData> data1, std::shared_ptr<ioData> data2) {
	//only allow connection between an input and an output
	if (data1->isInput() && data2->isInput()) return false;
	else if (data1->isOutput() && data2->isOutput()) return false;

	//don't allow multiple links on an input pin
	if (data1->isInput() && !data1->ioLinks.empty() && !data1->acceptsMultipleInputs()) return false;
	else if (data2->isInput() && !data2->ioLinks.empty() && !data2->acceptsMultipleInputs()) return false;

	//check if the link already exists, don't allow duplicates
	for (std::shared_ptr<ioLink> link : data1->ioLinks) {
		if (link->outputData == data2 || link->inputData == data2) return false;
	}

	//don't allow connecting two ioData pins on the same node
	if (data1->parentNode == data2->parentNode) return false;

	//if all checks pass, check if the data types are compatible to decide validity
	return data1->isDataTypeCompatible(data2);
}

std::shared_ptr<ioLink> NodeGraph::connect(std::shared_ptr<ioData> data1, std::shared_ptr<ioData> data2) {
	if (!isConnectionValid(data1, data2)) return nullptr;
	std::shared_ptr<ioLink> newIoLink = std::make_shared<ioLink>();
	newIoLink->uniqueID = uniqueID;
	uniqueID++;
	newIoLink->inputData = data1->isOutput() ? data1 : data2;
	newIoLink->outputData = data2->isInput() ? data2 : data1;
	data1->ioLinks.push_back(newIoLink);
	data2->ioLinks.push_back(newIoLink);
	ioLinkList.push_back(newIoLink);
	return newIoLink;
}

void NodeGraph::disconnect(std::shared_ptr<ioLink> removedIoLink) {
	std::vector<std::shared_ptr<ioLink>>& inputDataLinks = removedIoLink->inputData->ioLinks;
	std::vector<std::shared_ptr<ioLink>>& outputDataLinks = removedIoLink->outputData->ioLinks;

	std::shared_ptr<ioNode> updatedNode = removedIoLink->getOutputData()->getNode();

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

	evaluate(updatedNode);
}

std::shared_ptr<ioNode> NodeGraph::getIoNode(int Id) {
	for (std::shared_ptr<ioNode> node : ioNodeList) {
		if (Id == node->uniqueID) return node;
	}
	return nullptr;
}

std::shared_ptr<ioData> NodeGraph::getIoData(int Id) {
	for (std::shared_ptr<ioData> pin : ioDataList) {
		if (Id == pin->uniqueID) return pin;
	}
	return nullptr;
}

std::shared_ptr<ioLink> NodeGraph::getIoLink(int Id) {
	for (std::shared_ptr<ioLink> link : ioLinkList) {
		if (Id == link->uniqueID) return link;
	}
	return nullptr;
}