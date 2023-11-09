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


void EL7221_9014::firstSetup(){
	std::thread worker([this](){
	
		auto doFirstSetup = [this]()->bool{
			
			if(identity == nullptr) return false;
			
			Logger::info("Starting First Setup.");
			
			//Enable all autoconfig options
			//this way the drive configures all parameters for the connected motor
			//we will turn off autoconfig when first setup is done
			if(!writeSDO_U8(0x8008, 0x1, 1, "Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8008, 0x2, 1, "Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8008, 0x3, 1, "Reconfig Non-Identical Motor")) return false;
				
			//set ESM state to Init and the Pre-Operational to trigger reading of motor nameplate
			identity->state = EC_STATE_INIT;
			ec_writestate(getSlaveIndex());
			if(EC_STATE_INIT != ec_statecheck(getSlaveIndex(), EC_STATE_INIT, EC_TIMEOUTSTATE)) {
				return Logger::warn("Could not set ESM state to Init");
			}
			identity->state = EC_STATE_PRE_OP;
			ec_writestate(getSlaveIndex());
			if(EC_STATE_PRE_OP != ec_statecheck(getSlaveIndex(), EC_STATE_PRE_OP, EC_TIMEOUTSTATE)) {
				return Logger::warn("Could not set ESM state to Pre-Operational");
			}
			
			Logger::info("Starting reading of motor nameplate. (this can take up to 10 seconds)");
			double octReadStartTime = Timing::getProgramTime_seconds();
			while(true){
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				uint8_t fbPositionValid;
				if(readSDO_U8(0x6000, 0xE, fbPositionValid) && fbPositionValid == 0x0) break;
				if(Timing::getProgramTime_seconds() - octReadStartTime > 10.0){
					Logger::warn("failed to read motor nameplate, request timed out.");
					return false;
				}
			}
			
			char motorVendor[64];
			if(!readSDO_String(0x9009, 0x1, motorVendor, 64, "Motor Vendor")) return false;
			char motorType[64];
			if(!readSDO_String(0x9009, 0x2, motorType, 64, "Motor Type")) return false;
			char serialNumber[64];
			if(!readSDO_String(0x9009, 0x3, serialNumber, 64, "Serial Number")) return false;
			char orderCode[64];
			if(!readSDO_String(0x9009, 0x4, orderCode, 64, "Order Code")) return false;
			char motorContruction[64];
			if(!readSDO_String(0x9009, 0x5, motorContruction, 64, "Motor Construction")) return false;
			char brakeType[64];
			if(!readSDO_String(0x9009, 0x1B, brakeType, 64, "Brake Type")) return false;
			
			uint32_t encoderWorkingRangeRevolutions;
			if(!readSDO_U32(0x9008, 0x13, encoderWorkingRangeRevolutions, "Encoder Working Range")) return false;
			uint32_t encoderOperatingMinutes;
			if(!readSDO_U32(0x9008, 0x1C, encoderOperatingMinutes, "Encoder Operation Minute Counter")) return false;
			
			uint8_t singleturnBits;
			if(!readSDO_U8(0x8000, 0x12, singleturnBits, "Singleturn Resolution")) return false;
			uint8_t multiturnBits;
			if(!readSDO_U8(0x8000, 0x13, multiturnBits, "Multiturn Resolution")) return false;
			
			uint32_t motorMaxCurrentmA;
			if(!readSDO_U32(0x8011, 0x11, motorMaxCurrentmA, "Max Motor Current")) return false;
			uint32_t motorRatedCurrentmA;
			if(!readSDO_U32(0x8011, 0x12, motorRatedCurrentmA, "Rated Motor Current")) return false;
			uint32_t motorSpeedLimitationRpm;
			if(!readSDO_U32(0x8011, 0x1B, motorSpeedLimitationRpm, "Motor Speed Limitation")) return false;
			
			uint32_t velocityEncoderResolution;
			if(!readSDO_U32(0x9010, 0x14, velocityEncoderResolution, "Velocity Encoder Resolution")) return false;
			uint32_t positionEncoderResolution;
			if(!readSDO_U32(0x9010, 0x15, positionEncoderResolution, "Position Encoder Resolution")) return false;
			
			Logger::info("---- Motor Nameplate ----");
			Logger::info("	Vendor: {}", motorVendor);
			Logger::info("	Type: {}", motorType);
			Logger::info("	Serial Number: {}", serialNumber);
			Logger::info("	Order Code: {}", orderCode);
			Logger::info("	Motor Construction: {}", motorContruction);
			Logger::info("	Brake Type: {}", brakeType);
			Logger::info("	Rated Current: {}A", double(motorRatedCurrentmA) / 1000.0);
			Logger::info("	Max Current: {}A", double(motorMaxCurrentmA) / 1000.0);
			Logger::info("	Max Speed: {}rpm", motorSpeedLimitationRpm);
			Logger::info("	Encoder Resolution: {}bits singleturn / {}bits multiturn", singleturnBits, multiturnBits);
			Logger::info("	Encoder Working Range: {}rev", encoderWorkingRangeRevolutions);
			Logger::info("	Operating Hours: {}h", encoderOperatingMinutes / 60);
			
			//disable autoconfig for future motor connections
			if(!writeSDO_U8(0x8008, 0x1, 0, "Enable Autoconfig")) return false;
			if(!writeSDO_U8(0x8008, 0x2, 0, "Reconfig Identical Motor")) return false;
			if(!writeSDO_U8(0x8008, 0x3, 0, "Reconfig Non-Identical Motor")) return false;
			
			//set position offset source to encoder memory
			if(!writeSDO_U8(0x8000, 0xD, 1, "Position Offset Source")) return false;
						
			//Select Info Data 1 : Driver Errors
			if(!writeSDO_U8(0x8010, 0x39, 5, "Select Info Data 1")) return false;
			
			//Select Info Data 2 : Digital Input Levels
			if(!writeSDO_U8(0x8010, 0x3A, 10, "Select Info Data 2")) return false;
			
			//set REFERENCED field
			//It will get set to false/0x0 if a motor with a different serial number or a motor with single turn encoder is connected.
			//This way we can detect if the motor changed and force first setup again.
			if(!writeSDO_U8(0x8000, 0x2, 1)) return false;
			
			motorSettings.maxCurrent_amps = double(motorMaxCurrentmA) / 1000.0;
			motorSettings.maxVelocity_rps = double(motorSpeedLimitationRpm) / 60.0;
			motorSettings.workingRange_rev = encoderWorkingRangeRevolutions;
			motorSettings.velocityResolution_rps = velocityEncoderResolution;
			motorSettings.positionResolution_rev = positionEncoderResolution;
			
			return true;
		};
		
		if(doFirstSetup()) Logger::info("First Setup Finished Successfully.");
		else Logger::warn("First Setup Failed");
	});
	worker.detach();
}

void EL7221_9014::resetEncoderPosition(){
	std::thread worker([this](){
		
		auto overrideEncoderPosition = [this]() -> bool {

			if(!isStateOperational()) {
				Logger::warn("Encoder cannot be reset while the fieldbus is not running");
				return false;
			}
			
			uint8_t fbPositionValid;
			if(!readSDO_U8(0x6000, 0xE, fbPositionValid) || fbPositionValid != 0x0){
				Logger::warn("Encoder is Offline or being identified.");
				return false;
			}

			uint32_t positionEncoderResolution;
			if(!readSDO_U32(0x9010, 0x15, positionEncoderResolution, "Position Encoder Resolution")) return false;
			uint32_t encoderWorkingRangeRevolutions;
			if(!readSDO_U32(0x9008, 0x13, encoderWorkingRangeRevolutions, "Encoder Working Range")) return false;
			uint32_t positionActual;
			if(!readSDO_U32(0x6000, 0x11, positionActual, "Encoder Position")) return false;
			uint32_t offsetActual;
			if(!readSDO_U32(0x9008, 0x20, offsetActual, "Encoder Position Offset")) return false;
			
			int maxOffset = positionEncoderResolution * encoderWorkingRangeRevolutions;
			
			//positionActual = rawPosition - currentOffset
			//rawPosition = positionActual + currentOffset
			//
			//rawPosition - newOffset = 0
			//newOffset = rawPosition
			
			uint32_t rawPosition = positionActual + offsetActual;
			if(rawPosition >= maxOffset) rawPosition -= maxOffset;
			uint32_t offset = rawPosition;
			
			Logger::info("Actual Position: {}", positionActual);
			Logger::info("Current Offset: {}", offsetActual);
			Logger::info("Raw Position: {}", rawPosition);
			
			
			Logger::info("Starting encoder offset write...");
			
			//Set OCT Interface Command to "Write Encoder Position Offset"
			if(!writeSDO_S16(0xB001, 0x1, 16, "OCT Interface Command 'Write Encoder Position Offset'")) return false;
					
			//Set Data Buffer containing new position offset
			uint32_t buffer[8] = {offset,0,0,0,0,0,0,0};
			if(1 != ec_SDOwrite(getSlaveIndex(), 0xB001, 0x6, false, 32, buffer, EC_TIMEOUTSAFE)) {
				Logger::warn("Could not upload encoder position offset to data buffer");
				return false;
			}
			
			//Execute Command
			if(!writeSDO_S16(0xB001, 0x5, 1, "Execute OCT Interface Command")) return false;
			
			//Wait for command status update
			double commandExecuteTime = Timing::getProgramTime_seconds();
			while(true){
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				int16_t result;
				if(readSDO_S16(0xB001, 0x5, result)){
					if(result == 3) break;
					if(result == 4){
						Logger::warn("Encoder offset write returned error.");
						return false;
					}
				}
				if(Timing::getProgramTime_seconds() - commandExecuteTime > 2.0){
					Logger::warn("Encoder offset write timed out.");
					return false;
				}
			}
			
			//read encoder position offset from FB OCT Info Data
			uint32_t offsetReadback;
			if(!readSDO_U32(0x9008, 0x20, offsetReadback, "Encoder Position Offset")) return false;
			
			if(offsetReadback != offset){
				Logger::warn("Encoder offset write succeeded but readback was not identical to request.");
				Logger::warn("Requested Offset: {}  Readback: {}", offset, offsetReadback);
				return false;
			}
			
			return true;
			
		};
		
		if(overrideEncoderPosition()) Logger::info("Encoder Position Reset Succeeded.");
		else Logger::warn("Encoder Position Reset Failed.");
		
		
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
		if(errors == "") 									errors += " -STO triggered with active axis (or other unknown error).\n";
		faultMsg += errors;
		faultMsg.pop_back(); //remove last line return from message
		return faultMsg;
	}
	if(isEnabled()) return "Drive is Enabled.";
	if(isReady()) return "Drive is Ready.";
	return "Unknown State";
}


void EL7221_9014::downloadDiagnostics(){
	
	uint8_t newestMessageSubindex;
	if(!readSDO_U8(0x10F3, 0x2, newestMessageSubindex)) return;
	if(newestMessageSubindex == 0x0) {
		Logger::info("No Diagnostics Message Available.");
		return;
	}
	
	uint8_t messageCount = newestMessageSubindex - 5;
	
	Logger::info("----- {} Diagnostics Messages -----", messageCount);
	for(int i = 0; i < messageCount; i++){
		
		uint8_t messageSubindex = 6 + i;
		int size = 28;
		uint8_t buffer[28];
		if(1 == ec_SDOread(getSlaveIndex(), 0x10F3, messageSubindex, false, &size, buffer, EC_TIMEOUTSAFE)){
			uint32_t diagCode = *((uint32_t*)(&buffer[0]));
			uint16_t flags = *((uint16_t*)(&buffer[4]));
			uint16_t textID = *((uint16_t*)(&buffer[6]));
			uint64_t timestamp = *((uint64_t*)(&buffer[8]));
			std::string message = getDiagnosticsStringFromTextID(textID);
			
			if(flags == 0x0) 		Logger::info(		"[{}] Info    0x{:x} : {}", i, textID, message);
			else if(flags == 0x1) 	Logger::warn(		"[{}] Warning 0x{:x} : {}", i, textID, message);
			else if(flags == 0x2) 	Logger::error(		"[{}] Error   0x{:x} : {}", i, textID, message);
			else 					Logger::critical(	"[{}] Message 0x{:x} : {}", i, textID, message);
		}
	}
	
}



std::string EL7221_9014::getDiagnosticsStringFromTextID(uint16_t textID){
	switch(textID){
		case 0x0001: return "No error";
		case 0x0002: return "Communication established";
		case 0x1201: return "Communication re-established";
		case 0x4101: return "Terminal-Overtemperature";
		case 0x4102: return "Discrepancy in the PDO-Configuration";
		case 0x4301: return "Feedback-Warning";
		case 0x4411: return "DC-Link undervoltage";
		case 0x4412: return "DC-Link overvoltage";
		case 0x4413: return "I2T Amplifier overload";
		case 0x4414: return "I2T Motor overload";
		case 0x4415: return "Speed limitation active";
		case 0x4417: return "Motor-Overtemperature";
		case 0x4418: return "Limit: Current";
		case 0x4419: return "Limit: Amplifier I2T-model exceeds 100%%";
		case 0x441A: return "Limit: Motor I2T-model exceeds 100%%";
		case 0x441B: return "Limit: Velocity limitation";
		case 0x441C: return "Voltage on STO-/Hardware enable input missing";
		case 0x441D: return "Internal hardware error";
		case 0x4420: return "Cogging compensation not supported (%u)";
		case 0x8002: return "Communication aborted";
		case 0x8102: return "Invalid combination of Inputs and Outputs PDOs";
		case 0x8103: return "No variable linkage";
		case 0x8104: return "Terminal-Overtemperature";
		case 0x8105: return "PD-Watchdog";
		case 0x8135: return "Cycletime has to be a multiple of 125 �s";
		case 0x8137: return "Electronic name plate: CRC error";
		case 0x8146: return "Sync-Mode and PDO-Configuration are not compatible";
		case 0x8201: return "No communication to field-side (Auxiliary voltage missing)";
		case 0x8302: return "Feedback-Error";
		case 0x8304: return "OCT communication error";
		case 0x8403: return "ADC Error";
		case 0x8404: return "Overcurrent";
		case 0x8406: return "Undervoltage DC-Link";
		case 0x8407: return "Overvoltage DC-Link";
		case 0x8408: return "I2T-Model Amplifier overload";
		case 0x8409: return "I2T-Model motor overload";
		case 0x840B: return "Motor error or commutation malfunction";
		case 0x840C: return "Phase failure";
		case 0x8416: return "Motor-Overtemperature";
		case 0x8417: return "Maximum rotating field velocity exceeded";
		case 0x841C: return "STO-/Hardware enable input de-energized while the axis was enabled";
		case 0x841D: return "Internal hardware error";
		case 0x8441: return "Following error";
		case 0x8450: return "Invalid start type 0x%x";
		case 0x8451: return "Invalid limit switch level";
		case 0x8452: return "Drive error during positioning";
		case 0x8453: return "Latch unit will be used by multiple modules";
		case 0x8454: return "Drive not in control";
		case 0x8455: return "Invalid value for \"Target acceleration\"";
		case 0x8456: return "Invalid value for \"Target deceleration\"";
		case 0x8457: return "Invalid value for \"Target velocity\"";
		case 0x8458: return "Invalid value for \"Target position\"";
		case 0x8459: return "Emergency stop active";
		case 0x845A: return "Target position exceeds Modulofactor";
		case 0x845B: return "Drive must be disabled";
		case 0x845C: return "No Feedback found";
		case 0x845D: return "Modulo factor invalid";
		case 0x845E: return "Invalid target position window";
		default: return "Unknown Diagnostics Message";
	}
}
