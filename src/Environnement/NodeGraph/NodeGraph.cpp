#include <pch.h>
#include "NodeGraph.h"

void NodeGraph::addNode(std::shared_ptr<Node> newNode) {
	if(newNode->uniqueID == -1) newNode->uniqueID = getNewUniqueID();
	newNode->nodeGraph = std::static_pointer_cast<NodeGraph>(shared_from_this());
	nodes.push_back(newNode);
	for (std::shared_ptr<NodePin> data : newNode->nodeInputPins) {
		data->uniqueID = getNewUniqueID();
		data->parentNode = newNode;
		pins.push_back(data);
	}
	for (std::shared_ptr<NodePin> data : newNode->nodeOutputPins) {
		data->uniqueID = getNewUniqueID();
		data->parentNode = newNode;
		pins.push_back(data);
	}
	newNode->onAddToNodeGraph();
	nodeAddCallback(newNode);
}

void NodeGraph::removeNode(std::shared_ptr<Node> removedNode) {
	removedNode->nodeGraph = nullptr;
	for (auto data : removedNode->nodeInputPins) {
		for (std::shared_ptr<NodeLink> link : data->nodeLinks) link->disconnect();
	}
	for (auto data : removedNode->nodeOutputPins) {
		for (std::shared_ptr<NodeLink> link : data->nodeLinks) link->disconnect();
	}
	for (int i = (int)nodes.size() - 1; i >= 0; i--) {
		if (nodes[i] == removedNode) {
			nodes.erase(nodes.begin() + i);
			break;
		}
	}
	for (int i = (int)selectedNodes.size() - 1; i >= 0; i--) {
		if (selectedNodes[i] == removedNode) {
			selectedNodes.erase(selectedNodes.begin() + i);
		}
	}
	removedNode->onRemoveFromNodeGraph();
	nodeRemoveCallback(removedNode);
}

std::shared_ptr<Node> NodeGraph::getNode(int Id) {
	for (std::shared_ptr<Node> node : nodes) {
		if (Id == node->uniqueID) return node;
	}
	return nullptr;
}

std::shared_ptr<NodePin> NodeGraph::getPin(int Id) {
	for (std::shared_ptr<NodePin> pin : pins) {
		if (Id == pin->uniqueID) return pin;
	}
	return nullptr;
}

std::shared_ptr<NodeLink> NodeGraph::getLink(int Id) {
	for (std::shared_ptr<NodeLink> link : links) {
		if (Id == link->uniqueID) return link;
	}
	return nullptr;
}

