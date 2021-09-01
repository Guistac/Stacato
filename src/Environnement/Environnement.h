#pragma once

#include "nodeGraph/nodeGraph.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

class Environnement {
public:

	static bool add(std::shared_ptr<EtherCatSlave> slave) {
		//don't allow adding a slave that is already there
		std::shared_ptr<EtherCatSlave> dummy;
		if (hasSlave(slave, dummy)) return false;
		//remove the slave we are adding from the available slave list
		std::vector<std::shared_ptr<EtherCatSlave>>& availableSlaves = EtherCatFieldbus::slaves_unassigned;
		for (int i = availableSlaves.size() - 1; i >= 0; i--) {
			if (availableSlaves[i] == slave) {
				availableSlaves.erase(availableSlaves.begin() + i);
			}
		}
		//add the slave to the environnement slave list
		etherCatSlaves.push_back(slave);
		//add the slave to the node graph (as a raw pointer)
		nodeGraph.addIoNode(slave.get());
	}

	static bool hasSlave(std::shared_ptr<EtherCatSlave> slave, std::shared_ptr<EtherCatSlave>& matchingSlave) {
		for (auto otherSlave : etherCatSlaves) {
			if (slave->matches(otherSlave)) {
				matchingSlave = otherSlave;
				return true;
			}
		}
		matchingSlave = nullptr;
		return false;
	}

	static std::vector<std::shared_ptr<EtherCatSlave>> etherCatSlaves;

	static NodeGraph nodeGraph;

	//holds all devices and links
	//holds timeline data
	//handles saving and loading xml data
	//holds set geometry
};