#include <pch.h>

#include "Environnement.h"

#include "Fieldbus/EtherCatDevice.h"
#include "Motion/Machine/Machine.h"

namespace Environnement {

	NodeGraph nodeGraph;

	std::vector<std::shared_ptr<EtherCatDevice>> getEtherCatDevices() {
		std::vector<std::shared_ptr<EtherCatDevice>> output;
		for (auto node : nodeGraph.getNodes()) {
			if (node->getType() == Node::Type::IODEVICE) {
				std::shared_ptr<Device> device = std::dynamic_pointer_cast<Device>(node);
				if (device->getDeviceType() == Device::Type::ETHERCATSLAVE) {
					std::shared_ptr<EtherCatDevice> otherSlave = std::dynamic_pointer_cast<EtherCatDevice>(device);
					output.push_back(otherSlave);
				}
			}
		}
		return output;
	}

	std::vector<std::shared_ptr<Machine>> getMachines() {
		std::vector<std::shared_ptr<Machine>> output;
		for (auto node : nodeGraph.getNodes()) {
			if (node->getType() == Node::Type::MACHINE) {
				std::shared_ptr<Machine> machine = std::dynamic_pointer_cast<Machine>(node);
				output.push_back(machine);
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