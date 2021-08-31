#pragma once

#include "nodeGraph/nodeGraph.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

class Environnement {
public:

	static bool add(EtherCatSlave* slave) {
		//don't allow adding a slave that is already there
		if (hasSlave(slave)) return false;
		//remove the slave we are adding from the available slave list
		std::vector<EtherCatSlave*>& availableSlaves = EtherCatFieldbus::slaves;
		for (int i = availableSlaves.size() - 1; i >= 0; i--) {
			if (availableSlaves[i] == slave) {
				availableSlaves.erase(availableSlaves.begin() + i);
			}
		}
		//add the slave to the environnement slave list
		etherCatSlaves.push_back(slave);
		//add the slave to the node graph
		nodeGraph.addIoNode(slave);
	}

	static bool hasSlave(EtherCatSlave* slave, EtherCatSlave** matchingSlave = nullptr) {
		for (EtherCatSlave* otherSlave : etherCatSlaves) {
			if (slave->matches(otherSlave)) {
				if (matchingSlave != nullptr) *matchingSlave = otherSlave;
				return true;
			}
		}
		if (matchingSlave != nullptr) *matchingSlave = nullptr;
		return false;
	}

	static std::vector<EtherCatSlave*> etherCatSlaves;

	static NodeGraph nodeGraph;

	//holds all devices and links
	//holds timeline data
	//handles saving and loading xml data
	//holds set geometry
};