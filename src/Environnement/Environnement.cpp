#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatSlave.h"

namespace Environnement {

	NodeGraph nodeGraph;

	bool hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
		for (auto node : nodeGraph.getIoNodes()) {
			if (node->getType() == NodeType::IODEVICE && node->getDeviceType() == DeviceType::ETHERCATSLAVE) {
				std::shared_ptr<EtherCatSlave> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(node);
				if (slave->matches(otherDevice)) return true;
			}
		}
		return false;
	}

	std::shared_ptr<EtherCatSlave> getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
		for (auto node : nodeGraph.getIoNodes()) {
			if (node->getType() == NodeType::IODEVICE && node->getDeviceType() == DeviceType::ETHERCATSLAVE) {
				std::shared_ptr<EtherCatSlave> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(node);
				if (slave->matches(otherDevice)) return otherDevice;
			}
		}
		return nullptr;
	}

	void setAllEtherCatSlavesOffline() {
		for (auto node : nodeGraph.getIoNodes()) {
			if (node->getType() == NodeType::IODEVICE && node->getDeviceType() == DeviceType::ETHERCATSLAVE) {
				std::shared_ptr<EtherCatSlave> device = std::dynamic_pointer_cast<EtherCatSlave>(node);
				device->setOnline(false);
			}
		}
	}

	char name[256] = "Default Environnement";

	void setName(const char* newName) {
		strcpy(name, newName);
		//TODO: change title of window
	}

	const char* getName() { return name; }

}