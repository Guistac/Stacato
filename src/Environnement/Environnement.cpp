#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatSlave.h"

NodeGraph Environnement::nodeGraph;

bool Environnement::hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == ioNode::NodeType::IODEVICE) {
			std::shared_ptr<DeviceNode> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (otherDevice->getDeviceType() == DeviceNode::DeviceType::ETHERCATSLAVE) {
				std::shared_ptr<EtherCatSlave> otherSlave = std::dynamic_pointer_cast<EtherCatSlave>(otherDevice);
				if (slave->matches(otherSlave)) return true;
			}
		}
	}
	return false;
}

std::shared_ptr<EtherCatSlave> Environnement::getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == ioNode::NodeType::IODEVICE) {
			std::shared_ptr<DeviceNode> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (otherDevice->getDeviceType() == DeviceNode::DeviceType::ETHERCATSLAVE) {
				std::shared_ptr<EtherCatSlave> otherSlave = std::dynamic_pointer_cast<EtherCatSlave>(otherDevice);
				if (slave->matches(otherSlave)) return otherSlave;
			}
		}
	}
	return nullptr;
}

void Environnement::setAllEtherCatSlavesOffline() {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == ioNode::NodeType::IODEVICE) {
			std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (device->getDeviceType() == DeviceNode::DeviceType::ETHERCATSLAVE) {
				device->setOnline(false);
			}
		}
	}
}