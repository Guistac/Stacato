#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatSlave.h"

namespace Environnement {

	NodeGraph nodeGraph;

	bool hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
		for (auto node : nodeGraph.getIoNodes()) {
			if (node->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(node);
				if (device->getDeviceType() == DeviceType::ETHERCATSLAVE) {
					std::shared_ptr<EtherCatSlave> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(device);
					if (slave->matches(otherDevice)) return true;
				}
			}
		}
		return false;
	}

	std::shared_ptr<EtherCatSlave> getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave> slave) {
		for (auto node : nodeGraph.getIoNodes()) {
			if (node->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(node);
				if (device->getDeviceType() == DeviceType::ETHERCATSLAVE) {
					std::shared_ptr<EtherCatSlave> otherDevice = std::dynamic_pointer_cast<EtherCatSlave>(device);
					if (slave->matches(otherDevice)) return otherDevice;
				}
			}
		}
		return nullptr;
	}

	char name[256] = "Default Environnement";

	void setName(const char* newName) {
		strcpy(name, newName);
		//TODO: change title of window
	}

	const char* getName() { return name; }

}