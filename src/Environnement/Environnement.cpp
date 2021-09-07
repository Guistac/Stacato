#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatSlave.h"

NodeGraph Environnement::nodeGraph;

bool Environnement::hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == NodeType::IODEVICE && node->getDeviceType() == DeviceType::ETHERCATSLAVE) {
			std::shared_ptr<EtherCatSlave> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (slave->matches(otherDevice)) return true;
		}
	}
	return false;
}

std::shared_ptr<EtherCatSlave> Environnement::getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == NodeType::IODEVICE && node->getDeviceType() == DeviceType::ETHERCATSLAVE) {
			std::shared_ptr<EtherCatSlave> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(node);
			if (slave->matches(otherDevice)) return otherDevice;
		}
	}
	return nullptr;
}

void Environnement::setAllEtherCatSlavesOffline() {
	for (auto node : nodeGraph.getIoNodes()) {
		if (node->getType() == NodeType::IODEVICE && node->getDeviceType() == DeviceType::ETHERCATSLAVE) {
			std::shared_ptr<EtherCatSlave> device = std::dynamic_pointer_cast<EtherCatSlave>(node);
			device->setOnline(false);
		}
	}
}