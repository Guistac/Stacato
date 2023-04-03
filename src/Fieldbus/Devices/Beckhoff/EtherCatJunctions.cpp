#include <pch.h>

#include "EtherCatJunctions.h"

void CU1124::onDisconnection() {}
void CU1124::onConnection() {}
void CU1124::onConstruction() { EtherCatDevice::onConstruction(); }
bool CU1124::startupConfiguration() { return true; }
void CU1124::readInputs() {}
void CU1124::writeOutputs(){}
bool CU1124::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1124::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void CU1128::onDisconnection() {}
void CU1128::onConnection() {}
void CU1128::onConstruction() { EtherCatDevice::onConstruction(); }
bool CU1128::startupConfiguration() { return true; }
void CU1128::readInputs() {}
void CU1128::writeOutputs(){}
bool CU1128::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool CU1128::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

