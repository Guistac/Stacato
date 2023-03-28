#include "KincoFD.h"

void KincoFD::onDisconnection() {}
void KincoFD::onConnection() {}
void KincoFD::initialize() {}
bool KincoFD::startupConfiguration() { return true; }
void KincoFD::readInputs() {}
void KincoFD::writeOutputs(){}
bool KincoFD::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool KincoFD::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }
