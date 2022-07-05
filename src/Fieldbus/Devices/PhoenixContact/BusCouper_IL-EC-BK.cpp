#include <pch.h>

#include "BusCouper_IL-EC-BK.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>


void IL_EC_BK_BusCoupler::onConnection() {}

void IL_EC_BK_BusCoupler::onDisconnection() {}

void IL_EC_BK_BusCoupler::initialize() {

	/*
	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x6040, 0x0, 16, "AX0_ControlWord_U16", &ds402Control.controlWord);
	rxPdoAssignement.addEntry(0x6060, 0x0, 8,  "AX0_ModesOfOperation_I8", &ds402Control.operatingMode);
	rxPdoAssignement.addEntry(0x607A, 0x0, 32, "AX0_TargetPosition_I32", &targetPosition);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6041, 0x0, 16, "AX0_StatusWord_U16", &ds402Status.statusWord);
	txPdoAssignement.addEntry(0x6060, 0x0, 8,  "AX0_ModesOfOperationDisplay_I8", &ds402Status.operatingMode);
	txPdoAssignement.addEntry(0x6064, 0x0, 32, "AX0_ActualPosition_I32", &actualPosition);
	*/
	 
}






//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool IL_EC_BK_BusCoupler::startupConfiguration() {

	/*
	//=============== PROCESS DATA ASSIGNEMENT ===============

	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) return false;
	
	//=========================== TIMING AND SYNC CONFIGURATION ============================
	
	ec_dcsync0(getSlaveIndex(), true, EtherCatFieldbus::processInterval_milliseconds * 1000000.0, 0);
	*/
	 
	return true;
}



//======================= READING INPUTS =======================

void IL_EC_BK_BusCoupler::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	/*
	actualPowerState = ds402Status.getPowerState();
	actualOperatingMode = ds402Status.getOperatingMode();
	 */
}


//====================== PREPARING OUTPUTS =====================

void IL_EC_BK_BusCoupler::writeOutputs() {
	/*
	ds402Control.setPowerState(requestedPowerState, actualPowerState);
	ds402Control.setOperatingMode(requestedOperatingMode);
	ds402Control.updateControlWord();
	*/
	rxPdoAssignement.pushDataTo(identity->outputs);
}



//============================= SAVING AND LOADING DEVICE DATA ============================

bool IL_EC_BK_BusCoupler::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
    return true;
}


bool IL_EC_BK_BusCoupler::loadDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    return true;
}
