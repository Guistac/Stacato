#include <pch.h>

#include "MicroFlexE190.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>


void MicroFlex_e190::onConnection() {}

void MicroFlex_e190::onDisconnection() {}

void MicroFlex_e190::initialize() {
	
	auto thisDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	axis = DS402Axis::make(thisDevice);
	
	axis->processDataConfiguration.operatingModes.cyclicSynchronousPosition = true;
	axis->processDataConfiguration.operatingModes.cyclicSynchronousVelocity = true;
	axis->processDataConfiguration.operatingModes.cyclicSynchronousTorque = true;
	axis->processDataConfiguration.errorCode = true;
	axis->processDataConfiguration.currentActualValue = true;
	axis->processDataConfiguration.digitalInputs = true;
	axis->processDataConfiguration.digitalOutputs = true;
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	
	axis->configureProcessData();
	
	txPdoAssignement.addEntry(0x4022, 0x1, 16, "Analog Input 0", &AI0);

	//For some reason we are aborting motion after first enable, How do you abort motion in microflex e190?
	
}






//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool MicroFlex_e190::startupConfiguration() {
	
	int16_t AX0_AbortMode_I16 = 0; //do not generator abort errors
	if(!writeSDO_U16(0x505E, 0x0, AX0_AbortMode_I16)) return false;
	
	//TODO: what are the posible values ??
	//if(!axis->setAbortConnectionOptionCode(0)) return false;
	
	//STOPMODE
	//0 _smCRASH_STOP
	//1 _smERROR_DECEL
	//2 _smDECEL
	if(!axis->setQuickstopOptionCode(0)) return false;
	
	//ABORTMODE
	//0 _emIGNORE
	//1 _emCRASH_STOP_DISABLE
	//2 _emCRASH_STOP
	//3 _emERROR_DECEL
	//4 [reserved]
	//5 _emCALL_HANDLER
	if(!axis->setShutdownOptionCode(0)) return false;
	
	//internal only, always 1
	//if(!axis->setHaltOptionCode(0)) return false;
	
	//FAULTREACITONACTIVEOPTIONCODE
	//-1 Manufacturer Specific
	//0 Disable
	//1 Slow Ramp (DECEL)
	//2 Quick ramp (ERRORDECEL)
	if(!axis->setFaultReactionOptionCode(0)) return false;
		
	//=============== PROCESS DATA ASSIGNEMENT ===============

	rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex());
	txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex());
	
	//=========================== TIMING AND SYNC CONFIGURATION ============================
	
	ec_dcsync0(getSlaveIndex(), true, EtherCatFieldbus::processInterval_milliseconds * 1000000.0, EtherCatFieldbus::processInterval_milliseconds * 500000.0);
	
	return true;
}



//======================= READING INPUTS =======================

void MicroFlex_e190::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
}


//====================== PREPARING OUTPUTS =====================

void MicroFlex_e190::writeOutputs() {
	
	
	if(!axis->isEnabled()){
		velocity = axis->getActualVelocity() / axisUnitsPerVel;
		position = axis->getActualPosition() / axisUnitsPerPos;
		axis->setPosition(position * axisUnitsPerPos);
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
		axis->setPosition(position * axisUnitsPerPos);
	}
	
	axis->updateControlWordBits(false, false, false, false, false);
	
	axis->updateOutput();
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
