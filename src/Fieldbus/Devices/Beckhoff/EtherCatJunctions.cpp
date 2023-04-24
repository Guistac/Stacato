#include <pch.h>

#include "EtherCatJunctions.h"

void CU1124::onDisconnection() {}
void CU1124::onConnection() {}
void CU1124::initialize() {}
bool CU1124::startupConfiguration() { return true; }
void CU1124::readInputs() {}
void CU1124::writeOutputs(){}
bool CU1124::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1124::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void CU1128::onDisconnection() {}
void CU1128::onConnection() {}
void CU1128::initialize() {}
bool CU1128::startupConfiguration() { return true; }
void CU1128::readInputs() {}
void CU1128::writeOutputs(){}
bool CU1128::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1128::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EK1100::onDisconnection() {}
void EK1100::onConnection() {}
void EK1100::initialize() {}
bool EK1100::startupConfiguration() { return true; }
void EK1100::readInputs() {}
void EK1100::writeOutputs(){}
bool EK1100::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EK1100::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EL2008::onDisconnection() {}
void EL2008::onConnection() {}
void EL2008::initialize() {
	rxPdoAssignement.addNewModule(0x1600);
	for(int i = 0; i < 8; i++){
		char name[32];
		snprintf(name, 32, "Channel %i", i + 1);
		rxPdoAssignement.addEntry(0x1600 + i, 0x1, 1, name, &outputs[i]);
	}
}
bool EL2008::startupConfiguration() { return true; }
void EL2008::readInputs() {}
void EL2008::writeOutputs(){
	rxPdoAssignement.pushDataTo(identity->outputs);
}
bool EL2008::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL2008::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void EL5001::onDisconnection() {}
void EL5001::onConnection() {}
void EL5001::initialize() {
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x3101, 0x1, 8, "Status", &status);
	txPdoAssignement.addEntry(0x3101, 0x2, 32, "Value", &ssiValue);
}
bool EL5001::startupConfiguration() { return true; }
void EL5001::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
}
void EL5001::writeOutputs(){}
bool EL5001::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL5001::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }
