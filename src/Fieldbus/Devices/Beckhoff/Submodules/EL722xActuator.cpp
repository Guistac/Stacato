#include "EL722xActuator.h"

void EL722x_Actuator::initialize(){
	auto thisActuatorInterface = std::static_pointer_cast<ActuatorInterface>(shared_from_this());
	actuatorPin->assignData(thisActuatorInterface);
	driveSettings.accelerationLimit->addEditCallback([this](){ updateProprieties(); });
	driveSettings.velocityLimit->addEditCallback([this](){ updateProprieties(); });
}

void EL722x_Actuator::updateProprieties(){
	actuatorConfig.accelerationLimit = std::abs(driveSettings.accelerationLimit->value);
	actuatorConfig.velocityLimit = std::abs(driveSettings.velocityLimit->value);
	actuatorConfig.b_supportsPositionControl = true;
	actuatorConfig.b_supportsVelocityControl = true;
	actuatorConfig.b_supportsForceControl = false;
	actuatorConfig.b_supportsHoldingBrakeControl = false;
	actuatorConfig.b_supportsEffortFeedback = true;
	actuatorConfig.b_canQuickstop = false;
	
	feedbackConfig.b_supportsPositionFeedback = true;
	feedbackConfig.b_supportsVelocityFeedback = true;
	feedbackConfig.b_supportsForceFeedback = true;
	feedbackConfig.positionFeedbackType = PositionFeedbackType::ABSOLUTE;
	feedbackConfig.positionLowerWorkingRangeBound = -motorNameplate.workingRange_rev / 2.0;
	feedbackConfig.positionUpperWorkingRangeBound = motorNameplate.workingRange_rev / 2.0;
	
	actuatorPin->updateConnectedPins();
}

void EL722x_Actuator::onDisconnection(){
	state = DeviceState::OFFLINE;
	processData.b_motorConnected = false;
}

void EL722x_Actuator::readInputs(){
	
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
	
	if(statusWord.fault)					processData.powerStateActual = PowerState::FAULT;
	else if(statusWord.operationEnabled)	processData.powerStateActual = PowerState::OPERATION_ENABLED;
	else if(statusWord.switchedOn)			processData.powerStateActual = PowerState::SWITCHED_ON;
	else if(statusWord.readyToSwitchOn)		processData.powerStateActual = PowerState::READY_TO_SWITCH_ON;
	else if(statusWord.switchOnDisabled)	processData.powerStateActual = PowerState::SWITCH_ON_DISABLED;
	else 									processData.powerStateActual = PowerState::NOT_READY_TO_SWITCH_ON;
	
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
	
	processData.b_motorConnected = !bool(txPdo.fbStatus & (0x1 << 13));
	
   
	//actuatorProcessData.b_isEmergencyStopActive	= !bool(txPdo.infoData2_digitalInputs & (0x1 << 8));
	actuatorProcessData.followingErrorActual = double(txPdo.followingErrorActualValue) / motorNameplate.positionResolution_rev;
	double actualCurrent = std::abs(motorNameplate.ratedCurrent_amps * double(txPdo.torqueActualValue) / 1000.0);
	actuatorProcessData.effortActual = actualCurrent / driveSettings.currentLimit->value;
   
	feedbackProcessData.positionActual = double(txPdo.fbPosition) / motorNameplate.positionResolution_rev;
	feedbackProcessData.velocityActual = double(txPdo.velocityActualValue) / motorNameplate.velocityResolution_rps;
	feedbackProcessData.forceActual = double(txPdo.torqueActualValue) / 1000.0 * motorNameplate.ratedCurrent_amps * motorNameplate.torqueConstant_mNmpA / 1000.0;
   
   if(!processData.b_motorConnected) 										state = DeviceState::NOT_READY;
   else if(isEmergencyStopActive())											state = DeviceState::NOT_READY;
   else if(processData.powerStateActual == PowerState::OPERATION_ENABLED && processData.powerStateTarget != PowerState::OPERATION_ENABLED) state = DeviceState::DISABLING;
   else if(processData.powerStateTarget == PowerState::OPERATION_ENABLED && processData.powerStateActual != PowerState::OPERATION_ENABLED) state = DeviceState::ENABLING;
   else if(processData.powerStateActual == PowerState::OPERATION_ENABLED)	state = DeviceState::ENABLED;
   else if(processData.powerStateActual == PowerState::READY_TO_SWITCH_ON) 	state = DeviceState::READY;
   else if(processData.powerStateActual == PowerState::SWITCHED_ON) 		state = DeviceState::READY;
   else if(processData.powerStateActual == PowerState::FAULT)				state = DeviceState::READY;
   else 																	state = DeviceState::NOT_READY;
   
   //fault event detection
   if(!processData.b_hadFault && statusWord.fault){
	   processData.b_hadFault = true;
	   Logger::warn("[{}] Fault Detected.", getName());
   }
   else if(processData.b_hadFault && !statusWord.fault){
	   processData.b_hadFault = false;
	   Logger::info("[{}] Fault Cleared", getName());
   }
   
}

void EL722x_Actuator::writeOutputs(){
	
	 //reset fault request bit
	 if(controlWord.faultReset) controlWord.faultReset = false;
	 
	 if(actuatorProcessData.b_enable){
		 actuatorProcessData.b_enable = false;
		 processData.b_waitingForEnable = true;
		 processData.enableRequestTime_nanos = etherCatDevice->cycleProgramTime_nanoseconds;
		 if(statusWord.fault) controlWord.faultReset = true;
	 }
	 if(actuatorProcessData.b_disable){
		 actuatorProcessData.b_disable = false;
		 processData.b_waitingForEnable = false;
		 processData.powerStateTarget = PowerState::READY_TO_SWITCH_ON;
	 }
	 
	 if(processData.b_waitingForEnable){
		 if(!statusWord.fault) processData.powerStateTarget = PowerState::OPERATION_ENABLED;
		 //else processData.powerStateTarget = PowerState::READY_TO_SWITCH_ON;
		 double timeSinceEnableRequest_ms = double(etherCatDevice->cycleProgramTime_nanoseconds - processData.enableRequestTime_nanos) / 1000000.0;
		 if(timeSinceEnableRequest_ms > 100.0){
			 Logger::warn("[{}] Enable request timed out", getName());
			 processData.b_waitingForEnable = false;
			 processData.powerStateTarget = PowerState::READY_TO_SWITCH_ON;
		 }
		 if(processData.powerStateActual == PowerState::OPERATION_ENABLED) {
			 processData.b_waitingForEnable = false;
			 Logger::trace("[{}] Drive Enabled (took {}ms)", getName(), timeSinceEnableRequest_ms);
		 }
	 }
	 else if(statusWord.fault) processData.powerStateTarget = PowerState::READY_TO_SWITCH_ON;
	 
	switch(processData.powerStateTarget){
			
		case PowerState::OPERATION_ENABLED:
			switch (processData.powerStateActual) {
				case PowerState::READY_TO_SWITCH_ON:
					controlWord.enableVoltage = true;
					controlWord.switchOn = true;
					controlWord.enableOperation = false;
					break;
				case PowerState::SWITCHED_ON:
				case PowerState::OPERATION_ENABLED:
					controlWord.enableVoltage = true;
					controlWord.switchOn = true;
					controlWord.enableOperation = true;
					break;
				default:
					controlWord.enableVoltage = true;
					controlWord.switchOn = false;
					controlWord.enableOperation = false;
					break;
			}
			break;
		case PowerState::READY_TO_SWITCH_ON:
		default:
			controlWord.enableVoltage = true;
			controlWord.switchOn = false;
			controlWord.enableOperation = false;
			break;
	}
	
	 
	 rxPdo.controlWord = 0x0;
	 if(controlWord.switchOn) 		rxPdo.controlWord |= 0x1 << 0;
	 if(controlWord.enableVoltage) 	rxPdo.controlWord |= 0x1 << 1;
	 if(!controlWord.quickstop) 		rxPdo.controlWord |= 0x1 << 2;
	 if(controlWord.enableOperation) rxPdo.controlWord |= 0x1 << 3;
	 if(controlWord.faultReset)		rxPdo.controlWord |= 0x1 << 7;
	 
	 rxPdo.targetVelocity = actuatorProcessData.velocityTarget * motorNameplate.velocityResolution_rps;
	 rxPdo.targetPosition = std::abs(actuatorProcessData.positionTarget) * motorNameplate.positionResolution_rev;
	 if(actuatorProcessData.positionTarget < 0.0) rxPdo.targetPosition = UINT32_MAX - rxPdo.targetPosition;
	 
	 switch(actuatorProcessData.controlMode){
		 case ActuatorInterface::ControlMode::POSITION:
			 rxPdo.modeOfOperationSelection = 0x8;
			 break;
		 case ActuatorInterface::ControlMode::VELOCITY:
			 rxPdo.modeOfOperationSelection = 0x9;
			 break;
		 case ActuatorInterface::ControlMode::FORCE:
			 rxPdo.modeOfOperationSelection = 0xA;
			 break;
	 }
}



//std::string firstSetupProgress = "";
void EL722x_Actuator::firstSetup(){
	 std::thread worker([this](){
	 
		 auto doFirstSetup = [this]()->bool{
			 
			 if(etherCatDevice->identity == nullptr) return false;
			 
			 Logger::info("Starting First Setup.");
			 firstSetupProgress.write("Starting...");
			 
			 //Enable all autoconfig options
			 //this way the drive configures all parameters for the connected motor
			 //we will turn off autoconfig when first setup is done
			 if(!etherCatDevice->writeSDO_U8(0x8008 + getCanOpenIndexOffset(), 0x1, 1, "Enable Autoconfig")) return false;
			 if(!etherCatDevice->writeSDO_U8(0x8008 + getCanOpenIndexOffset(), 0x2, 1, "Reconfig Identical Motor")) return false;
			 if(!etherCatDevice->writeSDO_U8(0x8008 + getCanOpenIndexOffset(), 0x3, 1, "Reconfig Non-Identical Motor")) return false;
				 
			 
			 
			 //set ESM state to Init and the Pre-Operational to trigger reading of motor nameplate
			 etherCatDevice->identity->state = EC_STATE_PRE_OP;
			 ec_writestate(etherCatDevice->getSlaveIndex());
			 if(EC_STATE_INIT != ec_statecheck(etherCatDevice->getSlaveIndex(), EC_STATE_INIT, EC_TIMEOUTSTATE)) {
				 return Logger::warn("Could not set ESM state to Init");
			 }else Logger::info("State Preop");
			 
			 etherCatDevice->identity->state = EC_STATE_SAFE_OP;
			 ec_writestate(etherCatDevice->getSlaveIndex());
			 if(EC_STATE_PRE_OP != ec_statecheck(etherCatDevice->getSlaveIndex(), EC_STATE_PRE_OP, EC_TIMEOUTSTATE)) {
				 return Logger::warn("Could not set ESM state to Safe-Op");
			 }else Logger::info("State SafeOP");
			 
			 
			 //reading of nameplate seems to happen on transition to preop
	
			 Logger::info("Starting reading of motor nameplate. (this can take up to 10 seconds)");
			 firstSetupProgress.write("Reading motor nameplate (0s)");
			 
			 double octReadStartTime = Timing::getProgramTime_seconds();
			 while(true){
				 std::this_thread::sleep_for(std::chrono::milliseconds(100));
				 uint8_t fbPositionValid;
				 if(etherCatDevice->readSDO_U8(0x6000 + getCanOpenIndexOffset(), 0xE, fbPositionValid) && fbPositionValid == 0x0) break;
				 
				 double readTime = Timing::getProgramTime_seconds() - octReadStartTime;
				 if(readTime > 10.0){
					 Logger::warn("failed to read motor nameplate, request timed out.");
					 return false;
				 }
				 
				 char loadString[32];
				 snprintf(loadString, 32, "Reading motor nameplate (%.1fs)", readTime);
				 firstSetupProgress.write(loadString);
			 }
			 
			 char motorVendor[64];
			 if(!etherCatDevice->readSDO_String(0x9009 + getCanOpenIndexOffset(), 0x1, motorVendor, 64, "Motor Vendor")) return false;
			 char motorType[64];
			 if(!etherCatDevice->readSDO_String(0x9009 + getCanOpenIndexOffset(), 0x2, motorType, 64, "Motor Type")) return false;
			 char serialNumber[64];
			 if(!etherCatDevice->readSDO_String(0x9009 + getCanOpenIndexOffset(), 0x3, serialNumber, 64, "Serial Number")) return false;
			 char orderCode[64];
			 if(!etherCatDevice->readSDO_String(0x9009 + getCanOpenIndexOffset(), 0x4, orderCode, 64, "Order Code")) return false;
			 char motorContruction[64];
			 if(!etherCatDevice->readSDO_String(0x9009 + getCanOpenIndexOffset(), 0x5, motorContruction, 64, "Motor Construction")) return false;
			 char brakeType[64];
			 if(!etherCatDevice->readSDO_String(0x9009 + getCanOpenIndexOffset(), 0x1B, brakeType, 64, "Brake Type")) return false;
			 
			 uint32_t encoderWorkingRangeRevolutions;
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x13, encoderWorkingRangeRevolutions, "Encoder Working Range")) return false;
			 uint32_t encoderOperatingMinutes;
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x1C, encoderOperatingMinutes, "Encoder Operation Minute Counter")) return false;
			 
			 uint8_t singleturnBits;
			 if(!etherCatDevice->readSDO_U8(0x8000 + getCanOpenIndexOffset(), 0x12, singleturnBits, "Singleturn Resolution")) return false;
			 uint8_t multiturnBits;
			 if(!etherCatDevice->readSDO_U8(0x8000 + getCanOpenIndexOffset(), 0x13, multiturnBits, "Multiturn Resolution")) return false;
			 
			 uint32_t motorMaxCurrentmA;
			 if(!etherCatDevice->readSDO_U32(0x8011 + getCanOpenIndexOffset(), 0x11, motorMaxCurrentmA, "Max Motor Current")) return false;
			 uint32_t motorRatedCurrentmA;
			 if(!etherCatDevice->readSDO_U32(0x8011 + getCanOpenIndexOffset(), 0x12, motorRatedCurrentmA, "Rated Motor Current")) return false;
			 uint32_t motorSpeedLimitationRpm;
			 if(!etherCatDevice->readSDO_U32(0x8011 + getCanOpenIndexOffset(), 0x1B, motorSpeedLimitationRpm, "Motor Speed Limitation")) return false;
			 uint32_t torqueContant;
			 if(!etherCatDevice->readSDO_U32(0x8011 + getCanOpenIndexOffset(), 0x16, torqueContant, "Torque Constant")) return false;
			 
			 uint32_t velocityEncoderResolution;
			 if(!etherCatDevice->readSDO_U32(0x9010 + getCanOpenIndexOffset(), 0x14, velocityEncoderResolution, "Velocity Encoder Resolution")) return false;
			 uint32_t positionEncoderResolution;
			 if(!etherCatDevice->readSDO_U32(0x9010 + getCanOpenIndexOffset(), 0x15, positionEncoderResolution, "Position Encoder Resolution")) return false;
			 
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
			 Logger::info("	Torque Constant: {}mNm/A", torqueContant);
			 Logger::info("	Encoder Resolution: {}bits singleturn / {}bits multiturn", singleturnBits, multiturnBits);
			 Logger::info("	Encoder Working Range: {}rev", encoderWorkingRangeRevolutions);
			 Logger::info("	Operating Hours: {}h", encoderOperatingMinutes / 60);
			 
			 firstSetupProgress.write("Finishing");
			 
			 //disable autoconfig for future motor connections
			 if(!etherCatDevice->writeSDO_U8(0x8008 + getCanOpenIndexOffset(), 0x1, 0, "Enable Autoconfig")) return false;
			 if(!etherCatDevice->writeSDO_U8(0x8008 + getCanOpenIndexOffset(), 0x2, 0, "Reconfig Identical Motor")) return false;
			 if(!etherCatDevice->writeSDO_U8(0x8008 + getCanOpenIndexOffset(), 0x3, 0, "Reconfig Non-Identical Motor")) return false;
			 
			 //set position offset source to encoder memory
			 if(!etherCatDevice->writeSDO_U8(0x8000 + getCanOpenIndexOffset(), 0xD, 1, "Position Offset Source")) return false;
						 
			 /*
			 //Select Info Data 1 : Driver Errors
			 if(!etherCatDevice->writeSDO_U8(0x8010 + getCanOpenIndexOffset(), 0x39, 5, "Select Info Data 1")) return false;
			 
			 //Select Info Data 2 : Digital Input Levels
			 if(!etherCatDevice->writeSDO_U8(0x8010 + getCanOpenIndexOffset(), 0x3A, 10, "Select Info Data 2")) return false;
			 
			 //Set Feature bits to RMS Current values + Increased Output Current
			 if(!etherCatDevice->writeSDO_U32(0x8010 + getCanOpenIndexOffset(), 0x54, 3, "Feature Bits")) return false;
			 */
			  
			  
			 //set REFERENCED field
			 //It will get set to false/0x0 if a motor with a different serial number or a motor with single turn encoder is connected.
			 //This way we can detect if the motor changed and force first setup again.
			 if(!etherCatDevice->writeSDO_U8(0x8000 + getCanOpenIndexOffset(), 0x2, 1)) return false;
			 
			 motorNameplate.ratedCurrent_amps = double(motorRatedCurrentmA) / 1000.0;
			 motorNameplate.maxCurrent_amps = double(motorMaxCurrentmA) / 1000.0;
			 motorNameplate.maxVelocity_rps = double(motorSpeedLimitationRpm) / 60.0;
			 motorNameplate.workingRange_rev = encoderWorkingRangeRevolutions;
			 motorNameplate.velocityResolution_rps = velocityEncoderResolution;
			 motorNameplate.positionResolution_rev = positionEncoderResolution;
			 motorNameplate.torqueConstant_mNmpA = torqueContant;
			 motorNameplate.b_hasBrake = false; //TODO: detect motor brake
			 motorNameplate.motorType = motorType;
			 motorNameplate.serialNumber = serialNumber;
			 motorNameplate.b_motorIdentified = true;
			 
			 updateProprieties();
			 
			 return true;
		 };
		 
		 if(doFirstSetup()) {
			 firstSetupProgress.write("Done");
			 Logger::info("First Setup Finished Successfully.");
		 }
		 else {
			 firstSetupProgress.write("Failed");
			 Logger::warn("First Setup Failed");
		 }
	 });
	
	 worker.detach();
}


void EL722x_Actuator::resetEncoderPosition(){
	 std::thread worker([this](){
		
		 auto overrideEncoderPosition = [this]() -> bool {
		
			 resetEncoderPositionProgress.write("Starting");
			 
			 if(!etherCatDevice->isStateOperational()) {
				 resetEncoderPositionProgress.write("Encoder cannot be reset while the fieldbus is not running");
				 return false;
			 }
			 
			 uint8_t fbPositionValid;
			 if(!etherCatDevice->readSDO_U8(0x6000 + getCanOpenIndexOffset(), 0xE, fbPositionValid) || fbPositionValid != 0x0){
				 Logger::warn("Encoder is Offline or being identified.");
				 return false;
			 }

			 uint32_t positionEncoderResolution;
			 if(!etherCatDevice->readSDO_U32(0x9010 + getCanOpenIndexOffset(), 0x15, positionEncoderResolution, "Position Encoder Resolution")) return false;
			 uint32_t encoderWorkingRangeRevolutions;
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x13, encoderWorkingRangeRevolutions, "Encoder Working Range")) return false;
			 uint32_t positionActual;
			 if(!etherCatDevice->readSDO_U32(0x6000 + getCanOpenIndexOffset(), 0x11, positionActual, "Encoder Position")) return false;
			 uint32_t offsetActual;
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x20, offsetActual, "Encoder Position Offset")) return false;
			 
			 int maxOffset = positionEncoderResolution * encoderWorkingRangeRevolutions;
			 
			 //positionActual = rawPosition - currentOffset
			 //rawPosition = positionActual + currentOffset
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
			 if(!etherCatDevice->writeSDO_S16(0xB001 + getCanOpenIndexOffset(), 0x1, 16, "OCT Interface Command 'Write Encoder Position Offset'")) return false;
					 
			 //Set Data Buffer containing new position offset
			 uint32_t buffer[8] = {offset,0,0,0,0,0,0,0};
			 if(1 != ec_SDOwrite(etherCatDevice->getSlaveIndex(), 0xB001 + getCanOpenIndexOffset(), 0x6, false, 32, buffer, EC_TIMEOUTSAFE)) {
				 Logger::warn("Could not upload encoder position offset to data buffer");
				 return false;
			 }
			 
			 //Execute Command
			 if(!etherCatDevice->writeSDO_S16(0xB001 + getCanOpenIndexOffset(), 0x5, 1, "Execute OCT Interface Command")) return false;
			 
			 
			 resetEncoderPositionProgress.write("Waiting for encoder");
			 
			 //Wait for command status update
			 double commandExecuteTime = Timing::getProgramTime_seconds();
			 while(true){
				 std::this_thread::sleep_for(std::chrono::milliseconds(100));
				 int16_t result;
				 if(etherCatDevice->readSDO_S16(0xB001 + getCanOpenIndexOffset(), 0x5, result)){
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
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x20, offsetReadback, "Encoder Position Offset")) return false;
			 
			 if(offsetReadback != offset){
				 Logger::warn("Encoder offset write succeeded but readback was not identical to request.");
				 Logger::warn("Requested Offset: {}  Readback: {}", offset, offsetReadback);
				 return false;
			 }
			 
			 return true;
			 
		 };
		 
		 if(overrideEncoderPosition()) {
			 resetEncoderPositionProgress.write("Done");
			 Logger::info("Encoder Position Reset Succeeded.");
		 }
		 else {
			 resetEncoderPositionProgress.write("Failed");
			 Logger::warn("Encoder Position Reset Failed.");
		 }
		 
		 
	 });
	 worker.detach();
}


void EL722x_Actuator::uploadParameters(){
	
	std::thread worker([this](){
		
		auto uploadParameters = [this]()->bool{
			
			uploadParameterStatus.write("Starting");
			
			uint16_t torqueLimitation = 1000.0 * driveSettings.currentLimit->value / motorNameplate.ratedCurrent_amps;
			if(!etherCatDevice->writeSDO_U16(0x7010 + getCanOpenIndexOffset(), 0xB, torqueLimitation, "Torque Limitation")) return false;
			
			if(!etherCatDevice->writeSDO_U8(0x8010 + getCanOpenIndexOffset(), 0x65, driveSettings.invertDirection->value, "Invert Direction")) return false;
			
			uint32_t followingErrorWindow = driveSettings.positionFollowingErrorWindow->value * motorNameplate.positionResolution_rev;
			if(!etherCatDevice->writeSDO_U32(0x8010 + getCanOpenIndexOffset(), 0x50, followingErrorWindow, "Position Following Error Window")) return false;
			
			if(!etherCatDevice->writeSDO_U16(0x8010 + getCanOpenIndexOffset(), 0x51, driveSettings.positionFollowingErrorTimeout->value, "Position Following Error Timeout")) return false;
			
			if(!etherCatDevice->writeSDO_U16(0x8010 + getCanOpenIndexOffset(), 0x52, driveSettings.faultReaction->value, "Fault Reaction Option Code")) return false;
			
			if(driveSettings.faultReaction->value == driveSettings.option_faultReaction_HaltRamp.getInt()){
				//in 0.1rad/s^2 increments
				uint32_t haltRampDeceleration = driveSettings.haltRampDeceleration->value * M_PI * 2.0 * 10.0;
				if(!etherCatDevice->writeSDO_U32(0x8010 + getCanOpenIndexOffset(), 0x49, haltRampDeceleration, "Halt Ramp Deceleration")) return false;
			}
			
			return true;
		};
		
		if(uploadParameters()) {
			uploadParameterStatus.write("Done");
			Logger::info("Driver Parameter Upload Succeeded.");
		}
		else {
			uploadParameterStatus.write("Failed");
			Logger::warn("Driver Parameter Upload Failed.");
		}
		
	});
	
	worker.detach();
}





bool EL722x_Actuator::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	XMLElement* motorNameplateXML = xml->InsertNewChildElement("MotorNameplate");
	motorNameplate.save(motorNameplateXML);
	
	XMLElement* driveSettingsXML = xml->InsertNewChildElement("DriveSettings");
	driveSettings.save(driveSettingsXML);
	return true;
}

bool EL722x_Actuator::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	if(XMLElement* motorNameplateXML = xml->FirstChildElement("MotorNameplate")) motorNameplate.load(motorNameplateXML);
	if(XMLElement* driveSettingsXML = xml->FirstChildElement("DriveSettings")) driveSettings.load(driveSettingsXML);
	updateProprieties();
	return true;
}


bool EL722x_Actuator::MotorNameplate::save(tinyxml2::XMLElement* parent){
	using namespace tinyxml2;
	parent->SetAttribute("IsIdentified", b_motorIdentified);
	parent->SetAttribute("RatedCurrentAmps", ratedCurrent_amps);
	parent->SetAttribute("MaxCurrentAmps", maxCurrent_amps);
	parent->SetAttribute("MaxVelocityRps", maxVelocity_rps);
	parent->SetAttribute("WorkingRangeRev", workingRange_rev);
	parent->SetAttribute("VelocityResolutionRps", velocityResolution_rps);
	parent->SetAttribute("PositionResolutionRev", positionResolution_rev);
	parent->SetAttribute("TorqueConstant", torqueConstant_mNmpA);
	parent->SetAttribute("HasBrake", b_hasBrake);
	parent->SetAttribute("MotorType", motorType.c_str());
	parent->SetAttribute("SerialNumber", serialNumber.c_str());
	return true;
}

bool EL722x_Actuator::MotorNameplate::load(tinyxml2::XMLElement* parent){
	using namespace tinyxml2;
	parent->QueryAttribute("IsIdentified", &b_motorIdentified);
	parent->QueryAttribute("MaxCurrentAmps", &maxCurrent_amps);
	parent->QueryAttribute("RatedCurrentAmps", &ratedCurrent_amps);
	parent->QueryAttribute("MaxVelocityRps", &maxVelocity_rps);
	parent->QueryAttribute("WorkingRangeRev", &workingRange_rev);
	parent->QueryAttribute("VelocityResolutionRps", &velocityResolution_rps);
	parent->QueryAttribute("PositionResolutionRev", &positionResolution_rev);
	parent->QueryAttribute("TorqueConstant", &torqueConstant_mNmpA);
	parent->QueryAttribute("HasBrake", &b_hasBrake);
	const char* motorTypeString;
	if(parent->QueryAttribute("MotorType", &motorTypeString) == XML_SUCCESS) motorType = motorTypeString;
	const char* serialNumberString;
	if(parent->QueryAttribute("SerialNumber", &serialNumberString) == XML_SUCCESS) serialNumber = serialNumberString;
	return true;
}



bool EL722x_Actuator::DriveSettings::save(tinyxml2::XMLElement* parent){
	velocityLimit->save(parent);
	accelerationLimit->save(parent);
	currentLimit->save(parent);
	invertDirection->save(parent);
	positionFollowingErrorWindow->save(parent);
	positionFollowingErrorTimeout->save(parent);
	faultReaction->save(parent);
	haltRampDeceleration->save(parent);
	return true;
}

bool EL722x_Actuator::DriveSettings::load(tinyxml2::XMLElement* parent){
	velocityLimit->load(parent);
	accelerationLimit->load(parent);
	currentLimit->load(parent);
	invertDirection->load(parent);
	positionFollowingErrorWindow->load(parent);
	positionFollowingErrorTimeout->load(parent);
	faultReaction->load(parent);
	haltRampDeceleration->load(parent);
	return true;
}


std::string EL722x_Actuator::getStatusString(){
	if(etherCatDevice->isOffline()) return "Drive is Offline.";
	if(!etherCatDevice->isStateOperational()) return "Drive is not in Operational State.";
	if(!processData.b_motorConnected) return "Motor is not connected.";
	//if(actuatorProcessData.b_isEmergencyStopActive) return "STO is active.";
	if(statusWord.fault){
		std::string faultMsg = "Drive has one or more errors:\n";
		std::string errors = "";
		if(driverErrors.adc)				errors += "	-ADC Error\n";
		if(driverErrors.overcurrent) 		errors += "	-Overcurrent error\n";
		if(driverErrors.undervoltage) 		errors += "	-Undervoltage error\n";
		if(driverErrors.overvoltage) 		errors += "	-Overvoltage error\n";
		if(driverErrors.overtemperature)	errors += "	-Overtemperature error\n";
		if(driverErrors.i2tAmplifier) 		errors += "	-I2tAmplifier error\n";
		if(driverErrors.i2tMotor) 			errors += "	-I2tMotor error\n";
		if(driverErrors.encoder) 			errors += "	-Encoder error\n";
		if(driverErrors.watchdog) 			errors += "	-Watchdog error\n";
		if(errors == "") 									errors += "	-Generic error (download diagnostics for more information)\n";
		faultMsg += errors;
		faultMsg.pop_back(); //remove last line return from message
		return faultMsg;
	}
	if(isEnabled()) return "Drive is Enabled.";
	if(isReady()) return "Drive is Ready.";
	if(isEnabling()) return "Drive is Enabling...";
	if(state == DeviceState::DISABLING) return "Drive is Disabling...";
	return "Unknown State";
}


void EL722x_Actuator::gui(){
	
	if(ImGui::BeginTabBar("##ActuatorTabBar")){
		
		if(ImGui::BeginTabItem("Control")){
			
			auto tableRowBool = [](std::string fieldName, bool data){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", fieldName.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", data ? "Yes" : "No");
			};

			auto tableRowInt = [](std::string fieldName, int data, std::string suffix){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", fieldName.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%i %s", data, suffix.c_str());
			};

			auto tableRowDouble = [](std::string fieldName, double data, std::string suffix){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", fieldName.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f %s", data, suffix.c_str());
			};

			auto tableRowString = [](std::string fieldName, std::string& data){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", fieldName.c_str());
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", data.c_str());
			};

			if(ImGui::CollapsingHeader("Motor Nameplate")){
				if(ImGui::BeginTable("##MotorNameplate", 2, ImGuiTableFlags_Borders)){
					tableRowBool("Motor Identified", motorNameplate.b_motorIdentified);
					tableRowString("Type", motorNameplate.motorType);
					tableRowString("Serial Number", motorNameplate.serialNumber);
					tableRowDouble("Rated Current", motorNameplate.ratedCurrent_amps, "A");
					tableRowDouble("Max Current", motorNameplate.maxCurrent_amps, "A");
					tableRowDouble("Max Velocity", motorNameplate.maxVelocity_rps, "rev/s");
					tableRowDouble("Working Range", motorNameplate.workingRange_rev, "rev");
					tableRowInt("Velocity Encoder Resolution", motorNameplate.velocityResolution_rps, "inc");
					tableRowInt("Position Encoder Resolution", motorNameplate.positionResolution_rev, "inc");
					tableRowDouble("Torque Constant", motorNameplate.torqueConstant_mNmpA, "mNm/A");
					tableRowBool("Has Brake", motorNameplate.b_hasBrake);
					ImGui::EndTable();
				};
			}

			if(ImGui::CollapsingHeader("Status Word")){
				if(ImGui::BeginTable("##StatusWord", 2, ImGuiTableFlags_Borders)){
					tableRowBool("Ready To Switch On", statusWord.readyToSwitchOn);
					tableRowBool("Switched On", statusWord.switchedOn);
					tableRowBool("Operation Enabled", statusWord.operationEnabled);
					tableRowBool("Fault", statusWord.fault);
					tableRowBool("Quickstop", statusWord.quickstop);
					tableRowBool("Switch On Disabled", statusWord.switchOnDisabled);
					tableRowBool("Warning", statusWord.warning);
					tableRowBool("TxPdo Toggle", statusWord.TxPdoToggle);
					tableRowBool("Internal Limit Active", statusWord.internalLimitActive);
					tableRowBool("Command Value Followed", statusWord.commandValueFollowed);
					ImGui::EndTable();
				}
			}

			if(ImGui::CollapsingHeader("Control Word")){
				if(ImGui::BeginTable("##ControlWord", 2, ImGuiTableFlags_Borders)){
					tableRowBool("Switch On", controlWord.switchOn);
					tableRowBool("Enable Voltage", controlWord.enableVoltage);
					tableRowBool("Quickstop", controlWord.quickstop);
					tableRowBool("Enable Operation", controlWord.enableOperation);
					tableRowBool("Fault Reset", controlWord.faultReset);
					ImGui::EndTable();
				}
			}

			if(ImGui::CollapsingHeader("Errors")){
				if(ImGui::BeginTable("##Errors", 2, ImGuiTableFlags_Borders)){
					tableRowBool("Motor Connected", processData.b_motorConnected);
					//tableRowBool("STO Active", actuator->actuatorProcessData.b_isEmergencyStopActive);
					tableRowBool("ADC Error", driverErrors.adc);
					tableRowBool("Overcurrent Error", driverErrors.overcurrent);
					tableRowBool("Undervoltage Error", driverErrors.undervoltage);
					tableRowBool("Overvoltage Error", driverErrors.overvoltage);
					tableRowBool("Overtemperature Error", driverErrors.overtemperature);
					tableRowBool("I2tAmplifier Error", driverErrors.i2tAmplifier);
					tableRowBool("I2tMotor Error", driverErrors.i2tMotor);
					tableRowBool("Encoder Error", driverErrors.encoder);
					tableRowBool("Watchdog Error", driverErrors.watchdog);
					ImGui::EndTable();
				}
			}

			if(ImGui::CollapsingHeader("Warnings")){
				if(ImGui::BeginTable("##Warnings", 2, ImGuiTableFlags_Borders)){
					tableRowBool("Undervoltage Warning", driverWarnings.undervoltage);
					tableRowBool("Overvoltage Warning", driverWarnings.overvoltage);
					tableRowBool("Overtemperature Warning", driverWarnings.overtemperature);
					tableRowBool("I2tAmplifier Warning", driverWarnings.i2tAmplifier);
					tableRowBool("I2tMotor Warning", driverWarnings.i2tMotor);
					tableRowBool("Encoder Warning", driverWarnings.encoder);
					ImGui::EndTable();
				}
			}

			ImGui::Separator();

			if(isEnabled()){
				if(ImGui::Button("Disable")) disable();
			}
			else {
				ImGui::BeginDisabled(!isReady());
				if(ImGui::Button("Enable")) enable();
				ImGui::EndDisabled();
			}

			if(ImGui::SliderFloat("##TargetVelocity", &velocitySliderValue, -getVelocityLimit(), getVelocityLimit())){
				setVelocityTarget(velocitySliderValue);
			}
			if(ImGui::IsItemDeactivatedAfterEdit()) {
				setVelocityTarget(0.0);
				velocitySliderValue = 0.0;
			}

			float velocityNormalized = getVelocityNormalized();
			float positionNormalized = getPositionNormalizedToWorkingRange();
			float followingErrorNormalized = getFollowingErrorNormalized();
			float effortNormalized = getEffort();
			
			ImVec2 progressSize(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
			char progressString[64];
			snprintf(progressString, 64, "Position: %.3frev", getPosition());
			ImGui::ProgressBar(positionNormalized, progressSize, progressString);
			snprintf(progressString, 64, "Following Error: %.3frev", getFollowingError());
			ImGui::ProgressBar(followingErrorNormalized, progressSize, progressString);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, velocityNormalized < 0.0 ? Colors::red : Colors::green);
			snprintf(progressString, 64, "Velocity: %.1frev/s", getVelocity());
			ImGui::ProgressBar(std::abs(velocityNormalized), progressSize, progressString);
			ImGui::PopStyleColor();
			snprintf(progressString, 64, "Torque: %.2fNm", getForce());
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::orange);
			ImGui::ProgressBar(getEffort(), progressSize, progressString);
			ImGui::PopStyleColor();
			snprintf(progressString, 64, "Effort: %.1f%%", effortNormalized * 100.0);
			ImGui::ProgressBar(effortNormalized, progressSize, progressString);

			//ImGui::Text("Status: %s", getStatusString().c_str());
			
			
			
			ImGui::Checkbox("switchOn", &controlWord.switchOn);
			ImGui::Checkbox("enableVoltage", &controlWord.enableVoltage);
			ImGui::Checkbox("quickstop", &controlWord.quickstop);
			ImGui::Checkbox("enableOperation", &controlWord.enableOperation);
			ImGui::Checkbox("faultReset", &controlWord.faultReset);
			
			
			
			
			ImGui::EndTabItem();
		}
		
		if(ImGui::BeginTabItem("Settings")){
			
			
			float frameHeight = ImGui::GetFrameHeight();
			ImGui::PushFont(Fonts::sansBold15);
			ImVec2 offset(ImGui::GetStyle().CellPadding.y, (frameHeight - ImGui::GetTextLineHeight()) / 2.0);
			ImGui::PopFont();
			
			auto parameterTableRow = [&](std::shared_ptr<Parameter> param){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(ImVec2(cursorPos.x + offset.x, cursorPos.y + offset.y));
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("%s", param->getName());
				ImGui::PopFont();
				ImGui::TableSetColumnIndex(1);
				param->gui();
			};
			
			if(ImGui::BeginTable("##driveSettings", 2, ImGuiTableFlags_RowBg)){
				parameterTableRow(driveSettings.velocityLimit);
				parameterTableRow(driveSettings.accelerationLimit);
				parameterTableRow(driveSettings.currentLimit);
				parameterTableRow(driveSettings.invertDirection);
				parameterTableRow(driveSettings.positionFollowingErrorWindow);
				parameterTableRow(driveSettings.positionFollowingErrorTimeout);
				parameterTableRow(driveSettings.faultReaction);
				parameterTableRow(driveSettings.haltRampDeceleration);
				ImGui::EndTable();
			}
			
			if(ImGui::Button("First Setup")) firstSetup();
			ImGui::SameLine();
			ImGui::Text("%s", firstSetupProgress.read().c_str());
			if(ImGui::Button("Reset Encoder")) resetEncoderPosition();
			ImGui::SameLine();
			ImGui::Text("%s", resetEncoderPositionProgress.read().c_str());
			if(ImGui::Button("Upload Parameters")) uploadParameters();
			ImGui::SameLine();
			ImGui::Text("%s", uploadParameterStatus.read().c_str());
			
			
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}
	
		
}

