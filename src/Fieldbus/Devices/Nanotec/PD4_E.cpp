#include <pch.h>

#include "PD4_E.h"


bool PD4_E::isDeviceReady() { return false; }
void PD4_E::enable() {}
void PD4_E::disable() {}
bool PD4_E::isEnabled() { return false; }
void PD4_E::onDisconnection() {}
void PD4_E::onConnection() {}
void PD4_E::resetData() {}

void PD4_E::assignIoData() {
	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x6040, 0x0, 16, "DS402 Control Word", &DS402controlWord);
	rxPdoAssignement.addEntry(0x6060, 0x0, 8, "Operating Mode Control", &operatingModeControl);

	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6041, 0x0, 16, "DS402 Status Word", &DS402statusWord);
	txPdoAssignement.addEntry(0x6061, 0x0, 8, "Operating Mode Display", &operatingModeDisplay);
	txPdoAssignement.addEntry(0x6064, 0x0, 32, "Actual Position", &actualPosition);
}

bool PD4_E::startupConfiguration() {
	std::shared_ptr<EtherCatDevice> thisDevice = std::dynamic_pointer_cast<EtherCatDevice>(shared_from_this());
	if (!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) {
		Logger::warn("RxPDO assignement failed");
	}
	if (!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) {
		Logger::warn("TxPDO assignement failed");
	}
	return true;
}


void PD4_E::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
}

void PD4_E::prepareOutputs() {
	uint8_t* outputData = identity->outputs;
}

bool PD4_E::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool PD4_E::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }