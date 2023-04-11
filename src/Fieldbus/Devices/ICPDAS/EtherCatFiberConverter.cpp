#include <pch.h>

#include "EtherCatFiberConverter.h"

void ECAT_2511_A_FiberConverter::onDisconnection() {}
void ECAT_2511_A_FiberConverter::onConnection() {}
void ECAT_2511_A_FiberConverter::onConstruction() {
	EtherCatDevice::onConstruction();
	setName("ECAT-2511-A RJ45 to Fiber EtherCAT Converter [A]");
}
bool ECAT_2511_A_FiberConverter::startupConfiguration() { return true; }
void ECAT_2511_A_FiberConverter::readInputs() {}
void ECAT_2511_A_FiberConverter::writeOutputs(){}
bool ECAT_2511_A_FiberConverter::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool ECAT_2511_A_FiberConverter::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void ECAT_2511_B_FiberConverter::onDisconnection() {}
void ECAT_2511_B_FiberConverter::onConnection() {}
void ECAT_2511_B_FiberConverter::onConstruction() {
	EtherCatDevice::onConstruction();
	setName("ECAT-2511-A Fiber to RJ45 EtherCAT Converter [B]");
}
bool ECAT_2511_B_FiberConverter::startupConfiguration() { return true; }
void ECAT_2511_B_FiberConverter::readInputs() {}
void ECAT_2511_B_FiberConverter::writeOutputs(){}
bool ECAT_2511_B_FiberConverter::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool ECAT_2511_B_FiberConverter::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void ECAT_2515_6PortJunction::onConstruction() {
	EtherCatDevice::onConstruction();
	setName("ECAT-2515 6-Port EtherCAT Junction");
}
bool ECAT_2515_6PortJunction::startupConfiguration() { return true; }
void ECAT_2515_6PortJunction::onDisconnection() {}
void ECAT_2515_6PortJunction::onConnection() {}
void ECAT_2515_6PortJunction::readInputs() {}
void ECAT_2515_6PortJunction::writeOutputs(){}
bool ECAT_2515_6PortJunction::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool ECAT_2515_6PortJunction::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

