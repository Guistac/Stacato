#include "EL7221-9014-MotionInterface.h"

#include "Fieldbus/EtherCatFieldbus.h"

void EL7221_9014::onDisconnection() {
	actuator->state = DeviceState::OFFLINE;
	gpio->state = DeviceState::OFFLINE;
	b_motorConnected = false;
}
void EL7221_9014::onConnection() {}
void EL7221_9014::initialize() {
	auto thisEL7211 = std::static_pointer_cast<EL7221_9014>(shared_from_this());
	actuator = std::make_shared<EL7211ServoMotor>(thisEL7211);
	gpio = std::make_shared<EL7211Gpio>(thisEL7211);
	
	actuatorPin->assignData(std::static_pointer_cast<ActuatorInterface>(actuator));
	gpioPin->assignData(std::static_pointer_cast<GpioInterface>(gpio));
	
	addNodePin(actuatorPin);
	addNodePin(gpioPin);
	addNodePin(digitalInput1_pin);
	addNodePin(digitalInput2_pin);
	
	rxPdoAssignement.addNewModule(0x1600);
	rxPdoAssignement.addEntry(0x7010, 0x1, 16, "DRV Controlword", &rxPdo.controlWord);
	rxPdoAssignement.addNewModule(0x1601);
	rxPdoAssignement.addEntry(0x7010, 0x6, 32, "DRV Target Velocity", &rxPdo.targetVelocity);
	rxPdoAssignement.addNewModule(0x1606);
	rxPdoAssignement.addEntry(0x7010, 0x5, 32, "DRV Target position", &rxPdo.targetPosition);
	rxPdoAssignement.addNewModule(0x1608);
	rxPdoAssignement.addEntry(0x7010, 0x3, 8, "DRV Modes of operation", &rxPdo.modeOfOperationSelection);
	
	txPdoAssignement.addNewModule(0x1A00);
	txPdoAssignement.addEntry(0x6000, 0x11, 32, "FB Position", &txPdo.fbPosition);
	txPdoAssignement.addNewModule(0x1A01);
	txPdoAssignement.addEntry(0x6010, 0x1, 16, "DRV Statusword", &txPdo.statusWord);
	txPdoAssignement.addNewModule(0x1A02);
	txPdoAssignement.addEntry(0x6010, 0x7, 32, "DRV Velocity actual value", &txPdo.velocityActualValue);
	txPdoAssignement.addNewModule(0x1A03);
	txPdoAssignement.addEntry(0x6010, 0x8, 16, "DRV Torque actual value", &txPdo.torqueActualValue);
	txPdoAssignement.addNewModule(0x1A04);
	txPdoAssignement.addEntry(0x6010, 0x12, 16, "Info Data 1 : Errors", &txPdo.infoData1_errors);
	txPdoAssignement.addNewModule(0x1A05);
	txPdoAssignement.addEntry(0x6010, 0x13, 16, "Info Data 2 : Digital Inputs", &txPdo.infoData2_digitalInputs);
	txPdoAssignement.addNewModule(0x1A06);
	txPdoAssignement.addEntry(0x6010, 0x6, 32, "DRV Following error actual value", &txPdo.followingErrorActualValue);
	txPdoAssignement.addNewModule(0x1A0C);
	txPdoAssignement.addEntry(0x6000, 0xE, 16, "FB Status", &txPdo.fbStatus);
	txPdoAssignement.addNewModule(0x1A0E);
	txPdoAssignement.addEntry(0x6010, 0x3, 8, "DRV Modes of operation display", &txPdo.modeOfOperationDisplay);
}



bool EL7221_9014::startupConfiguration() {
	
	rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12, false);
	txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13, false);
	
	double cycleTime_millis = EtherCatFieldbus::processInterval_milliseconds;
	uint32_t cycleTime_nanos = cycleTime_millis * 1000000;
	uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
	ec_dcsync01(getSlaveIndex(), true, cycleTime_nanos, 0, cycleOffset_nanos);
	
	return true;
}


void EL7221_9014::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	
	statusWord.readyToSwitchOn		= bool(txPdo.statusWord & (0x1 << 0));
	statusWord.switchedOn			= bool(txPdo.statusWord & (0x1 << 1));
	statusWord.operationEnabled		= bool(txPdo.statusWord & (0x1 << 2));
	statusWord.fault 				= bool(txPdo.statusWord & (0x1 << 3));
	statusWord.quickstop 			= bool(txPdo.statusWord & (0x1 << 5));
	statusWord.switchOnDisabled 	= bool(txPdo.statusWord & (0x1 << 6));
	statusWord.warning 				= bool(txPdo.statusWord & (0x1 << 7));
	statusWord.TxPdoToggle 			= bool(txPdo.statusWord & (0x1 << 10));
	statusWord.internalLimitActive	= bool(txPdo.statusWord & (0x1 << 11));
	statusWord.commandValueFollowed	= bool(txPdo.statusWord & (0x1 << 12));
	
	if(statusWord.fault)					powerStateActual = PowerState::FAULT;
	else if(statusWord.operationEnabled)	powerStateActual = PowerState::OPERATION_ENABLED;
	else if(statusWord.switchedOn)			powerStateActual = PowerState::SWITCHED_ON;
	else if(statusWord.readyToSwitchOn)		powerStateActual = PowerState::READY_TO_SWITCH_ON;
	else if(statusWord.switchOnDisabled)	powerStateActual = PowerState::SWITCH_ON_DISABLED;
	else 									powerStateActual = PowerState::NOT_READY_TO_SWITCH_ON;
	
	driverErrors.adc				= bool(txPdo.infoData1_errors & (0x1 << 0));
	driverErrors.overcurrent		= bool(txPdo.infoData1_errors & (0x1 << 1));
	driverErrors.undervoltage		= bool(txPdo.infoData1_errors & (0x1 << 2));
	driverErrors.overvoltage		= bool(txPdo.infoData1_errors & (0x1 << 3));
	driverErrors.overtemperature	= bool(txPdo.infoData1_errors & (0x1 << 4));
	driverErrors.i2tAmplifier		= bool(txPdo.infoData1_errors & (0x1 << 5));
	driverErrors.i2tMotor			= bool(txPdo.infoData1_errors & (0x1 << 6));
	driverErrors.encoder			= bool(txPdo.infoData1_errors & (0x1 << 7));
	driverErrors.watchdog			= bool(txPdo.infoData1_errors & (0x1 << 8));
	
	driverWarnings.undervoltage		= bool(txPdo.infoData2_digitalInputs & (0x1 << 2));
	driverWarnings.overvoltage		= bool(txPdo.infoData2_digitalInputs & (0x1 << 3));
	driverWarnings.overtemperature	= bool(txPdo.infoData2_digitalInputs & (0x1 << 4));
	driverWarnings.i2tAmplifier		= bool(txPdo.infoData2_digitalInputs & (0x1 << 5));
	driverWarnings.i2tMotor			= bool(txPdo.infoData2_digitalInputs & (0x1 << 6));
	driverWarnings.encoder			= bool(txPdo.infoData2_digitalInputs & (0x1 << 7));
	
	b_motorConnected = !bool(txPdo.fbStatus & (0x1 << 13));
	
	*digitalInput1_Value 									= bool(txPdo.infoData2_digitalInputs & (0x1 << 0));
	*digitalInput2_Value									= bool(txPdo.infoData2_digitalInputs & (0x1 << 1));
	actuator->actuatorProcessData.b_isEmergencyStopActive	= !bool(txPdo.infoData2_digitalInputs & (0x1 << 8));
	
}


void EL7221_9014::writeOutputs(){
	
	rxPdo.modeOfOperationSelection = 0x9;
	
	
	//handle fault reset request
	if(controlWord.faultReset) controlWord.faultReset = false;
	else if(b_faultResetRequest && statusWord.fault){
		b_faultResetRequest = false;
		controlWord.faultReset = true;
	}
	
	if(b_enableRequest) powerStateTarget = PowerState::OPERATION_ENABLED;
	else powerStateTarget = PowerState::READY_TO_SWITCH_ON;
	
	switch(powerStateTarget){
		case PowerState::OPERATION_ENABLED:
			controlWord.enableVoltage = true;
			controlWord.switchOn = true;
			controlWord.enableOperation = true;
			break;
		case PowerState::SWITCHED_ON:
			controlWord.enableVoltage = true;
			controlWord.switchOn = true;
			controlWord.enableOperation = false;
			break;
		case PowerState::READY_TO_SWITCH_ON:
			controlWord.enableVoltage = true;
			controlWord.switchOn = false;
			controlWord.enableOperation = false;
			break;
		case PowerState::SWITCH_ON_DISABLED:
			controlWord.enableVoltage = false;
			controlWord.switchOn = false;
			controlWord.enableOperation = false;
			break;
		default:
			controlWord.enableVoltage = false;
			controlWord.switchOn = false;
			controlWord.enableOperation = false;
	}
	
	rxPdo.controlWord = 0x0;
	if(controlWord.switchOn) 		rxPdo.controlWord |= 0x1 << 0;
	if(controlWord.enableVoltage) 	rxPdo.controlWord |= 0x1 << 1;
	if(!controlWord.quickstop) 		rxPdo.controlWord |= 0x1 << 2;
	if(controlWord.enableOperation) rxPdo.controlWord |= 0x1 << 3;
	if(controlWord.faultReset)		rxPdo.controlWord |= 0x1 << 7;
	
	rxPdo.targetVelocity = velocityRequest_rps * motorSettings.velocityResolution_rps;
	
	rxPdoAssignement.pushDataTo(identity->outputs);
}
bool EL7221_9014::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EL7221_9014::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }


void EL7221_9014::configureDrive(){
	
	//autoconfig has to be enabled and the device scanned ??
	if(!writeSDO_U8(0x8008, 0x1, 1, "Enable Autoconfig")) return;
	if(!writeSDO_U8(0x8008, 0x2, 1, "Reconfig Identical Motor")) return;
	if(!writeSDO_U8(0x8008, 0x3, 1, "Reconfig Non-Identical Motor")) return;
	
	
	//——— Info PDO Selection
	
	//Select Info Data 1 : Driver Errors
	if(!writeSDO_U8(0x8010, 0x39, 5, "Select Info Data 1")) return;
	
	//Select Info Data 2 : Digital Input Levels
	if(!writeSDO_U8(0x8010, 0x3A, 10, "Select Info Data 2")) return;
	
	//Select Info Data 2 : Driver Warnings
	//if(!writeSDO_U8(0x8010, 0x3A, 6, "Select Info Data 2")) return;
	
	
	//——— Motor Properties Download
	
	uint32_t velocityEncoderResolution;
	if(!readSDO_U32(0x9010, 0x14, velocityEncoderResolution, "Velocity Encoder Resolution")) return;
	motorSettings.velocityResolution_rps = velocityEncoderResolution;
	
	uint32_t positionEncoderResolution;
	if(!readSDO_U32(0x9010, 0x15, positionEncoderResolution, "Position Encoder Resolution")) return;
	motorSettings.positionResolution_rev = positionEncoderResolution;
	
	uint32_t motorMaxCurrent;
	if(!readSDO_U32(0x8011, 0x11, motorMaxCurrent, "Motor Max Current"))return;
	motorSettings.maxCurrent_amps = double(motorMaxCurrent) / 1000.0;
	
	uint32_t motorRatedCurrent;
	if(!readSDO_U32(0x8011, 0x12, motorRatedCurrent, "Motor Rated Current")) return;
	motorSettings.ratedCurrent_amps = double(motorRatedCurrent) / 1000.0;
	
	uint32_t motorSpeedLimitation;
	if(!readSDO_U32(0x8011, 0x1B, motorSpeedLimitation, "Motor Speed Limitation")) return;
	motorSettings.speedLimitation_rps = double(motorSpeedLimitation) / 60.0;
	
	Logger::info("Configuration Succeeded");
	
}

void EL7221_9014::writeEncoderPositionOffset(uint32_t offset){
	
	std::thread worker([this,offset](){
		
		uint8_t fbPositionValid;
		if(!readSDO_U8(0x6000, 0xE, fbPositionValid)) return;
		if(fbPositionValid != 0x0){
			Logger::warn("Encoder is Offline or being identified...");
			return;
		}
		
		Logger::info("Starting encoder offset write...");

		//Using "FB OCT Memory Interface"
		
		//set Command to "Write Encoder Position Offset"
		if(!writeSDO_S16(0xB001, 0x1, 16)) return;
				
		//Set Data Buffer containing new position offset
		uint32_t buffer[8] = {offset,0,0,0,0,0,0,0};
		if(1 != ec_SDOwrite(getSlaveIndex(), 0xB001, 0x6, false, 32, buffer, EC_TIMEOUTSAFE)) return;
		
		//Execute Command
		if(!writeSDO_S16(0xB001, 0x5, 1)) return;
		double startTime = Timing::getProgramTime_seconds();
		
		//Read status of command
		bool b_succcess = false;
		while(Timing::getProgramTime_seconds() - startTime < 2.0){
			int16_t result;
			if(readSDO_S16(0xB001, 0x5, result)){
				if(result == 3) {
					b_succcess = true;
					break;
				}
				else if(result == 4){
					Logger::warn("Encoder offset write returned error.");
					return;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		if(!b_succcess) {
			Logger::warn("Encoder offset write timed out.");
			return;
		}
		
		//read encoder position offset from FB OCT Info Data
		uint32_t offsetReadback;
		if(!readSDO_U32(0x9008, 0x20, offsetReadback)) return;
		
		if(offsetReadback == offset){
			Logger::info("Encoder offset write succeeded ! New offset is {}", offsetReadback);
		}
		else{
			Logger::warn("Encoder offset write succeeded but readback was not identical to request.");
			Logger::warn("Requested Offset: {}  Readback: {}", offset, offsetReadback);
		}
		
	});
	
	worker.detach();
	
	
}



std::string EL7221_9014::EL7211ServoMotor::getStatusString(){
	if(etherCatDevice->isOffline()) return "Drive is Offline.";
	if(!etherCatDevice->isStateOperational()) return "Drive is not in Operational State.";
	if(!etherCatDevice->b_motorConnected) return "Motor is not connected.";
	if(actuatorProcessData.b_isEmergencyStopActive) return "STO is active.";
	if(etherCatDevice->statusWord.fault){
		std::string faultMsg = "Drive has one or more errors:\n";
		std::string errors = "";
		if(etherCatDevice->driverErrors.adc)				errors += "	-ADC Error\n";
		if(etherCatDevice->driverErrors.overcurrent) 		errors += "	-Overcurrent error\n";
		if(etherCatDevice->driverErrors.undervoltage) 		errors += "	-Undervoltage error\n";
		if(etherCatDevice->driverErrors.overvoltage) 		errors += "	-Overvoltage error\n";
		if(etherCatDevice->driverErrors.overtemperature)	errors += "	-Overtemperature error\n";
		if(etherCatDevice->driverErrors.i2tAmplifier) 		errors += "	-I2tAmplifier error\n";
		if(etherCatDevice->driverErrors.i2tMotor) 			errors += "	-I2tMotor error\n";
		if(etherCatDevice->driverErrors.encoder) 			errors += "	-Encoder error\n";
		if(etherCatDevice->driverErrors.watchdog) 			errors += "	-Watchdog error\n";
		if(errors == "") errors = " -STO triggered with active axis (or other unknown error).";
		faultMsg += errors;
		return faultMsg;
	}
	if(isEnabled()) return "Drive is Enabled.";
	if(isReady()) return "Drive is Ready.";
	return "Unknown State";
}
