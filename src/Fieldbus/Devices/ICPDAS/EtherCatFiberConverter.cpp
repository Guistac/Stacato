#include <pch.h>

#include "EtherCatFiberConverter.h"

void ECAT_2511_A_FiberConverter::onDisconnection() {}
void ECAT_2511_A_FiberConverter::onConnection() {}
void ECAT_2511_A_FiberConverter::initialize() {}
bool ECAT_2511_A_FiberConverter::startupConfiguration() { return true; }
void ECAT_2511_A_FiberConverter::readInputs() {}
void ECAT_2511_A_FiberConverter::writeOutputs(){}
bool ECAT_2511_A_FiberConverter::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool ECAT_2511_A_FiberConverter::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

void ECAT_2511_B_FiberConverter::onDisconnection() {}
void ECAT_2511_B_FiberConverter::onConnection() {}
void ECAT_2511_B_FiberConverter::initialize() {}
bool ECAT_2511_B_FiberConverter::startupConfiguration() { return true; }
void ECAT_2511_B_FiberConverter::readInputs() {}
void ECAT_2511_B_FiberConverter::writeOutputs(){}
bool ECAT_2511_B_FiberConverter::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool ECAT_2511_B_FiberConverter::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }

