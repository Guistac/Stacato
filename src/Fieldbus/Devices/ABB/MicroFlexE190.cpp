#include <pch.h>

#include "MicroFlexE190.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>


void MicroFlex_e190::onConnection() {}

void MicroFlex_e190::onDisconnection() {}

void MicroFlex_e190::initialize() {
	
	auto thisDevice = std::static_pointer_cast<EtherCatDevice>(shared_from_this());
	axis = DS402Axis::make(thisDevice);
	
	axis->processDataConfiguration.enableCyclicSynchronousPositionMode();
	axis->processDataConfiguration.enableCyclicSynchronousVelocityMode();
	axis->processDataConfiguration.enableHomingMode();
	axis->processDataConfiguration.errorCode = true;
	axis->processDataConfiguration.currentActualValue = true;
	axis->processDataConfiguration.digitalInputs = true;
	axis->processDataConfiguration.digitalOutputs = true;
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->configureProcessData();
	txPdoAssignement.addEntry(0x4022, 0x1, 16, "Analog Input 0", &AI0);
}






//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool MicroFlex_e190::startupConfiguration() {
	
	//———— General Drive Control
	
	//set drive control to DS402
	int16_t controlRefSource = 1;
	if(!writeSDO_S16(0x5002, 0x0, controlRefSource)) return false;
	
	//interpolation
	//TODO: how do we set this? it seem the drive works best at 10ms cycle time and stutters at smaller cycle times
	//if(!axis->setInterpolationTimePeriod(EtherCatFieldbus::processInterval_milliseconds)) return false;
	
	
	//WHY IS THIS NOT WORKING ?
	//6510.1 uint32 drive rated current, impossible to download a value (should be 6 amps)
	//we keep getting error codes 10000 (motion aborted) and 149 (incorrect reference source) on enable
	
	//todo:
	//current limiting
	//drive units
	
	//checkout:
	//6410.13 motor rated speed rpm
	//5027.1 int32 drive current ???
	
	
	
	//———— Current Limitation
	
	double maxCurrentPercentage = 100.0;
	
	//as .1% increments of drive rated current
	uint16_t maxCurrent = 10.0 * maxCurrentPercentage;
	if(!axis->setMaxCurrent(maxCurrent)) return false;
	
	
	//———— Error Reactions
	
	//ERRORDECEL
	uint32_t errorDeceleration = 100;
	if(!axis->setQuickstopDeceleration(errorDeceleration)) return false;
	
	//STOPMODE (Quickstop Reaction or stop input)
	//1 ERROR_DECEL
	if(!axis->setQuickstopOptionCode(1)) return false;
	
	//ABORTMODE (when leaving State Operation Enabled Manually?)
	//0 IGNORE (no error triggered)
	if(!axis->setShutdownOptionCode(0)) return false;
	
	//DS402 Fault Reaction
	//0 Disable
	//2 Quick ramp (ERRORDECEL)
	if(!axis->setFaultReactionOptionCode(0)) return false;
	
	//500D.0 LIMITMODE (when a limit signal is triggered)
	//500E.0 FOLERRORMODE (when following error happens)
	
	
	//———— PDO Assignement

	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	//———— Synchronisation
	
	uint32_t cycleTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1'000'000;
	uint32_t shiftTime_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 500'000;
	ec_dcsync0(getSlaveIndex(), true, cycleTime_nanoseconds, shiftTime_nanoseconds);
	
	return true;
}



//======================= READING INPUTS =======================

void MicroFlex_e190::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
	
	
	//Drive Status
	b_estop = !isStateNone() && !axis->hasVoltage();
	b_isReady = !isStateNone() && isStateOperational() && !b_estop && axis->isRemoteControlActive();
	b_isEnabled = axis->isEnabled() && !b_waitingForEnable && axis->isRemoteControlActive();
	
	//fault logging
	if(previousErrorCode != axis->getErrorCode()){
		if(axis->getErrorCode() == 0x0) Logger::info("Fault cleared.");
		else Logger::error("fault {:x} {}", axis->getErrorCode(), getErrorCodeString());
	}
	previousErrorCode = axis->getErrorCode();
	
	//status word:
	//b10 : Target Reached (quickstop, halt ???)
	//b12 :	homing mode 1 = homing attained
	//		cyclic modes 0 = target ignored
	//b13 : homing mode 1 = homing error
	//		cyclic position 1 = following error
	bool b_targetReached = axis->getOperatingModeSpeciricStatusWordBit_10();
	bool b_homingAttainedOrFollowingTarget = axis->getOperatingModeSpeciricStatusWordBit_12();
	bool b_homingErrorOrFollowingError = axis->getOperatingModeSpeciricStatusWordBit_13();
}


//====================== PREPARING OUTPUTS =====================

void MicroFlex_e190::writeOutputs() {
	
	long long now_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
	
	//handle enabling & disabling
	if(b_shouldDisable){
		b_shouldDisable = false;
		b_shouldEnable = false;
		b_waitingForEnable = false;
		axis->disable();
	}
	else if(b_shouldEnable){
		if(axis->hasFault()) {
			axis->doFaultReset();
		}else{
			b_shouldEnable = false;
			b_waitingForEnable = true;
			enableRequestTime_nanoseconds = now_nanoseconds;
			axis->enable();
		}
	}
	else if(b_waitingForEnable){
		static const long long maxEnableTime_nanoseconds = 100'000'000; //100ms
		if(axis->isEnabled()) {
			Logger::info("Drive Enabled");
			b_waitingForEnable = false;
		}
		else if(now_nanoseconds - enableRequestTime_nanoseconds > maxEnableTime_nanoseconds){
			Logger::warn("Enable request timed out");
			b_waitingForEnable = false;
			axis->disable();
		}
	}
	
	//fault handling
	if(axis->hasFault()){
		//auto clear fault during enable
		if(b_waitingForEnable) axis->doFaultReset();
		else axis->disable();
	}
	
	//control word:
	//b4 : 	Homing = Start homing
	//b11 : reset warnings
	bool b_startHoming = false;
	bool b_resetWarning = false;
	axis->setOperatingModeSpecificControlWorldBit_4(b_startHoming);
	axis->setManufacturerSpecificControlWordBit_11(b_resetWarning);
	
	
	
	
	
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
