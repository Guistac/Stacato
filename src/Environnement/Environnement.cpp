#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatSlave.h"

namespace Environnement {

	NodeGraph nodeGraph;

	std::vector<std::shared_ptr<EtherCatSlave>> getEtherCatSlaves() {
		std::vector<std::shared_ptr<EtherCatSlave>> output;
		for (auto node : nodeGraph.getNodes()) {
			if (node->getType() == NodeType::IODEVICE) {
				std::shared_ptr<DeviceNode> device = std::dynamic_pointer_cast<DeviceNode>(node);
				if (device->getDeviceType() == DeviceType::ETHERCATSLAVE) {
					std::shared_ptr<EtherCatSlave> otherSlave = std::dynamic_pointer_cast<EtherCatSlave>(device);
					output.push_back(otherSlave);
				}
			}
		}
		return output;
	}

	char name[256] = "Default Environnement";

	void setName(const char* newName) {
		strcpy(name, newName);
		//TODO: change title of window
	}

	const char* getName() { return name; }

}