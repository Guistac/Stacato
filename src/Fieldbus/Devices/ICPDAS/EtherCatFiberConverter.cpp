#include <pch.h>

#include "EtherCatFiberConverter.h"

void ECAT_2511_A_FiberConverter::onDisconnection() {}
void ECAT_2511_A_FiberConverter::onConnection() {}
bool ECAT_2511_A_FiberConverter::startupConfiguration() { return true; }
void ECAT_2511_A_FiberConverter::readInputs() {}
void ECAT_2511_A_FiberConverter::writeOutputs(){}

void ECAT_2511_B_FiberConverter::onDisconnection() {}
void ECAT_2511_B_FiberConverter::onConnection() {}
bool ECAT_2511_B_FiberConverter::startupConfiguration() { return true; }
void ECAT_2511_B_FiberConverter::readInputs() {}
void ECAT_2511_B_FiberConverter::writeOutputs(){}

bool ECAT_2515_6PortJunction::startupConfiguration() { return true; }
void ECAT_2515_6PortJunction::onDisconnection() {}
void ECAT_2515_6PortJunction::onConnection() {}
void ECAT_2515_6PortJunction::readInputs() {}
void ECAT_2515_6PortJunction::writeOutputs(){}

