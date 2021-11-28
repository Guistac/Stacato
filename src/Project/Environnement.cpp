#include <pch.h>

#include "Environnement.h"


#include "NodeGraph/NodeGraph.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Motion/Machine/Machine.h"

namespace Environnement {

	std::shared_ptr<NodeGraph> nodeGraph = std::make_shared<NodeGraph>();
	std::vector<std::shared_ptr<EtherCatDevice>> etherCatDevices;
	std::vector<std::shared_ptr<Machine>> machines;

	std::shared_ptr<Machine> selectedMachine;
	std::shared_ptr<EtherCatDevice> selectedEtherCatDevice;

	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices() {
		return etherCatDevices;
	}

	std::vector<std::shared_ptr<Machine>>& getMachines() {
		return machines;
	}

	void enableAllMachines() {
		for (auto machine : machines) {
			machine->enable();
		}
	}

	void disableAllMachines() {
		for (auto machine : machines) {
			machine->disable();
		}
	}

	bool areAllMachinesEnabled() {
		if (machines.empty()) return false;
		for (auto machine : machines) {
			if (!machine->isEnabled()) return false;
		}
		return true;
	}

	bool areNoMachinesEnabled() {
		for(auto machine : machines) {
			if (machine->isEnabled()) return false;
		}
		return true;
	}

	void addNode(std::shared_ptr<Node> node) {
		switch (node->getType()) {
			case Node::Type::MACHINE:
				machines.push_back(std::dynamic_pointer_cast<Machine>(node));
				break;
			case Node::Type::IODEVICE:{
				std::shared_ptr<Device> deviceNode = std::dynamic_pointer_cast<Device>(node);
				switch (deviceNode->getDeviceType()) {
					case Device::Type::ETHERCAT_DEVICE:
						etherCatDevices.push_back(std::dynamic_pointer_cast<EtherCatDevice>(deviceNode));
						break;
					case Device::Type::NETWORK_DEVICE:
						break;
					case Device::Type::USB_DEVICE:
						break;
				}
			}break;
			default:
				break;
		}
	}

	void removeNode(std::shared_ptr<Node> node){
		switch (node->getType()) {
			case Node::Type::MACHINE:{
				std::shared_ptr<Machine> machineNode = std::dynamic_pointer_cast<Machine>(node);
				for (int i = 0; i < machines.size(); i++) {
					if (machines[i] == machineNode) {
						machines.erase(machines.begin() + i);
						break;
					}
				}
			}break;
			case Node::Type::IODEVICE:{
				std::shared_ptr<Device> deviceNode = std::dynamic_pointer_cast<Device>(node);
				switch (deviceNode->getDeviceType()) {
				case Device::Type::ETHERCAT_DEVICE: {
					std::shared_ptr<EtherCatDevice> etherCatDeviceNode = std::dynamic_pointer_cast<EtherCatDevice>(deviceNode);
					for (int i = 0; i < etherCatDevices.size(); i++) {
						if (etherCatDevices[i] == etherCatDeviceNode) {
							etherCatDevices.erase(etherCatDevices.begin() + i);
							break;
						}
					}
				}break;
				case Device::Type::NETWORK_DEVICE: {
				}break;
				case Device::Type::USB_DEVICE: {
				}break;
				}
			}break;
			default:
				break;
		}
	}

	char name[256] = "Default Environnement";

	void setName(const char* newName) {
		strcpy(name, newName);
		//TODO: change title of window
	}

	const char* getName() { return name; }

}
