#include "EL722xActuator.h"

#include "../EL7222-0010.h"

void EL722x_Actuator::initialize(){
	auto thisActuatorInterface = std::static_pointer_cast<ActuatorInterface>(shared_from_this());
	actuatorPin->assignData(thisActuatorInterface);
	driveSettings.accelerationLimit->addEditCallback([this](){ updateProprieties(); });
	driveSettings.velocityLimit->addEditCallback([this](){ updateProprieties(); });
	driveSettings.positionFollowingErrorWindow->addEditCallback([this](){ updateProprieties(); });
}

void EL722x_Actuator::updateProprieties(){
	actuatorConfig.accelerationLimit = std::abs(driveSettings.accelerationLimit->value);
	actuatorConfig.velocityLimit = std::abs(driveSettings.velocityLimit->value);
	actuatorConfig.followingErrorLimit = std::abs(driveSettings.positionFollowingErrorWindow->value);
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
	
	processData.b_motorConnected = !bool(txPdo.fbStatus & (0x1 << 13));
   
	actuatorProcessData.followingErrorActual = double(txPdo.followingErrorActualValue) / motorNameplate.positionResolution_rev;
	double actualCurrent = std::abs(motorNameplate.ratedCurrent_amps * double(txPdo.torqueActualValue) / 1000.0);
	actuatorProcessData.effortActual = actualCurrent / driveSettings.currentLimit->value;
   
	feedbackProcessData.positionActual = double(txPdo.fbPosition) / motorNameplate.positionResolution_rev;
	feedbackProcessData.velocityActual = double(txPdo.velocityActualValue) / motorNameplate.velocityResolution_rps;
	feedbackProcessData.forceActual = double(txPdo.torqueActualValue) / 1000.0 * motorNameplate.ratedCurrent_amps * motorNameplate.torqueConstant_mNmpA / 1000.0;
	
	if(!processData.b_motorConnected)										state = DeviceState::NOT_READY;
	else if(statusWord.operationEnabled && !processData.b_enableTarget)		state = DeviceState::DISABLING;
	else if(!statusWord.operationEnabled && processData.b_enableTarget)		state = DeviceState::ENABLING;
	else if(statusWord.operationEnabled)									state = DeviceState::ENABLED;
	else 																	state = DeviceState::READY;
	
	
	//fault event detection
	if(!processData.b_hadFault && statusWord.fault) {
		Logger::warn("[{}] Fault Detected, downloading error code...", getName());
		//since we can't have an error message in the process data
		//we request an asynchronous download of the last diagnostics message by the drive
		//this method will copy the last error message and raise a flag when it's done
		std::static_pointer_cast<EL7222_0010>(etherCatDevice)->downloadLatestDiagnosticsMessage(&lastErrorTextID, &b_newErrorID);
	}
	else if(processData.b_hadFault && !statusWord.fault) Logger::info("[{}] Fault Cleared", getName());
	processData.b_hadFault = statusWord.fault;
	
	//check if a new error id is available from download thread
	if(b_newErrorID){
		b_newErrorID = false;
		lastErrorString = std::static_pointer_cast<EL7222_0010>(etherCatDevice)->getDiagnosticsStringFromTextID(lastErrorTextID);
		Logger::warn("[{}] Error {:x} : {}", getName(), lastErrorTextID, lastErrorString);
	}
   
}

void EL722x_Actuator::writeOutputs(){
	
	
	if(feedbackProcessData.b_overridePosition){
		feedbackProcessData.b_overridePosition = false;
		feedbackProcessData.b_positionOverrideBusy = true;
		actuatorProcessData.b_disable = true;
	}
	if(feedbackProcessData.b_positionOverrideBusy && !isEnabled()){
		resetEncoderPosition(&feedbackProcessData.b_positionOverrideBusy, &feedbackProcessData.b_positionOverrideSucceeded);
		feedbackProcessData.b_positionOverrideBusy = false;
		feedbackProcessData.b_positionOverrideSucceeded = true;
	}
	if(feedbackProcessData.b_positionOverrideSucceeded && !feedbackProcessData.b_positionOverrideBusy){
		//we could automatically reenable after homing, but it's not really needed right now
		//actuatorProcessData.b_enable = true;
	}
	
	
	//reset fault request bit
	if(controlWord.faultReset) controlWord.faultReset = false;
	 
	//react to power state commands
	if(actuatorProcessData.b_enable){
		actuatorProcessData.b_enable = false;
		//start enable request and mark down time of request
		processData.b_waitingForEnable = true;
		processData.enableRequestTime_nanos = etherCatDevice->cycleProgramTime_nanoseconds;
		//clear fault on enable request
		if(statusWord.fault) controlWord.faultReset = true;
	}
	if(actuatorProcessData.b_disable){
		actuatorProcessData.b_disable = false;
		processData.b_waitingForEnable = false;
		processData.b_enableTarget = false;
	}

	//handle drive enable process
	if(processData.b_waitingForEnable){
		//don't enable while there is a fault
		processData.b_enableTarget = !statusWord.fault;
		//check for enable timeout
		if(etherCatDevice->cycleProgramTime_nanoseconds - processData.enableRequestTime_nanos > 200'000'000){
			Logger::warn("[{}] Enable request timed out", getName());
			processData.b_waitingForEnable = false;
			processData.b_enableTarget = false;
		}
		//check if drive is enabled
		else if(statusWord.operationEnabled){
			processData.b_waitingForEnable = false;
			Logger::trace("[{}] Drive Enabled (took {}ms)", getName(), double(etherCatDevice->cycleProgramTime_nanoseconds - processData.enableRequestTime_nanos) / 1000000.0);
		}
	}
	//react to unexpected drive disable
	else if(processData.b_enableTarget && !statusWord.operationEnabled){
		processData.b_enableTarget = false;
		processData.b_waitingForEnable = false;
		Logger::warn("[{}] Drive was Disabled", getName());
	}
	
	
	//DS402 State Machine Control
	if(processData.b_enableTarget && statusWord.switchedOn){
		//third: enable operation
		controlWord.enableVoltage = true;
		controlWord.switchOn = true;
		controlWord.enableOperation = true;
	}
	else if(processData.b_enableTarget && statusWord.readyToSwitchOn){
		//second: switch on
		controlWord.enableVoltage = true;
		controlWord.switchOn = true;
		controlWord.enableOperation = false;
	}
	else if(processData.b_enableTarget && statusWord.switchOnDisabled){
		//first: enable voltage
		controlWord.enableVoltage = true;
		controlWord.switchOn = false;
		controlWord.enableOperation = false;
	}
	else{
		//if the drive is disabled, we go to this state
		//another state doesn't allow us to reset encoder position offset
		controlWord.enableVoltage = false;
		controlWord.switchOn = false;
		controlWord.enableOperation = false;
	}
	
	 
	//construct control word
	rxPdo.controlWord = 0x0;
	if(controlWord.switchOn) 			rxPdo.controlWord |= 0x1 << 0;
	if(controlWord.enableVoltage) 		rxPdo.controlWord |= 0x1 << 1;
	if(!controlWord.quickstop) 			rxPdo.controlWord |= 0x1 << 2;
	if(controlWord.enableOperation) 	rxPdo.controlWord |= 0x1 << 3;
	if(controlWord.faultReset)			rxPdo.controlWord |= 0x1 << 7;
	 
	//set operating mode
	if(!actuatorPin->isConnected()) {
		rxPdo.modeOfOperationSelection = 0x9;
		
		//update manual velocity profile generator
		double velocityDelta_rps = std::abs(driveSettings.accelerationLimit->value * etherCatDevice->cycleDeltaTime_seconds);
		if(processData.manualVelocityProfile_rps < processData.manualVelocityTarget_rps){
			processData.manualVelocityProfile_rps += velocityDelta_rps;
			if(processData.manualVelocityProfile_rps > processData.manualVelocityTarget_rps){
				processData.manualVelocityProfile_rps = processData.manualVelocityTarget_rps;
			}
		}
		else if(processData.manualVelocityProfile_rps > processData.manualVelocityTarget_rps){
			processData.manualVelocityProfile_rps -= velocityDelta_rps;
			if(processData.manualVelocityProfile_rps < processData.manualVelocityTarget_rps){
				processData.manualVelocityProfile_rps = processData.manualVelocityTarget_rps;
			}
		}
		if(!isEnabled()) processData.manualVelocityProfile_rps = 0.0;
		rxPdo.targetVelocity = processData.manualVelocityProfile_rps * motorNameplate.velocityResolution_rps;
	}
	else{
		processData.manualVelocityTarget_rps = 0.0;
		processData.manualVelocityProfile_rps = 0.0;
		switch(actuatorProcessData.controlMode){
			case ActuatorInterface::ControlMode::POSITION:	rxPdo.modeOfOperationSelection = 0x8; break;
			case ActuatorInterface::ControlMode::VELOCITY:	rxPdo.modeOfOperationSelection = 0x9; break;
			case ActuatorInterface::ControlMode::FORCE: 	rxPdo.modeOfOperationSelection = 0xA; break;
		}
		rxPdo.targetVelocity = actuatorProcessData.velocityTarget * motorNameplate.velocityResolution_rps;
	}
	
	rxPdo.targetPosition = std::abs(actuatorProcessData.positionTarget) * motorNameplate.positionResolution_rev;
	if(actuatorProcessData.positionTarget < 0.0) rxPdo.targetPosition = UINT32_MAX - rxPdo.targetPosition;
	
}



//std::string firstSetupProgress = "";
bool EL722x_Actuator::readMotorNameplate(){
			 
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
	 
	 Logger::info("	--------- MOTOR {} ---------", channel);
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
	 
	 //set position offset source to encoder memory
	 if(!etherCatDevice->writeSDO_U8(0x8000 + getCanOpenIndexOffset(), 0xD, 1, "Position Offset Source")) return false;
	
	//set position offset source to drive memory
	//if(!etherCatDevice->writeSDO_U8(0x8000 + getCanOpenIndexOffset(), 0xD, 2, "Position Offset Source")) return false;
				 
	 //Select Info Data 1 : Digital Inputs
	 if(!etherCatDevice->writeSDO_U8(0x8010 + getCanOpenIndexOffset(), 0x39, 10, "Select Info Data 1")) return false;
	  
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
}


void EL722x_Actuator::resetEncoderPosition(bool* b_busy, bool* b_success){
	
	if(b_busy == nullptr || b_success == nullptr) return;
	*b_busy = true;
	*b_success = false;
	
	 std::thread worker([this, b_busy, b_success](){
		
		 auto overrideEncoderPosition = [this, b_busy, b_success]() -> bool {
			 
			 if(isEnabled()) {
				 resetEncoderPositionProgress.write("Cannot reset encoder with power state enabled");
				 return false;
			 }
			 if(!etherCatDevice->isStateOperational()) {
				 resetEncoderPositionProgress.write("Cannot reset encoder while fieldbus is not running");
				 return false;
			 }
		
			 resetEncoderPositionProgress.write("Starting");
			 
			 uint8_t fbPositionValid;
			 if(!etherCatDevice->readSDO_U8(0x6000 + getCanOpenIndexOffset(), 0xE, fbPositionValid) || fbPositionValid != 0x0){
				 Logger::warn("Motor is Offline or being identified.");
				 return false;
			 }

			 uint32_t positionEncoderResolution; //increments per encoder revolution
			 if(!etherCatDevice->readSDO_U32(0x9010 + getCanOpenIndexOffset(), 0x15, positionEncoderResolution, "Position Encoder Resolution")) return false;
			 uint32_t encoderWorkingRangeRevolutions; //amount of turns
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x13, encoderWorkingRangeRevolutions, "Encoder Working Range")) return false;
			 uint32_t positionActual; //position feedback pdo
			 if(!etherCatDevice->readSDO_U32(0x6000 + getCanOpenIndexOffset(), 0x11, positionActual, "Encoder Position")) return false;
			 uint32_t currentEncoderOffset; //offset stored inside the motor nameplate
			 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x20, currentEncoderOffset, "Encoder Position Offset")) return false;
			 
			 //the offset should not be bigger than the entire resolution of the encoder
			 //for the motors we know, that resolution is 20bits ST + 12 bits multiturn
			 //so an offset greater than UINT32_MAX cannot exist
			 //in the case of our motors the following maxOffset evaluates to 0, which is equal to 2^32
			 //since the offset is also an uint32_t, any offset higher than that gets capped by the container bounds
			 //if there is ever an encoder with less resolution connected, this takes care of capping the max offset
			 uint32_t maxOffset = positionEncoderResolution * encoderWorkingRangeRevolutions;
			 
			 //positionActual = rawEncoderPosition - currentEncoderOffset
			 //rawEncoderPosition = positionActual + currentEncoderOffset
			 //rawEncoderPosition - newOffset = desiredPosition
			 //newOffset = rawEncoderPosition - desiredPosition
			 //if desired position == 0 then newOffset == rawEncoderPosition
			 
			 uint64_t rawPosition = positionActual + currentEncoderOffset;
			 if(maxOffset != 0) while(rawPosition >= maxOffset) rawPosition -= maxOffset;
			 uint32_t newEncoderOffset = uint32_t(rawPosition);
			 
			 Logger::info("Actual Position: {}", positionActual);
			 Logger::info("Current Offset: {}", currentEncoderOffset);
			 Logger::info("Raw Position: {}", rawPosition);
			 Logger::info("New Offset: {}", newEncoderOffset);
			 Logger::info("Starting encoder offset write...");
			 
			 //Set OCT Interface Command to "Write Encoder Position Offset"
			 if(!etherCatDevice->writeSDO_S16(0xB001 + getCanOpenIndexOffset(), 0x1, 16, "OCT Interface Command 'Write Encoder Position Offset'")) return false;
					 
			 //Set Data Buffer containing new position offset
			 uint32_t buffer[8] = {newEncoderOffset,0,0,0,0,0,0,0};
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
				 
				 uint32_t readback;
				 if(!etherCatDevice->readSDO_U32(0x9008 + getCanOpenIndexOffset(), 0x20, readback, "Encoder Position Offset")) return false;
				 if(readback == newEncoderOffset) break;
	
				 if(Timing::getProgramTime_seconds() - commandExecuteTime > 2.0){
					 Logger::warn("Encoder offset write timed out.");
					 return false;
				 }
			 }
			 
			 return true;
			 
		 };
		 
		 if(overrideEncoderPosition()) {
			 resetEncoderPositionProgress.write("Done");
			 Logger::info("Encoder Position Reset Succeeded.");
			 *b_success = true;
		 }
		 else {
			 resetEncoderPositionProgress.write("Failed");
			 Logger::warn("Encoder Position Reset Failed.");
			 *b_success = false;
		 }
		 *b_busy = false;
		 
		 
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
	if(!processData.b_motorConnected) return "Motor is not connected or is being identified";
	if(statusWord.fault) return "Fault: " + lastErrorString;
	if(isEnabled()) return "Drive is Enabled.";
	if(isReady()) return "Drive is Ready.";
	if(isEnabling()) return "Drive is Enabling...";
	if(state == DeviceState::DISABLING) return "Drive is Disabling...";
	return "Unknown State";
}


void EL722x_Actuator::gui(){
	
	if(ImGui::BeginTabBar("##ActuatorTabBar")){
		
		if(ImGui::BeginTabItem("Control")){

			double defaultFramePadding = ImGui::GetStyle().FramePadding.y;
			ImGui::GetStyle().FramePadding.y = ImGui::GetTextLineHeight() * 0.5;
			
			ImVec2 powerButtonSize(ImGui::GetTextLineHeight() * 5.0, ImGui::GetFrameHeight());
			ImVec4 powerButtonColor;
			if(isEnabled()) powerButtonColor = Colors::green;
			else if(!isOnline()) powerButtonColor = Colors::blue;
			else if(isReady()) powerButtonColor = Colors::yellow;
			else powerButtonColor = Colors::red;
			
			ImGui::PushStyleColor(ImGuiCol_Button, powerButtonColor);
			if(isEnabled()){
				if(ImGui::Button("Disable", powerButtonSize)) disable();
			}
			else {
				ImGui::BeginDisabled(!isReady());
				if(ImGui::Button("Enable", powerButtonSize)) enable();
				ImGui::EndDisabled();
			}
			ImGui::PopStyleColor();
			
			ImGui::SameLine();

			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
			char velocitySliderString[64];
			snprintf(velocitySliderString, 64, "Target Velocity: %.1frev/s", processData.manualVelocityTarget_rps);
			ImGui::BeginDisabled(!isEnabled());
			if(ImGui::SliderFloat("##TargetVelocity", &processData.manualVelocityTarget_rps, -getVelocityLimit(), getVelocityLimit(), velocitySliderString));
			if(ImGui::IsItemDeactivatedAfterEdit()) processData.manualVelocityTarget_rps = 0.0;
			ImGui::EndDisabled();
			ImGui::GetStyle().FramePadding.y = defaultFramePadding;

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

			ImGui::Text("Status: %s", getStatusString().c_str());
			
			if(ImGui::Button("Reset Encoder Position")) feedbackProcessData.b_overridePosition = true;
			ImGui::SameLine();
			ImGui::Text("%s", resetEncoderPositionProgress.read().c_str());
			
			ImGui::EndTabItem();
		}
		
		if(ImGui::BeginTabItem("Settings")){
			
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

			
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Motor Nameplate");
			ImGui::PopFont();
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
			
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Drive Settings");
			ImGui::PopFont();
			
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
			
			if(ImGui::Button("Upload Parameters")) uploadParameters();
			ImGui::SameLine();
			ImGui::Text("%s", uploadParameterStatus.read().c_str());
			
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}
	
		
}

