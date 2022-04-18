#include "ATV320.h"

void ATV320::onConnection() {
}

void ATV320::onDisconnection() {
}

void ATV320::initialize() {
}

//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool ATV320::startupConfiguration() {
	return true;
}

//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void ATV320::readInputs() {
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV320::prepareOutputs() {
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV320::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}

bool ATV320::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}

