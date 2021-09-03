#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatSlave.h"

NodeGraph Environnement::nodeGraph;

bool Environnement::add(std::shared_ptr<EtherCatSlave> slave) {
	//don't allow adding a slave that is already there
	std::shared_ptr<EtherCatSlave> dummy;
	if (hasEtherCatSlave(slave)) return false;
	//add the slave to the node graph (as a raw pointer)
	nodeGraph.addIoNode(slave);
}

bool Environnement::hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == ioNode::NodeType::EtherCatSlave) {
			std::shared_ptr<EtherCatSlave> otherSlave = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (slave->matches(otherSlave)) return true;
		}
	}
	return false;
}

std::shared_ptr<EtherCatSlave> Environnement::getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == ioNode::NodeType::EtherCatSlave) {
			std::shared_ptr<EtherCatSlave> otherSlave = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (slave->matches(otherSlave)) return otherSlave;
		}
	}
	return nullptr;
}

void Environnement::setAllEtherCatSlavesOffline() {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == ioNode::NodeType::EtherCatSlave) {
			std::shared_ptr<EtherCatSlave> slave = std::dynamic_pointer_cast<EtherCatSlave>(node);
			slave->b_online = false;
		}
	}
}