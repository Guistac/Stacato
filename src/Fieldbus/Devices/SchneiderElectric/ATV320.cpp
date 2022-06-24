#include "ATV320.h"

void ATV320::onConnection() {
}

void ATV320::onDisconnection() {
}

void ATV320::initialize() {
	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x6040, 0x0, 16, "ControlWorld", &ds402Control.controlWord);
	rxPdoAssignement.addEntry(0x6042, 0x0, 16, "VelocityTarget", &velocityTarget);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6041, 0x0, 16, "StatusWord", &ds402SStatus.statusWord);
	txPdoAssignement.addEntry(0x6044, 0x0, 16, "VelocityActual", &velocityActual);
	txPdoAssignement.addEntry(0x2016, 0x3, 16, "LogicInputs", &logicInputs);
}

//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool ATV320::startupConfiguration() {
	
	//———————————— MOTOR SETTINGS ———————————
	
	//set motor frequency to 50Hz (0 = 50Hz / 1 = 60Hz)
	uint16_t standartMotorFrequency = 0;
	if(!writeSDO_U16(0x2000, 0x10, standartMotorFrequency)) return false;
	
	//rated motor voltage in 1V increments
	uint16_t ratedMotorVoltage = 400;
	if(writeSDO_U16(0x2042, 0x2, ratedMotorVoltage)) return false;
	
	//rated motor power in 0.1Kw increments
	uint16_t ratedMotorPower = 75;
	if(!writeSDO_U16(0x2042, 0xE, ratedMotorPower)) return false;
	
	//rate motor current in 0.1A increments
	uint16_t ratedMotorCurrent = 35;
	if(!writeSDO_U16(0x2042, 0x4, ratedMotorCurrent)) return false;
	
	//nominal rate motor speed in rpm
	uint16_t nominalMotorSpeed = 1400;
	if(!writeSDO_U16(0x2042, 0x5, nominalMotorSpeed)) return false;

	//set motor control type to standart (Standart aynchronous motor control = 3)
	uint16_t motorControlType = 3;
	if(!writeSDO_U16(0x2042, 0x8, motorControlType)) return false;
	
	
	//————————— CONTROL SETTINGS —————————————
	
	//set max output frequency to 50Hz (in 0.1Hz increments)
	uint16_t maxOutputFrequency = 500;
	if(!writeSDO_U16(0x2001, 0x4, maxOutputFrequency)) return false;
	
	//set minimum control speed in hertz to 0Hz (in .1Hz increments)
	uint16_t lowSpeed = 0;
	if(!writeSDO_U16(0x2001, 0x6, lowSpeed)) return false;
	
	//set maximum control speed to in 0.1Hz increments
	uint16_t highSpeed = 500;
	if(!writeSDO_U16(0x2001, 0x5, highSpeed)) return false;
	
	//set drive switching frequency to 16 Khz (max)
	uint16_t switchingFrequency = 160;
	if(!writeSDO_U16(0x2001, 0x3, switchingFrequency)) return false;
	
	
	//—————————————— IO SETTINGS ———————————————
	
	//set terminal block control to Two Wire (0 = 2-Wire / 1 = 3-Wire)
	uint16_t twoOrThreeWireControl = 0;
	if(!writeSDO_U16(0x2051, 0x2, twoOrThreeWireControl)) return false;
	
	//2-Wire Control level detection is -Signal Transition-
	uint16_t twoWireControlType = 1;
	if(!writeSDO_U16(0x2051, 0x3, twoWireControlType)) return false;
	
	//set Forward input to Logic Input 1 (LI1)
	uint16_t forwardInputAssignement = 129;
	if(!writeSDO_U16(0x2051, 0x5, forwardInputAssignement)) return false;
	
	//set reverse input to Logic Input 2 (LI2)
	uint16_t reverseInputAssignement = 130;
	if(!writeSDO_U16(0x2051, 0x6, reverseInputAssignement)) return false;
	
	//set fault reset input assignement to pin LI5
	uint16_t faultResetInputAssignement = 133;
	if(!writeSDO_U16(0x2029, 0x19, faultResetInputAssignement)) return false;
	
	//set method of switching between frequency references (LI3 = 131)
	uint16_t referenceSwitchingInputAssignement = 131;
	if(!writeSDO_U16(0x2036, 0xC, referenceSwitchingInputAssignement)) return false;
	
	//set external fault pin to Logic Input 6 (LI6 = 134)
	uint16_t externalFaultInputAssignement = 134;
	if(!writeSDO_U16(0x2029, 0x20, externalFaultInputAssignement)) return false;
	
	//set preset slowdown speed input assignement to pin LI4 = 132
	uint16_t presetSpeed2InputAssignement = 132;
	if(!writeSDO_U16(0x2054, 0x2, presetSpeed2InputAssignement)) return false;
	   
	//set preset slowdown speed to 5Hz (in .1Hz Increments)
	uint16_t presetSpeed2Frequency = 50;
	if(!writeSDO_U16(0x2054, 0xB, presetSpeed2Frequency)) return false;
	
	
	//—————————————— RAMP SETTINGS ——————————————
	
	//set ramp unit increment to hundreds of seconds (.01s = 0  / .1s = 1  /  1.s = 2)
	uint16_t rampIncrement = 0;
	if(!writeSDO_U16(0x203C, 0x15, rampIncrement)) return false;
	
	//set acceleration time
	uint16_t accelerationTime = accelerationTime_seconds * 100.0;
	if(!writeSDO_U16(0x203C, 0x2, accelerationTime)) return false;
	
	//set deceleration time
	uint16_t decelerationTime = decelerationTime_seconds * 100.0;
	if(!writeSDO_U16(0x203C, 0x3, decelerationTime)) return false;
	
	
	//——————————————— Network Control Settings ——————————
	
	//set frequency reference 1 to Analog Input 1 (AI1)
	uint16_t reference1 = 1;
	if(!writeSDO_U16(0x2036, 0xE, reference1)) return false;
	
	//set frequency reference 2 to communication card (EtherCAT Card)
	uint16_t reference2 = 169;
	if(!writeSDO_U16(0x2036, 0xF, reference2)) return false;
	
	//set control profile to "Not separate" for pure CiA.402 Velocity Mode
	uint16_t controlProfile = 1;
	if(!writeSDO_U16(0x2036, 0x2, controlProfile)) return false;
	
	
	//—————————————— ASSIGN PDOS —————————————————
	
	if(!rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12)) return false;
	if(!txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13)) return false;
	
	return true;
}

//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void ATV320::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);

	powerStateActual = ds402SStatus.getPowerState();
	b_velocityTargetReached = ds402SStatus.getOperationModeSpecificByte10();
	
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV320::writeOutputs() {
	
	ds402Control.setPowerState(powerStateTarget, powerStateActual);
	if(b_resetFaultCommand) {
		b_resetFaultCommand = false;
		ds402Control.performFaultReset();
	}
	ds402Control.updateControlWord();
	
	
	rxPdoAssignement.pushDataTo(identity->outputs);
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

