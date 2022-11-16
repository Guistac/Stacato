#include <pch.h>

#include "MicroFlexE190.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>


void MicroFlex_e190::onConnection() {}

void MicroFlex_e190::onDisconnection() {}

void MicroFlex_e190::initialize() {
	
	axis.configure_CyclicSynchronousPosition = true;
	axis.configure_CyclicSynchronousVelocity = true;

	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	
	axis.configureProcessData(rxPdoAssignement, txPdoAssignement);
	
}






//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool MicroFlex_e190::startupConfiguration() {

	//=============== PROCESS DATA ASSIGNEMENT ===============

	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) return false;
	
	//=========================== TIMING AND SYNC CONFIGURATION ============================
	
	ec_dcsync0(getSlaveIndex(), true, EtherCatFieldbus::processInterval_milliseconds * 1000000.0, 0);
	
	return true;
}



//======================= READING INPUTS =======================

void MicroFlex_e190::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis.updateInputs();
}


//====================== PREPARING OUTPUTS =====================

void MicroFlex_e190::writeOutputs() {
	
	
	if(!axis.isEnabled()){
		velocity = axis.getActualVelocity() / axisUnitsPerVel;
		position = axis.getActualPosition() / axisUnitsPerPos;
		axis.setPosition(position * axisUnitsPerPos);
		//axis.setVelocity(velocity * axisUnitsPerVel);
	}else{
		double deltaT_s = EtherCatFieldbus::getCycleTimeDelta_seconds();
		double deltaV = acceleration * deltaT_s;
		if(manualVelocity > velocity) {
			velocity += deltaV;
			if(velocity > manualVelocity) velocity = manualVelocity;
		}
		else if(manualVelocity < velocity) {
			velocity -= deltaV;
			if(velocity < manualVelocity) velocity = manualVelocity;
		}
		velocity = std::clamp(velocity, -maxVelocity, maxVelocity);
		double deltaP = velocity * deltaT_s;
		position += deltaP;
		axis.setPosition(position * axisUnitsPerPos);
		//axis.setVelocity(velocity * axisUnitsPerVel);
	}
	
	
	
	axis.updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}



//============================= SAVING AND LOADING DEVICE DATA ============================

bool MicroFlex_e190::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
    return true;
}


bool MicroFlex_e190::loadDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    return true;
}
