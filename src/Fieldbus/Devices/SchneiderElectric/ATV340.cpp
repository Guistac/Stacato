#include <pch.h>
#include "ATV340.h"

#include "Fieldbus/EtherCatFieldbus.h"

void ATV340::onConnection() {}

void ATV340::onDisconnection() {}

void ATV340::initialize() {
	auto thisATV340 = std::static_pointer_cast<ATV340>(shared_from_this());
	axis = DS402Axis::make(thisATV340);
	
	rxPdoAssignement.addNewModule(0x1600);
	txPdoAssignement.addNewModule(0x1A00);
	axis->processDataConfiguration.operatingModes.frequency = true;
	axis->configureProcessData();
	
	rxPdoAssignement.addEntry(0x2016, 0xD, 16, "Logic Outputs States", &logicOutputs);
	
	txPdoAssignement.addEntry(0x2016, 0x2, 16, "Logic Inputs Physical Image", &logicInputs);
	txPdoAssignement.addEntry(0x207B, 0x17, 16, "Safe torque Off function Status", &stoState);
	txPdoAssignement.addEntry(0x2029, 0x16, 16, "LastFaultCode", &lastFaultCode);
	//txPdoAssignement.addEntry(0x2016, 0x17, 16, "AI1 Input Physical Image", &analogInput1);
	
	//txPdoAssignement.addEntry(0x2002, 0xC, 16, "MotorPower", &motorPower);
}

//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool ATV340::startupConfiguration() {
	
	axis->setOperatingMode(DS402Axis::OperatingMode::VELOCITY);
	
	//———— Control Profile Settings
	/*
	//[fr1] configuration for reference frequency channel 1
	uint16_t ref1configuration = 169; //169 = Communication Module
	if(!writeSDO_U16(0x2036, 0xE, ref1configuration)) return false;

	//[rfc] Frequency Switching assignement
	uint16_t frequencySwitchingAssigment = 96; //96 = Reference Frequency Channel 1
	if(!writeSDO_U16(0x2036, 0xC, frequencySwitchingAssigment)) return false;
	
	//[chcf] Control Mode
	uint16_t controlMode = 1; //1=combined mode, ethercat has complete control of the drive
	if(!writeSDO_U16(0x2036, 0x2, controlMode)) return false;
	*/
	
	/*
	//———— Dynamics and Limits
	
	//[inr] Ramp Time Increment: 0.01 second increments
	uint16_t rampIncrement = 0; //(0=0.01, 1=0.1, 2=1.0)
	if(!writeSDO_U16(0x203C, 0x15, rampIncrement)) return false;
	
	//[lsp] low speed (0.1Hz Increments)
	uint16_t lowSpeed = 0;
	if(!writeSDO_U16(0x2001, 0x6, lowSpeed)) return false;
	
	//[hsp] high speed (0.1Hz Increments)
	uint16_t highSpeed = 1000;
	if(!writeSDO_U16(0x2001, 0x5, highSpeed)) return false;
	
	//[acc] acceleration ramp time (0.01 second increments)
	uint16_t accelerationRampTime = 300;
	if(!writeSDO_U16(0x203C, 0x2, accelerationRampTime)) return false;
	
	//[dec] deceleration ramp time (0.01 second increments)
	uint16_t decelerationRampTime = 300;
	if(!writeSDO_U16(0x203C, 0x3, decelerationRampTime)) return false;
	*/
	
	
	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex())) return false;
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex())) return false;
	
	return true;
}

bool ATV340::configureMotor(){
	
	//———— Unit Scaling Selection
	
	uint16_t powerScalingCode;
	if(!readSDO_U16(0x2044, 0x3, powerScalingCode)) return false;
	double powerScaling;
	switch(powerScalingCode){
		case 20: powerScaling = 0.001; break;
		case 30: powerScaling = 0.01; break;
		case 40: powerScaling = 0.1; break;
		case 50: powerScaling = 1.0; break;
		case 60: powerScaling = 10.0; break;
		default: powerScaling = 0.0; break;
	}
	
	//Current Scaling: 0.01 Ampere increments
	uint16_t currentScalingCode = 30; //(20=0.001, 30=0.01, 40=0.1, 50=1.0, 60=10.0)
	if(!readSDO_U16(0x2044, 0x2, currentScalingCode)) return false;
	double currentScaling;
	switch(currentScalingCode){
		case 20: currentScaling = 0.001; break;
		case 30: currentScaling = 0.01; break;
		case 40: currentScaling = 0.1; break;
		case 50: currentScaling = 1.0; break;
		case 60: currentScaling = 10.0; break;
		default: currentScaling = 0.0; break;
	}
	
	//———— Motor Standard
	
	//[bfr] {Async} motor standard frequency
	uint16_t motorStandard = 0; // (0=50Hz,1=60Hz)
	if(!writeSDO_U16(0x2000, 0x10, motorStandard)) return false;
	
	
	//———— Motor Nameplate
	
	//[mpc] motor parameter choice (0=NominalPower,1=NominalCosinusPhi)
	uint16_t motorParameterChoice = 0;
	if(!writeSDO_U16(0x2042, 0xF, motorParameterChoice)) return false;
	
	
	if(motorParameterChoice == 1){
		//[cos] {Async} motor 1 cosinus phi (0.01 increments)
		uint16_t motor1CosinusPhi = 0;
		if(!writeSDO_U16(0x2042, 0x7, motor1CosinusPhi)) return false;
	}
	//	^
	//	| one or the other depending on Motor Param Choise [mpc]
	//	v
	if(motorParameterChoice == 0){
		//[npr] {Async} nominal motor power (0.01 Watt increments)
		uint16_t nominalMotorPower = 0;
		if(!writeSDO_U16(0x2042, 0xE, nominalMotorPower)) return false;
	}
	
	//[uns] {Async} nominal motor voltage (1v increments)
	uint16_t nominalMotorVoltage = 0;
	if(!writeSDO_U16(0x2042, 0x2, nominalMotorVoltage)) return false;
	
	//[ncr] {Async} nominal motor current (0.01 Ampere increments)
	uint16_t nominalMotorCurrent = 0;
	if(!writeSDO_U16(0x2042, 0x4, nominalMotorCurrent)) return false;
	
	//[frs] {Async} nominal motor frequency (0.1Hz increments)
	uint16_t nominalMotorFrequency = 0;
	if(!writeSDO_S16(0x2042, 0x3, nominalMotorFrequency)) return false;
	
	//[nsp] {Async} nominal motor speed (rpm)
	uint16_t nominalMotorSpeed = 0;
	if(!writeSDO_U16(0x2042, 0x5, nominalMotorSpeed)) return false;
	
	//[ith] Motor Thermal Current (0.01 Ampere increments)
	uint16_t motorThermalCurrent = 0;
	if(!writeSDO_U16(0x2042, 0x17, motorThermalCurrent)) return false;
	
	
	//———— Maximum Motor Frequency
	
	//[tfr] Motor Maximum Frequency (0.1 Hz increments)
	uint16_t motorMaximumFrequency = 0;
	if(!writeSDO_U16(0x2001, 0x4, motorMaximumFrequency)) return false;

	//———— Brake Logic Control
	
	//[blc] brake assignement
	//0 = None
	//2 = R2
	//64 = DQ1
	//65 = DQ2
	uint16_t brakeAssignement = 2;
	if(!writeSDO_U16(0x2046, 0x2, brakeAssignement)) return false;
	
	//[bst] movement type (0= Horizontal Movement, 1=Hoisting)
	uint16_t brakeMovementType = 0;
	if(!writeSDO_U16(0x2046, 0x9, brakeMovementType)) return false;
	
	//... other brake parameters
	
	
	//———— Embedded Encoder
	
	//[eecp] embedde encoder etype (0=None, 1=AB, 2=SinCos)
	uint16_t encoderType = 1;
	if(!writeSDO_U16(0x201A, 0x47, encoderType)) return false;
	
	//[eecv] embedded encoder supply voltage (5=5V, 12=12V, 24=24V)
	uint16_t encoderSupplyVoltage = 24;
	if(!writeSDO_U16(0x201A, 0x50, encoderSupplyVoltage)) return false;
	
	//[epg] pulses per encoder revolution
	uint16_t pulsesPerEncoderRevolution = 1024;
	if(!writeSDO_U16(0x201A, 0x48, pulsesPerEncoderRevolution)) return false;
	
	//[eeri] emebedded encoder revolution inversion (0=No, 1=Yes)
	uint16_t encoderinvertion = 0;
	if(!writeSDO_U16(0x201A, 0x4F, encoderinvertion)) return false;
	
	//[eenu] embedded encoder usage (0=None, 1=SpeedMonitoring, 2=SpeedRegulation, 3=SpeedReference)
	uint16_t embeddedEncoderUsage = 2;
	if(!writeSDO_U16(0x201A, 0x4E, embeddedEncoderUsage)) return false;
	
	

	//———— Motor Control Type
	
	//[ctt] Motor Control Type
	uint16_t motorControlType = 2;
	//0 = Sensorless flux vector V (no encoder feedback, multiple identical motors supported)
	//2 = Full flux Vector (encoder feedback necessary)
	if(!writeSDO_U16(0x2042, 0x8, motorControlType)) return false;
	
	
	
	//———— Standstill motor tune after motor parameter assignement
	
	//[tun] autotuning (0=NoAction; 1=ApplyAutotuning; 2=EraseAutotuning)
	uint16_t autotuning = 1;
	if(!writeSDO_U16(0x2042, 0x9, autotuning)) return false;

	//[stun] tune selection (0=Default; 1=Measure; 2=Custom)
	uint16_t autotuningSelection = 1;
	if(!writeSDO_U16(0x2042, 0x12, autotuningSelection)) return false;
	
	uint16_t autotuningStatus = 0;
	while(autotuning != 3 && autotuning != 4){
		//[tus] autotuning status (0=NotDone; 1=Pending; 2=InProgress; 3=Fail; 4=Done)
		readSDO_U16(0x2042, 0xA, autotuningStatus);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	if(autotuningStatus == 3) Logger::warn("Autotuning Failed");
	else if(autotuningStatus == 4) Logger::warn("Autotuning Done");
	
	return true;
}

//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void ATV340::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	axis->updateInputs();
}

//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void ATV340::writeOutputs() {
	
	logicOutputs = 0x0;
	if(relayOut1)	logicOutputs |= 0x1 << 0;
	if(relayOut2)	logicOutputs |= 0x1 << 1;
	if(digitalOut1) logicOutputs |= 0x1 << 8;
	if(digitalOut2) logicOutputs |= 0x1 << 9;
	
	axis->updateOutput();
	rxPdoAssignement.pushDataTo(identity->outputs);
}

//============================= SAVING AND LOADING DEVICE DATA ============================

bool ATV340::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}

bool ATV340::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	return true;
}
