#include "EL7221-9014-MotionInterface.h"

#include "Fieldbus/EtherCatFieldbus.h"

void EL7221_9014::onDisconnection() {}
void EL7221_9014::onConnection() {}
void EL7221_9014::initialize() {
	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x7010, 0x1, 16, "DRV Controlword", &controlWord);
	
	rxPdoAssignement.addNewModule(0x1608);
	rxPdoAssignement.addEntry(0x7010, 0x3, 8, "DRV Modes of operation", &modeOfOperationSelection);
	
	rxPdoAssignement.addNewModule(0x1606);
	rxPdoAssignement.addEntry(0x7010, 0x5, 32, "DRV Target position", &targetPosition);
	
	rxPdoAssignement.addNewModule(0x1601);
	rxPdoAssignement.addEntry(0x7010, 0x6, 32, "DRV Target Velocity", &targetVelocity);
	
	
	
	txPdoAssignement.addNewModule(0x1A01);
	txPdoAssignement.addEntry(0x6010, 0x1, 16, "DRV Statusword", &statusWord);
	
	txPdoAssignement.addNewModule(0x1A0E);
	txPdoAssignement.addEntry(0x6010, 0x3, 8, "DRV Modes of operation display", &modeOfOperationDisplay);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6000, 0x11, 32, "FB Position", &fbPosition);
	
	txPdoAssignement.addNewModule(0x1A06);
	txPdoAssignement.addEntry(0x6010, 0x6, 32, "DRV Following error actual value", &followingErrorActualValue);
	
	txPdoAssignement.addNewModule(0x1A02);
	txPdoAssignement.addEntry(0x6010, 0x7, 32, "DRV Velocity actual value", &velocityActualValue);
	
	txPdoAssignement.addNewModule(0x1A03);
	txPdoAssignement.addEntry(0x6010, 0x8, 16, "DRV Torque actual value", &torqueActualValue);
}

bool EL7221_9014::startupConfiguration() {
	
	rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12, false);
	txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13, false);
	
	double cycleTime_millis = EtherCatFieldbus::processInterval_milliseconds;
	uint32_t cycleTime_nanos = cycleTime_millis * 1000000;
	uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
	ec_dcsync01(getSlaveIndex(), true, cycleTime_nanos, 0, cycleOffset_nanos);
	
	return true;
}
void EL7221_9014::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
}
void EL7221_9014::writeOutputs(){
	rxPdoAssignement.pushDataTo(identity->outputs);
}
bool EL7221_9014::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL7221_9014::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }
