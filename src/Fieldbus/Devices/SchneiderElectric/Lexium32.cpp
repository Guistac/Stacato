#include <pch.h>

#include "Lexium32.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

void Lexium32::onConnection() {
	requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
}

void Lexium32::onDisconnection() {
	requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	servoMotor->state = DeviceState::OFFLINE;
	gpioDevice->state = DeviceState::OFFLINE;
	servoMotor->actuatorProcessData.b_isEmergencyStopActive = false;
}

void Lexium32::initialize() {

	auto thisLexiumDrive = std::dynamic_pointer_cast<Lexium32>(shared_from_this());
	servoMotor = std::make_shared<LexiumServoMotor>(thisLexiumDrive);
	gpioDevice = std::make_shared<LexiumGpio>(thisLexiumDrive);
		
	servoMotorPin->assignData(std::static_pointer_cast<ActuatorInterface>(servoMotor));
	gpioDevicePin->assignData(std::static_pointer_cast<GpioInterface>(gpioDevice));
	
	//working range
	updateEncoderWorkingRange();

    //node input data
	digitalOut0Pin->assignData(digitalOut0Value);
	digitalOut1Pin->assignData(digitalOut1Value);
	digitalOut2Pin->assignData(digitalOut2Value);
	
    addNodePin(digitalOut0Pin);
    addNodePin(digitalOut1Pin);
    addNodePin(digitalOut2Pin);

    //node output data
    addNodePin(servoMotorPin);
    addNodePin(actualLoadPin);
    addNodePin(actualPositionPin);
    addNodePin(actualVelocityPin);
    addNodePin(gpioDevicePin);
		
    addNodePin(digitalIn0Pin);
    addNodePin(digitalIn1Pin);
    addNodePin(digitalIn2Pin);
    addNodePin(digitalIn3Pin);
    addNodePin(digitalIn4Pin);
    addNodePin(digitalIn5Pin);

    rxPdoAssignement.addNewModule(0x1603);
    rxPdoAssignement.addEntry(0x6040, 0x0, 16, "DCOMcontrol", &ds402Control.controlWord);	//DS402 control word
    rxPdoAssignement.addEntry(0x6060, 0x0, 8, "DCOMopmode", &ds402Control.operatingMode);	//DS402 operating mode control
    rxPdoAssignement.addEntry(0x607A, 0x0, 32, "PPp_target", &PPp_target);					//Position Target
	rxPdoAssignement.addEntry(0x60FF, 0x0, 32, "PVv_target", &PVv_target);					//Velocity Target
	rxPdoAssignement.addEntry(0x6071, 0x0, 16, "PTtq_target", &PTtq_target);				//Torque Target
    rxPdoAssignement.addEntry(0x3008, 0x11, 16, "IO_DQ_set", &IO_DQ_set);					//Digital Outputs
	rxPdoAssignement.addEntry(0x3008, 0xA, 16, "BRK_release", &BRK_release);				//Manual Holding Brake Control
	
    txPdoAssignement.addNewModule(0x1A03);
    txPdoAssignement.addEntry(0x6041, 0x0, 16, "_DCOMstatus", &ds402Status.statusWord);		//DS402 status word
    txPdoAssignement.addEntry(0x6061, 0x0, 8, "_DCOMopmd_act", &ds402Status.operatingMode);	//DS402 operating mode display
    txPdoAssignement.addEntry(0x6064, 0x0, 32, "_p_act", &_p_act);							//Actual Position
	txPdoAssignement.addEntry(0x301E, 0x14, 32, "_p_dif_usr", &_p_dif_usr);					//Following Error
    txPdoAssignement.addEntry(0x606C, 0x0, 32, "_v_act", &_v_act);							//Actual Velocity
    txPdoAssignement.addEntry(0x301E, 0x3, 16, "_I_act", &_I_act);							//Actual Current
    txPdoAssignement.addEntry(0x603F, 0x0, 16, "_LastError", &_LastError);					//Current Error
	txPdoAssignement.addEntry(0x301C, 0x4, 16, "_actionStatus", &_actionStatus);			//error class / additional drive info
    txPdoAssignement.addEntry(0x3008, 0x1, 16, "_IO_act", &_IO_act);						//Digital Inputs
    txPdoAssignement.addEntry(0x3008, 0x26, 16, "_IO_STO_act", &_IO_STO_act);				//STO Status
	
	auto& fbcfg = servoMotor->feedbackConfig;
	fbcfg.b_supportsForceFeedback = true;
	fbcfg.b_supportsPositionFeedback = true;
	fbcfg.b_supportsVelocityFeedback = true;
	auto& acfg = servoMotor->actuatorConfig;
	acfg.b_supportsForceControl = false;
	acfg.b_supportsPositionControl = true;
	acfg.b_supportsVelocityControl = true;
	acfg.b_supportsEffortFeedback = true;
}






//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool Lexium32::startupConfiguration() {

    //===== Cia DS402 mandatory Startup Settings (According to Lexium32 EtherCAT module documentation)

    uint16_t CompParSyncMot_set = 1;
    if (!writeSDO_U16(0x3006, 0x3D, CompParSyncMot_set)) return false;

    uint16_t MOD_enable_set = 0;
    if (!writeSDO_U16(0x3006, 0x38, MOD_enable_set)) return false;

    uint16_t IOSigRespOfPS_set = 1;
    if (!writeSDO_U16(0x3006, 0x6, IOSigRespOfPS_set)) return false;

    int32_t ScalePOSdenom_set = positionUnitsPerRevolution;
    if (!writeSDO_S32(0x3006, 0x7, ScalePOSdenom_set)) return false;

    //setting becomes active after setting the numerator, so we set it last
    int32_t ScalePOSnum_set = 1;
    if (!writeSDO_S32(0x3006, 0x8, ScalePOSnum_set)) return false;

    uint16_t CTRL1_KFPp = 1000;
    if (!writeSDO_U16(0x3012, 0x6, CTRL1_KFPp)) return false;

    uint16_t CTRL2_KFPp = 1000;
    if (!writeSDO_U16(0x3013, 0x6, CTRL2_KFPp)) return false;

    //opmode cyclic synchronous position
    int8_t DCOMopmode_set = 8;
    if (!writeSDO_S8(0x6060, 0x0, DCOMopmode_set)) return false;

    uint32_t ECATinpshifttime_set = 250000;
    if (!writeSDO_U32(0x1C33, 0x3, ECATinpshifttime_set)) return false;

    //====== Custom User Startup Settings ======

    //Upload velocity unit scaling

    int32_t ScaleVELdenom = velocityUnitsPerRpm;
    if (!writeSDO_S32(0x3006, 0x21, ScaleVELdenom)) return false;

    int32_t ScaleVELnom = 1;
    if (!writeSDO_S32(0x3006, 0x22, ScaleVELnom)) return false;

    //units per rpm/s
    int32_t ScaleRAMPdenom = 1;
    if (!writeSDO_S32(0x3006, 0x30, ScaleRAMPdenom)) return false;

    int32_t ScaleRAMPnom = 1;
    if (!writeSDO_S32(0x3006, 0x31, ScaleRAMPnom)) return false;

	
	
	uint16_t _M_n_max; //max motor spec velocity in rpm
	if (!readSDO_U16(0x300D, 0x4, _M_n_max)) return false;
	maxMotorVelocity = (double)_M_n_max / 60.0;

	uint32_t CTRL_v_max = _M_n_max * velocityUnitsPerRpm; //Velocity Limit in usr_v units
	if (!writeSDO_U32(0x3011, 0x10, CTRL_v_max)) return false;
	
	
	
	//=============== Homing method for singleturn encoders =============
	
	int8_t HMmethod = 35;
	if(!writeSDO_S8(0x6098, 0x0, HMmethod)) return false;
	
	int32_t HMp_setP = 0;
	if(!writeSDO_S32(0x301B, 0x16, HMp_setP)) return false;

    //=============== PROCESS DATA ASSIGNEMENT ===============

    rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12);
    txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13);

    //=========================== TIMING AND SYNC CONFIGURATION ============================
/*
    //set interrupt routine for cyclic synchronous position mode
    //interval should be the same as the frame cycle time, and offset should be zero
    //the frame cycle time is offset 50% from dc_sync time (which is a integer multiple of the interval time)
    //by setting the sync0 event at 0 offset, maximum time offset is garanteed between the sync event and the frame receive time
    uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0;
    uint32_t sync0offset_nanoseconds = sync0Interval_nanoseconds / 2;
    ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);
	*/
	
    double maxCycleTime_millis = 10.0;
    double cycleTime_millis = std::floor(maxCycleTime_millis / EtherCatFieldbus::processInterval_milliseconds) * EtherCatFieldbus::processInterval_milliseconds;
    uint32_t cycleTime_nanos = cycleTime_millis * 1000000;
    uint32_t cycleOffset_nanos = EtherCatFieldbus::processInterval_milliseconds * 500000;
    
    ec_dcsync0(getSlaveIndex(), true, cycleTime_nanos, cycleOffset_nanos);
	 
    return true;
}




//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void Lexium32::readInputs() {

	//remember previous error to detect change
	uint16_t previousError = _LastError;
	
    txPdoAssignement.pullDataFrom(identity->inputs);
	
	if(b_isHoming){
		if(_p_act == 0) b_isHoming = false;
		_p_act = 0;
	}
	
	//read power state
	auto newPowerState = ds402Status.getPowerState();
	if(newPowerState != actualPowerState){
		std::string message = "Power State changed to " + std::string(Enumerator::getDisplayString(newPowerState));
		pushEvent(message.c_str(), false/*!DS402::isNominal(newPowerState)*/);
	}
	actualPowerState = newPowerState;
	
	
	//Read Error
	if(_LastError != previousError){
        if(_LastError == 0x0) pushEvent("Error Cleared", false);
		else{
			std::string message = "Error " + getErrorCodeString(_LastError);
			pushEvent(message.c_str(), true);
            requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
		}
	}
	b_hasFault = ds402Status.hasFault() || _LastError != 0x0;
	
	
	//react to power state change
	if(requestedPowerState == DS402::PowerState::OPERATION_ENABLED && actualPowerState != DS402::PowerState::OPERATION_ENABLED){
        if(EtherCatFieldbus::getCycleProgramTime_nanoseconds() - enableRequestTime_nanoseconds > enableRequestTimeout_nanoseconds){
            Logger::warn("{} : Enable Request Timeout", getName());
            requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
        }
	}

	//read operating mode
	auto newOperatingMode = ds402Status.getOperatingMode();
	if(newOperatingMode != actualOperatingMode){
		std::string message = "Operating Mode changed to " + std::string(Enumerator::getDisplayString(newOperatingMode));
		pushEvent(message.c_str(), false);
	}
	actualOperatingMode = newOperatingMode;
	
	//read STO State
	bool b_stoActive = _IO_STO_act == 0;
	if (b_stoActive != servoMotor->actuatorProcessData.b_isEmergencyStopActive) {
		if (b_stoActive) pushEvent("STO Activated", false);
		else pushEvent("STO Released", false);
	}
	servoMotor->actuatorProcessData.b_isEmergencyStopActive = b_stoActive;
	
	b_faultNeedsRestart = _actionStatus & 0x10;
	b_motorVoltagePresent = ds402Status.statusWord & 0x10;
	
    //set the encoder position in revolution units and velocity in revolutions per second
	servoMotor->feedbackProcessData.positionActual = ((double)_p_act / (double)positionUnitsPerRevolution) + servoMotor->positionOffset_revolutions;
	servoMotor->feedbackProcessData.velocityActual = (double)_v_act / ((double)velocityUnitsPerRpm * 60.0);
	servoMotor->actuatorProcessData.effortActual = ((double)_I_act / (double)currentUnitsPerAmp) / maxCurrent_amps;
	servoMotor->actuatorProcessData.followingErrorActual = (double)_p_dif_usr / (double)positionUnitsPerRevolution;
	
    //assign public input data
	*actualPositionValue = servoMotor->getPosition();
	*actualVelocityValue = servoMotor->getVelocity();
	*actualLoadValue = servoMotor->getEffort();
	
    bool DI0 = (_IO_act & 0x1) != 0x0;
    bool DI1 = (_IO_act & 0x2) != 0x0;
    bool DI2 = (_IO_act & 0x4) != 0x0;
    bool DI3 = (_IO_act & 0x8) != 0x0;
    bool DI4 = (_IO_act & 0x10) != 0x0;
    bool DI5 = (_IO_act & 0x20) != 0x0;
	
	*digitalIn0Value = b_invertDI0 ? !DI0 : DI0;
	*digitalIn1Value = b_invertDI1 ? !DI1 : DI1;
	*digitalIn2Value = b_invertDI2 ? !DI2 : DI2;
	*digitalIn3Value = b_invertDI3 ? !DI3 : DI3;
	*digitalIn4Value = b_invertDI4 ? !DI4 : DI4;
	*digitalIn5Value = b_invertDI5 ? !DI5 : DI5;

	
	if(!isConnected()) 														servoMotor->state = DeviceState::OFFLINE;
	else if(b_hasFault && b_faultNeedsRestart) 								servoMotor->state = DeviceState::OFFLINE;
	else if(b_stoActive) 													servoMotor->state = DeviceState::NOT_READY;
	else if(!b_motorVoltagePresent) 										servoMotor->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402::PowerState::NOT_READY_TO_SWITCH_ON) 	servoMotor->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402::PowerState::SWITCH_ON_DISABLED)		servoMotor->state = DeviceState::NOT_READY;
	else if(actualPowerState == DS402::PowerState::OPERATION_ENABLED) 		servoMotor->state = DeviceState::ENABLED;
	else 																	servoMotor->state = DeviceState::READY;

	if(!isConnected()) gpioDevice->state = DeviceState::OFFLINE;
	else if(isStateSafeOperational()) gpioDevice->state = DeviceState::READY;
	else if(isStateOperational()) gpioDevice->state = DeviceState::ENABLED;
	else gpioDevice->state = DeviceState::NOT_READY;
	
}





//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void Lexium32::writeOutputs() {
	
	if (digitalOut0Pin->isConnected()) digitalOut0Pin->copyConnectedPinValue();
	if (digitalOut1Pin->isConnected()) digitalOut1Pin->copyConnectedPinValue();
	if (digitalOut2Pin->isConnected()) digitalOut2Pin->copyConnectedPinValue();
	
	if(b_startHoming){
		b_startHoming = false;
		b_isHoming = true;
	}
	
	if(b_isHoming){
		servoMotor->setPositionTarget(servoMotor->getPosition());
	}
	else if(!servoMotorPin->isConnected()){
		if(!servoMotor->isEnabled()){
			//------ motor profile follows actual data ------
			servoMotor->setPositionTarget(servoMotor->getPosition());
		}else{
			//------ internal profile generator ------
			double deltaT_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
			double deltaV_rps = manualAcceleration_rpsps * deltaT_seconds;
			
			double previousVelocityCommand = servoMotor->actuatorProcessData.velocityTarget;
			double newVelocityCommand;
			if (manualVelocityCommand_rps > previousVelocityCommand) {
				newVelocityCommand = std::min(previousVelocityCommand + deltaV_rps, (double)manualVelocityCommand_rps);
			}
			else if (manualVelocityCommand_rps < previousVelocityCommand) {
				newVelocityCommand = std::max(previousVelocityCommand - deltaV_rps, (double)manualVelocityCommand_rps);
			}else newVelocityCommand = previousVelocityCommand;
			servoMotor->setVelocityTarget(newVelocityCommand);
		}
	}
	
	
	//update the target fields of inactive control modes
	//this allows us to switch between modes seamlessly
	if(servoMotor->actuatorProcessData.controlMode != ActuatorInterface::ControlMode::POSITION){
		servoMotor->actuatorProcessData.positionTarget = servoMotor->getPosition();
	}
	if(servoMotor->actuatorProcessData.controlMode != ActuatorInterface::ControlMode::VELOCITY){
		servoMotor->actuatorProcessData.velocityTarget = servoMotor->getVelocity();
	}
	if(servoMotor->actuatorProcessData.controlMode != ActuatorInterface::ControlMode::FORCE){
		servoMotor->actuatorProcessData.forceTarget = servoMotor->getForce();
	}
	
    //handle power state transitions
    if (servoMotor->actuatorProcessData.b_enable) {
		servoMotor->actuatorProcessData.b_enable = false;
		requestedPowerState = DS402::PowerState::OPERATION_ENABLED;
		enableRequestTime_nanoseconds = EtherCatFieldbus::getCycleProgramTime_nanoseconds();
    }
    if (servoMotor->actuatorProcessData.b_disable) {
		servoMotor->actuatorProcessData.b_disable = false;
		requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
    }
    if (servoMotor->actuatorProcessData.b_quickstop) {
		servoMotor->actuatorProcessData.b_quickstop = false;
		requestedPowerState = DS402::PowerState::QUICKSTOP_ACTIVE;
    }
	
	//manual holding brake control
	if(servoMotor->actuatorProcessData.b_releaseHoldingBrake){
		servoMotor->actuatorProcessData.b_releaseHoldingBrake = false;
		switch(actualPowerState){
			case DS402::PowerState::SWITCH_ON_DISABLED:
			case DS402::PowerState::READY_TO_SWITCH_ON:
			case DS402::PowerState::FAULT:
				servoMotor->actuatorProcessData.b_holdingBrakeIsReleased = true;
				break;
			default: break;
		}
	}
	if(servoMotor->actuatorProcessData.b_applyHoldingBrake){
		servoMotor->actuatorProcessData.b_applyHoldingBrake = false;
		servoMotor->actuatorProcessData.b_holdingBrakeIsReleased = false;
	}
	BRK_release = servoMotor->actuatorProcessData.b_holdingBrakeIsReleased ? 1 : 0;
	
	
	if(servoMotor->feedbackProcessData.b_overridePosition){
		servoMotor->feedbackProcessData.b_overridePosition = false;
		double overrideTargetPosition = servoMotor->feedbackProcessData.positionOverride;
		double positionRaw = (double)_p_act / (double)positionUnitsPerRevolution;
		servoMotor->positionOffset_revolutions = overrideTargetPosition - positionRaw;
		updateEncoderWorkingRange();
		servoMotor->feedbackProcessData.b_positionOverrideBusy = false;
		servoMotor->feedbackProcessData.b_positionOverrideSucceeded = true;
	}
	
    //========== PREPARE RXPDO OUTPUTS ==========
    //DCOMcontrol   (uint16_t)  2
    //DCOMopmode    (int8_t)    1
    //PPp_target    (int32_t)   4
    //IO_DQ_set     (uint16_t)  2

	//DCOMcontrol
	ds402Control.setPowerState(requestedPowerState, actualPowerState);
	
	if(b_isHoming && actualOperatingMode == DS402::OperatingMode::HOMING) ds402Control.setOperatingModeSpecificByte4(true);
	else ds402Control.setOperatingModeSpecificByte4(false);
	
	//clear errors when we enable the power stage
	if(b_hasFault && !b_isResettingFault && requestedPowerState == DS402::PowerState::OPERATION_ENABLED){
		b_isResettingFault = true;
		ds402Control.performFaultReset();
	}else b_isResettingFault = false;
	ds402Control.updateControlWord();
	
	//DCOMopmode
	if(b_isHoming) ds402Control.setOperatingMode(DS402::OperatingMode::HOMING);
	else if(servoMotor->actuatorProcessData.controlMode == ActuatorInterface::ControlMode::POSITION)
		ds402Control.setOperatingMode(DS402::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION);
	else if(servoMotor->actuatorProcessData.controlMode == ActuatorInterface::ControlMode::VELOCITY)
		ds402Control.setOperatingMode(DS402::OperatingMode::CYCLIC_SYNCHRONOUS_VELOCITY);
	/*
	else if(servoMotor->actuatorProcessData.controlMode == ActuatorInterface::ControlMode::FORCE)
		ds402Control.setOperatingMode(DS402::OperatingMode::CYCLIC_SYNCHRONOUS_TORQUE);
	*/
	else
		ds402Control.setOperatingMode(DS402::OperatingMode::NONE);
		
	//PPp_target
	PPp_target = (int32_t)((servoMotor->actuatorProcessData.positionTarget - servoMotor->positionOffset_revolutions) * positionUnitsPerRevolution);
	PVv_target = (int32_t)(servoMotor->actuatorProcessData.velocityTarget * velocityUnitsPerRpm * 60.0);
	//PTtq_target = (int16_t)(servoMotor->actuatorProcessData.forceTarget * ???);
	
	//IO_DQ_set
    IO_DQ_set = 0;
    if (*digitalOut0Value) IO_DQ_set |= 0x1;
    if (*digitalOut1Value) IO_DQ_set |= 0x2;
    if (*digitalOut2Value) IO_DQ_set |= 0x4;

    rxPdoAssignement.pushDataTo(identity->outputs);
}



























//==============================================================
//===================== GENERAL PARAMETERS =====================
//==============================================================

bool Lexium32::uploadGeneralParameters() {
    generalParameterUploadState = DataTransferState::TRANSFERRING;

	auto onFailure = [&]() -> bool {
		generalParameterUploadState = DataTransferState::FAILED;
		return false;
	};
	
	int32_t MON_p_dif_load_usr = servoMotor->actuatorConfig.followingErrorLimit * positionUnitsPerRevolution;
	if(!writeSDO_S32(0x3006, 0x3E, MON_p_dif_load_usr)) return onFailure();
	
	uint16_t CTRL_I_max = maxCurrent_amps * 100.0;
	if(!writeSDO_U16(0x3011, 0xC, CTRL_I_max)) return onFailure();
	
	int16_t LIM_QStopReact;
	switch(quickstopReaction){
		case QuickStopReaction::DECELERATION_RAMP: LIM_QStopReact = 6; break;
		case QuickStopReaction::TORQUE_RAMP: LIM_QStopReact = 7; break;
	}
	if(!writeSDO_S16(0x3006, 0x18, LIM_QStopReact)) return onFailure();

    switch (quickstopReaction) {
        case QuickStopReaction::DECELERATION_RAMP: {
			uint32_t RAMPquickstop = quickStopDeceleration_revolutionsPerSecondSquared * 60.0;
			if(!writeSDO_U32(0x3006, 0x12, RAMPquickstop)) return onFailure();
        }break;
        case QuickStopReaction::TORQUE_RAMP: {
			uint16_t LIM_I_maxQSTP = maxQuickstopCurrent_amps * 100.0;
			if(!writeSDO_U16(0x3011, 0xD, LIM_I_maxQSTP)) return onFailure();
        }break;
    }

	uint16_t InvertDirOfMove = b_invertDirectionOfMotorMovement ? 1 : 0;
	if(!writeSDO_U16(0x3006, 0xC, InvertDirOfMove)) return onFailure();
		
	uint16_t ShiftEncWorkRange = b_encoderRangeShifted ? 1 : 0;
	if(!writeSDO_U16(0x3005, 0x21, ShiftEncWorkRange)) return onFailure();

    generalParameterUploadState = DataTransferState::SAVING;
	if (!saveToEEPROM()) return onFailure();
    generalParameterUploadState = DataTransferState::SAVED;
	
	updateEncoderWorkingRange();

	return true;
}


bool Lexium32::downloadGeneralParameters() {
    generalParameterDownloadState = DataTransferState::TRANSFERRING;
	
	auto onFailure = [&]() -> bool {
		generalParameterDownloadState = DataTransferState::FAILED;
		return false;
	};
 
	//====== Fixed Drive and Motor Properties ======
	
	uint16_t _M_I_max;
	if(!readSDO_U16(0x300D, 0x6, _M_I_max)) return onFailure();
	maxMotorCurrent_amps = (float)_M_I_max / 100.0;
	
	uint16_t _M_n_max; //max motor spec velocity in rpm
	if (!readSDO_U16(0x300D, 0x4, _M_n_max)) return onFailure();
	maxMotorVelocity = (double)_M_n_max / 60.0;
	
	uint16_t _M_HoldingBrake;
	if(!readSDO_U16(0x300D, 0x20, _M_HoldingBrake)) return onFailure();
	servoMotor->actuatorConfig.b_supportsHoldingBrakeControl = _M_HoldingBrake == 1;
	
	uint16_t _M_Encoder;
	if(!readSDO_U16(0x300D, 0x3, _M_Encoder)) return onFailure();
	//detect encoder type
	
	
	//====== General Drive Settings ======
	
	uint16_t CTRL_I_max;
	if(!readSDO_U16(0x3011, 0xC, CTRL_I_max)) return onFailure();
	maxCurrent_amps = (float)CTRL_I_max / 100.0;
	
	int32_t MON_p_dif_load_usr;
	if(!readSDO_S32(0x3006, 0x3E, MON_p_dif_load_usr)) return onFailure();
	servoMotor->actuatorConfig.followingErrorLimit = (float)MON_p_dif_load_usr / positionUnitsPerRevolution;

	int16_t LIM_QStopReact;
	if(!readSDO_S16(0x3006, 0x18, LIM_QStopReact)) return onFailure();
	switch (LIM_QStopReact) {
		case -1:
		case 6: quickstopReaction = QuickStopReaction::DECELERATION_RAMP; break;
		case -2:
		case 7: quickstopReaction = QuickStopReaction::TORQUE_RAMP; break;
		default: return onFailure();
	}

    switch (quickstopReaction) {
        case QuickStopReaction::DECELERATION_RAMP: {
			uint32_t RAMPquickstop;
			if(!readSDO_U32(0x3006, 0X12, RAMPquickstop)) return onFailure();
            quickStopDeceleration_revolutionsPerSecondSquared = ((double)RAMPquickstop / (double)accelerationUnitsPerRpmps) / 60.0;
        }break;
        case QuickStopReaction::TORQUE_RAMP: {
			uint16_t LIM_I_maxQSTP;
			if(!readSDO_U16(0x3011, 0xD, LIM_I_maxQSTP)) return onFailure();
            maxQuickstopCurrent_amps = (float)LIM_I_maxQSTP / 100.0;
        }break;
    }
	
	uint16_t InvertDirOfMove;
	if(!readSDO_U16(0x3006, 0xC, InvertDirOfMove)) return onFailure();
	switch(InvertDirOfMove){
		case 0: b_invertDirectionOfMotorMovement = false; break;
		case 1: b_invertDirectionOfMotorMovement = true; break;
		default: return onFailure();
	}

	uint16_t ShiftEncWorkRange;
	if(!readSDO_U16(0x3005, 0x21, ShiftEncWorkRange)) return onFailure();
	switch(ShiftEncWorkRange){
		case 0: b_encoderRangeShifted = false; break;
		case 1: b_encoderRangeShifted = true; break;
		default: return onFailure();
	}
	
    generalParameterDownloadState = DataTransferState::SUCCEEDED;
	
	updateEncoderWorkingRange();

	return true;
}



//==============================================================
//======================= I/O ASSIGNEMENT ======================
//==============================================================


bool Lexium32::uploadPinAssignements() {

	pinAssignementUploadState = DataTransferState::TRANSFERRING;

	auto onFailure = [&]() -> bool{
		pinAssignementUploadState = DataTransferState::FAILED;
		return false;
	};
	
	for(auto& pin : Enumerator::getTypes<InputPin>()){
		if(pin.enumerator == InputPin::NONE) continue;
		uint16_t IOfunct_DIx;
		if(negativeLimitSwitchPin == pin.enumerator) IOfunct_DIx = 23;
		else if(positiveLimitSwitchPin == pin.enumerator) IOfunct_DIx = 22;
		else if(holdingBrakeReleasePin == pin.enumerator) IOfunct_DIx = 40;
		else IOfunct_DIx = 1;
		if(!writeSDO_U16(0x3007, getInputPinSubindex(pin.enumerator), IOfunct_DIx)) return onFailure();
	}
	
	uint16_t IOsigLIMN;
	if(negativeLimitSwitchPin == InputPin::NONE) IOsigLIMN = 0;
	else if(b_negativeLimitSwitchNormallyClosed) IOsigLIMN = 1;
	else IOsigLIMN = 2;
	if(!writeSDO_U16(0x3006, 0xF, IOsigLIMN)) return onFailure();
	
	uint16_t IOsigLIMP;
	if(positiveLimitSwitchPin == InputPin::NONE) IOsigLIMP = 0;
	else if(b_positiveLimitSwitchNormallyClosed) IOsigLIMP = 1;
	else IOsigLIMP = 2;
	if(!writeSDO_U16(0x3006, 0x10, IOsigLIMP)) return onFailure();

	
	//set all output pins to freely available
	uint16_t IOfunct_DQx = 1;
	if(!writeSDO_U16(0x3007, 0x9, IOfunct_DQx)) return onFailure();
	if(!writeSDO_U16(0x3007, 0xA, IOfunct_DQx)) return onFailure();
	if(!writeSDO_U16(0x3007, 0xB, IOfunct_DQx)) return onFailure();

	pinAssignementUploadState = DataTransferState::SAVING;
	if (!saveToEEPROM()) return onFailure();
	
	pinAssignementUploadState = DataTransferState::SAVED;
	Logger::info("Pin Assignement Successfull");

	return true;
}


bool Lexium32::downloadPinAssignements() {
	pinAssignementDownloadState = DataTransferState::NO_TRANSFER;

	auto onFailure = [&]() -> bool{
		pinAssignementDownloadState = DataTransferState::FAILED;
		return false;
	};
	
	negativeLimitSwitchPin = InputPin::NONE;
	positiveLimitSwitchPin = InputPin::NONE;
	holdingBrakeReleasePin = InputPin::NONE;
	
	for (auto& type : Enumerator::getTypes<InputPin>()) {
		if (type.enumerator == InputPin::NONE) continue;
		
		uint16_t IOfunct_DIx;
		if(!readSDO_U16(0x3007, getInputPinSubindex(type.enumerator), IOfunct_DIx)) return onFailure();
		
		//for each pin, read the function assignement stored on the drive
		switch (IOfunct_DIx) {
			case 23: negativeLimitSwitchPin = type.enumerator; break;
			case 22: positiveLimitSwitchPin = type.enumerator; break;
			case 40: holdingBrakeReleasePin = type.enumerator; break;
			case 1: //freely available pin / unassigned
			default: break;
		}
	}

	
	if(negativeLimitSwitchPin != InputPin::NONE){
		uint16_t IOsignLMN;
		if(!readSDO_U16(0x3006, 0xF, IOsignLMN)) return onFailure();
		switch(IOsignLMN){
			case 1: b_negativeLimitSwitchNormallyClosed = true; break;
			case 2: b_negativeLimitSwitchNormallyClosed = false; break;
			default: return onFailure();
		}
	}
	
	if(positiveLimitSwitchPin != InputPin::NONE){
		uint16_t IOsigLIMP;
		if(!readSDO_U16(0x3006, 0x10, IOsigLIMP)) return onFailure();
		switch(IOsigLIMP){
			case 1: b_positiveLimitSwitchNormallyClosed = true; break;
			case 2: b_positiveLimitSwitchNormallyClosed = false; break;
			default: return onFailure();
		}
	}

	pinAssignementDownloadState = DataTransferState::SUCCEEDED;

	return true;
}



//==============================================================
//====================== ENCODER SETTINGS ======================
//==============================================================

bool Lexium32::uploadManualAbsoluteEncoderPosition() {
    encoderAbsolutePositionUploadState = DataTransferState::TRANSFERRING;

	auto onFailure = [&]() -> bool {
		encoderAbsolutePositionUploadState = EtherCatDevice::DataTransferState::FAILED;
		return false;
	};
	
	int32_t ENC1_adjustment = manualAbsoluteEncoderPosition_revolutions * (float)(0x1 << encoder1_singleTurnResolutionBits);
	if(!writeSDO_S32(0x3005, 0x16, ENC1_adjustment)) return onFailure();

    encoderAbsolutePositionUploadState = DataTransferState::SAVING;
	if (!saveToEEPROM()) return onFailure();
	encoderAbsolutePositionUploadState = DataTransferState::SAVED;

	return true;
}



void Lexium32::updateEncoderWorkingRange() {
	if(b_encoderIsMultiturn){
		if (b_encoderRangeShifted) {
			servoMotor->feedbackConfig.positionLowerWorkingRangeBound = -(float)(0x1 << encoder1_multiTurnResolutionBits) / 2.0;
			servoMotor->feedbackConfig.positionUpperWorkingRangeBound = (float)(0x1 << encoder1_multiTurnResolutionBits) / 2.0;
		}
		else {
			servoMotor->feedbackConfig.positionLowerWorkingRangeBound = 0.0;
			servoMotor->feedbackConfig.positionUpperWorkingRangeBound = (float)(0x1 << encoder1_multiTurnResolutionBits);
		}
		servoMotor->feedbackConfig.positionLowerWorkingRangeBound += servoMotor->positionOffset_revolutions;
		servoMotor->feedbackConfig.positionUpperWorkingRangeBound += servoMotor->positionOffset_revolutions;
	}else{
		servoMotor->feedbackConfig.positionLowerWorkingRangeBound = -std::numeric_limits<double>::infinity();
		servoMotor->feedbackConfig.positionUpperWorkingRangeBound = std::numeric_limits<double>::infinity();
	}
}





//==============================================================
//========================= AUTO TUNING ========================
//==============================================================


void Lexium32::startAutoTuning() {
    if (isAutoTuning()) return;

    autoTuningHandler = std::thread([this]() {

        b_autoTuningSucceeded = false;
        autoTuningSaveState = DataTransferState::NO_TRANSFER;

        //clear any error inthe drive by setting the error clear bit to 1 and then back to 0
        EtherCatCoeData DCOMcontrol(0x6040, 0x0, EtherCatData::Type::UINT16_T);
        DCOMcontrol.setU16(0x80);
        DCOMcontrol.write(getSlaveIndex());
        DCOMcontrol.setU16(0x0);
        DCOMcontrol.write(getSlaveIndex());

        //set the current operating mode to -6 (Tuning)
        EtherCatCoeData DCOMopmode(0x6060, 0x0, EtherCatData::Type::INT8_T);
        DCOMopmode.setS8(-6);
        DCOMopmode.write(getSlaveIndex());

        //start auto tuning by setting AT_start to 1 (EasyTuning)
        EtherCatCoeData AT_start(0x302F, 0x1, EtherCatData::Type::UINT16_T);
        AT_start.setU16(1);
        b_isAutoTuning = AT_start.write(getSlaveIndex());
        if (b_isAutoTuning) Logger::info("Started Autotuning {}", getName());
        else Logger::warn("Failed to start autotuning {}", getName());

        while (b_isAutoTuning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            EtherCatCoeData _AT_progress(0x302F, 0xB, EtherCatData::Type::UINT16_T);
            _AT_progress.read(getSlaveIndex());

            EtherCatCoeData _AT_state(0x302F, 0x2, EtherCatData::Type::UINT16_T);
            _AT_state.read(getSlaveIndex());
            uint16_t lastTuningStep = _AT_state.getU16() & 0x7FF;
            bool auto_tune_process = _AT_state.getU16() & 0x2000;
            bool auto_tune_end = _AT_state.getU16() & 0x4000;
            bool auto_tune_error = _AT_state.getU16() & 0x8000;

            tuningProgress = (float)_AT_progress.getU16() / 100.0f;

            if (auto_tune_end) {
                Logger::info("Finished Auto Tuning {}", getName());
                b_autoTuningSucceeded = true;
                break;
            }
            else if (auto_tune_error) {
                Logger::error("Auto Tuning Error {}", getName());
                b_autoTuningSucceeded = false;
                break;
            }
        }


        if (!b_isAutoTuning) {
            //manual autotuning stop
            tuningProgress = 0.0;
            AT_start.setU16(0);
            AT_start.write(getSlaveIndex());
            Logger::info("Stopped Autotuning {}", getName());
        }
        else if (b_autoTuningSucceeded) {

            b_isAutoTuning = false;

            EtherCatCoeData _AT_M_friction(0x302F, 0x7, EtherCatData::Type::UINT16_T);
            _AT_M_friction.read(getSlaveIndex());
            tuning_frictionTorque_amperes = (float)_AT_M_friction.getU16() / 100.0;

            Logger::warn("friction: {}", _AT_M_friction.getU16());

            EtherCatCoeData _AT_M_load(0x302F, 0x8, EtherCatData::Type::INT16_T);
            _AT_M_load.read(getSlaveIndex());
            tuning_constantLoadTorque_amperes = (float)_AT_M_load.getS16() / 100.0;

            Logger::warn("load: {}", _AT_M_load.getS16());

            EtherCatCoeData _AT_J(0x302F, 0xC, EtherCatData::Type::UINT16_T);
            _AT_J.read(getSlaveIndex());
            tuning_momentOfInertia_kilogramcentimeter2 = (float)_AT_J.getU16() / 10.0;

            Logger::warn("moment: {}", _AT_J.getU16());


            //saving tuning parameters to devuce EEPROM
            autoTuningSaveState = DataTransferState::SAVING;
            Logger::info("Saving Tuning parameters to EEPROM {}", getName());
            if (saveToEEPROM()) {
                autoTuningSaveState = DataTransferState::SAVED;
                Logger::info("Saving Succeeded");
            }
            else {
                autoTuningSaveState = DataTransferState::FAILED;
                Logger::error("Saving Failed");
            }
        }
        });
    autoTuningHandler.detach();
}

void Lexium32::stopAutoTuning() {
    b_isAutoTuning = false;
}






//==============================================================
//=================== MISCELLANEOUS SETTINGS ===================
//==============================================================


bool Lexium32::saveToEEPROM() {
    EtherCatCoeData* PAReeprSave = new EtherCatCoeData(0x3004, 0x1, EtherCatData::Type::UINT16_T);
    PAReeprSave->setU16(1);
    PAReeprSave->write(getSlaveIndex());
    std::chrono::system_clock::time_point saveStart = std::chrono::system_clock::now();
    while (PAReeprSave->getU16() == 1) {
        if (std::chrono::system_clock::now() - saveStart > std::chrono::milliseconds(5000)) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        PAReeprSave->read(getSlaveIndex());
    }
    return true;
}

void Lexium32::factoryReset() {
    factoryResetTransferState = DataTransferState::TRANSFERRING;
    EtherCatCoeData PARuserReset(0x3004, 0x8, EtherCatData::Type::UINT16_T);
    PARuserReset.setU16(65535);
    if (PARuserReset.write(getSlaveIndex())) factoryResetTransferState = DataTransferState::SUCCEEDED;
    else factoryResetTransferState = DataTransferState::FAILED;
}

void Lexium32::setStationAlias(uint16_t a) {
    stationAliasUploadState = DataTransferState::TRANSFERRING;
    EtherCatCoeData ECAT2ndaddress(0x3045, 0x6, EtherCatData::Type::UINT16_T);
    ECAT2ndaddress.setU16(a);
    if (ECAT2ndaddress.write(getSlaveIndex())) {
        stationAliasUploadState = DataTransferState::SAVING;
        if (saveToEEPROM())  stationAliasUploadState = DataTransferState::SAVED;
        else  stationAliasUploadState = DataTransferState::FAILED;
    }
    else stationAliasUploadState = DataTransferState::FAILED;
}






















//============================= SAVING AND LOADING DEVICE DATA ============================

bool Lexium32::saveDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;

	XMLElement* motorPropertiesXML = xml->InsertNewChildElement("MotorProperties");
	motorPropertiesXML->SetAttribute("MaxVelocity", maxMotorVelocity);
	motorPropertiesXML->SetAttribute("MaxCurrent", maxMotorCurrent_amps);
	motorPropertiesXML->SetAttribute("EncoderIsMultiturn", b_encoderIsMultiturn);
	
    XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
    kinematicLimitsXML->SetAttribute("velocityLimit_rps", servoMotor->getVelocityLimit());
    kinematicLimitsXML->SetAttribute("accelerationLimit_rpsps", servoMotor->getAccelerationLimit());
    kinematicLimitsXML->SetAttribute("manualAcceleration_rpsps", manualAcceleration_rpsps);

    XMLElement* invertDirectionOfMovementXML = xml->InsertNewChildElement("InvertDirectionOfMovement");
    invertDirectionOfMovementXML->SetAttribute("Invert", b_invertDirectionOfMotorMovement);
	
	XMLElement* maxFollowingErrorXML = xml->InsertNewChildElement("MaxFollowingError");
	maxFollowingErrorXML->SetAttribute("revolutions", servoMotor->getFollowingErrorLimit());

    XMLElement* currentLimitXML = xml->InsertNewChildElement("CurrentLimit");
    currentLimitXML->SetAttribute("amps", maxCurrent_amps);
    
    XMLElement* quickstopReactionXML = xml->InsertNewChildElement("Quickstop");
    quickstopReactionXML->SetAttribute("Reaction", Enumerator::getSaveString(quickstopReaction));
    switch (quickstopReaction) {
        case QuickStopReaction::DECELERATION_RAMP:
            quickstopReactionXML->SetAttribute("Deceleration_RevolutionsPerSecondSquared", quickStopDeceleration_revolutionsPerSecondSquared);
            break;
        case QuickStopReaction::TORQUE_RAMP:
            quickstopReactionXML->SetAttribute("MaxCurrent_amps", maxQuickstopCurrent_amps);
            break;
    }

    XMLElement* negativeLimitSwitchXML = xml->InsertNewChildElement("NegativeLimitSwitch");
    negativeLimitSwitchXML->SetAttribute("Pin", Enumerator::getSaveString(negativeLimitSwitchPin));
    if (negativeLimitSwitchPin != InputPin::NONE) negativeLimitSwitchXML->SetAttribute("NormallyClosed", b_negativeLimitSwitchNormallyClosed);

    XMLElement* positiveLimitSwitchXML = xml->InsertNewChildElement("PositiveLimitSwitch");
    positiveLimitSwitchXML->SetAttribute("Pin", Enumerator::getSaveString(positiveLimitSwitchPin));
    if (positiveLimitSwitchPin != InputPin::NONE) positiveLimitSwitchXML->SetAttribute("NormallyClosed", b_positiveLimitSwitchNormallyClosed);

    XMLElement* pinInversionXML = xml->InsertNewChildElement("DigitalPinInversion");
    pinInversionXML->SetAttribute("DI0", b_invertDI0);
    pinInversionXML->SetAttribute("DI1", b_invertDI1);
    pinInversionXML->SetAttribute("DI2", b_invertDI2);
    pinInversionXML->SetAttribute("DI3", b_invertDI3);
    pinInversionXML->SetAttribute("DI4", b_invertDI4);
    pinInversionXML->SetAttribute("DI5", b_invertDI5);

    XMLElement* encoderSettingsXML = xml->InsertNewChildElement("EncoderSettings");
    encoderSettingsXML->SetAttribute("RangeShifted", b_encoderRangeShifted);
    encoderSettingsXML->SetAttribute("PositionOffset", servoMotor->positionOffset_revolutions);
	
	XMLElement* holdingBrakeXML = xml->InsertNewChildElement("HoldingBrake");
	holdingBrakeXML->SetAttribute("HasHoldingBrake", servoMotor->supportsHoldingBrakeControl());

    return true;
}


bool Lexium32::loadDeviceData(tinyxml2::XMLElement* xml) {

    using namespace tinyxml2;
	
	XMLElement* motorPropertiesXML = xml->FirstChildElement("MotorProperties");
	if(motorPropertiesXML == nullptr) return Logger::warn("Could not find motor properties");
	if(motorPropertiesXML->QueryDoubleAttribute("MaxVelocity", &maxMotorVelocity) != XML_SUCCESS) return Logger::warn("Could not read max motor velocity attribute");
	if(motorPropertiesXML->QueryDoubleAttribute("MaxCurrent", &maxMotorCurrent_amps) != XML_SUCCESS) return Logger::warn("Could not read max motor current attribute");
	if(motorPropertiesXML->QueryBoolAttribute("EncoderIsMultiturn", &b_encoderIsMultiturn) != XML_SUCCESS) return Logger::warn("Could not read encoder is multiturn attribute");

    XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
    if (kinematicLimitsXML == nullptr) return Logger::warn("Could not find kinematic limits attribute");
	
	if (kinematicLimitsXML->QueryDoubleAttribute("velocityLimit_rps", &servoMotor->actuatorConfig.velocityLimit) != XML_SUCCESS) return Logger::warn("Could not read velocity limit attribute");
    if (kinematicLimitsXML->QueryDoubleAttribute("accelerationLimit_rpsps", &servoMotor->actuatorConfig.accelerationLimit) != XML_SUCCESS) return Logger::warn("Could not read acceleration limit attribute");
    if (kinematicLimitsXML->QueryFloatAttribute("manualAcceleration_rpsps", &manualAcceleration_rpsps) != XML_SUCCESS) return Logger::warn("Could not read manual acceleration attribute");
	 
    XMLElement* invertDirectionOfMovementXML = xml->FirstChildElement("InvertDirectionOfMovement");
    if (invertDirectionOfMovementXML == nullptr) return Logger::warn("Could not find invert direction of movement attribute");
    if (invertDirectionOfMovementXML->QueryBoolAttribute("Invert", &b_invertDirectionOfMotorMovement) != XML_SUCCESS) return Logger::warn("Could not read direciton of movement attribute");

	XMLElement* maxFollowingErrorXML = xml->FirstChildElement("MaxFollowingError");
	if(maxFollowingErrorXML == nullptr) return Logger::warn("Could not find max following error attribute");
	if(maxFollowingErrorXML->QueryAttribute("revolutions", &servoMotor->actuatorConfig.followingErrorLimit) != XML_SUCCESS) return Logger::warn("Could not read max following error attribute");
	 
    XMLElement* currentLimitsXML = xml->FirstChildElement("CurrentLimit");
    if (currentLimitsXML == nullptr) return Logger::warn("Could not find current limits attribute");
    if (currentLimitsXML->QueryDoubleAttribute("amps", &maxCurrent_amps) != XML_SUCCESS) return Logger::warn("Could not read Max Current Attribute");



    XMLElement* quickstopReactionXML = xml->FirstChildElement("Quickstop");
    if (quickstopReactionXML == nullptr) return Logger::warn("Could not find quickstop attribute");
    const char* quickstopReactionTypeString;
    if (quickstopReactionXML->QueryStringAttribute("Reaction", &quickstopReactionTypeString) != XML_SUCCESS) return Logger::warn("Could not find quickstop reaction type attribute");
    if (!Enumerator::isValidSaveName<QuickStopReaction>(quickstopReactionTypeString)) return Logger::warn("Could not read quickstop reaction type");
	quickstopReaction = Enumerator::getEnumeratorFromSaveString<QuickStopReaction>(quickstopReactionTypeString);
      
    switch (quickstopReaction) {
        case QuickStopReaction::DECELERATION_RAMP:
            if (quickstopReactionXML->QueryDoubleAttribute("Deceleration_RevolutionsPerSecondSquared", &quickStopDeceleration_revolutionsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not read quickstop deceleration value");
            break;
        case QuickStopReaction::TORQUE_RAMP:
            if (quickstopReactionXML->QueryDoubleAttribute("MaxCurrent_amps", &maxQuickstopCurrent_amps) != XML_SUCCESS) return Logger::warn("Could not read quickstop current value");
            break;
    }

    XMLElement* negativeLimitSwitchXML = xml->FirstChildElement("NegativeLimitSwitch");
    if (negativeLimitSwitchXML == nullptr) return Logger::warn("Could not find negative limit switch attribute");
    const char* negativeLimitSwitchPinString = "";
    negativeLimitSwitchXML->QueryStringAttribute("Pin", &negativeLimitSwitchPinString);
    if (!Enumerator::isValidSaveName<InputPin>(negativeLimitSwitchPinString)) return Logger::warn("Could not read negative limit switch pin attribute");
	negativeLimitSwitchPin = Enumerator::getEnumeratorFromSaveString<InputPin>(negativeLimitSwitchPinString);
    if (negativeLimitSwitchPin != InputPin::NONE) {
        if (negativeLimitSwitchXML->QueryBoolAttribute("NormallyClosed", &b_negativeLimitSwitchNormallyClosed) != XML_SUCCESS) return Logger::warn("Could not read normally closed attribute of negative limit switch");
    }

    XMLElement* positiveLimitSwitchXML = xml->FirstChildElement("PositiveLimitSwitch");
    if (positiveLimitSwitchXML == nullptr) return Logger::warn("Could not find positive limit switch attribute");
    const char* positiveLimitSwitchPinString = "";
    positiveLimitSwitchXML->QueryStringAttribute("Pin", &positiveLimitSwitchPinString);
    if (!Enumerator::isValidSaveName<InputPin>(positiveLimitSwitchPinString)) return Logger::warn("Could not read positive limit switch pin attribute");
	positiveLimitSwitchPin = Enumerator::getEnumeratorFromSaveString<InputPin>(positiveLimitSwitchPinString);
    if (positiveLimitSwitchPin != InputPin::NONE) {
        if (positiveLimitSwitchXML->QueryBoolAttribute("NormallyClosed", &b_positiveLimitSwitchNormallyClosed) != XML_SUCCESS) return Logger::warn("Could not read normally closed attribute of positive limit switch");
    }

    XMLElement* pinInversionXML = xml->FirstChildElement("DigitalPinInversion");
    if (pinInversionXML == nullptr) return Logger::warn("Could not find pin inversion attribute");
    if (pinInversionXML->QueryBoolAttribute("DI0", &b_invertDI0) != XML_SUCCESS) return Logger::warn("Could not find inver DI0 attribute");
    if (pinInversionXML->QueryBoolAttribute("DI1", &b_invertDI1) != XML_SUCCESS) return Logger::warn("Could not find inver DI1 attribute");
    if (pinInversionXML->QueryBoolAttribute("DI2", &b_invertDI2) != XML_SUCCESS) return Logger::warn("Could not find inver DI2 attribute");
    if (pinInversionXML->QueryBoolAttribute("DI3", &b_invertDI3) != XML_SUCCESS) return Logger::warn("Could not find inver DI3 attribute");
    if (pinInversionXML->QueryBoolAttribute("DI4", &b_invertDI4) != XML_SUCCESS) return Logger::warn("Could not find inver DI4 attribute");
    if (pinInversionXML->QueryBoolAttribute("DI5", &b_invertDI5) != XML_SUCCESS) return Logger::warn("Could not find inver DI5 attribute");

    XMLElement* encoderSettingsXML = xml->FirstChildElement("EncoderSettings");
    if (encoderSettingsXML == nullptr) return Logger::warn("Could not find Encoder Settings Attribute");
    if (encoderSettingsXML->QueryBoolAttribute("RangeShifted", &b_encoderRangeShifted) != XML_SUCCESS) return Logger::warn("Could not find encoder range shift attribute");
    if (encoderSettingsXML->QueryDoubleAttribute("PositionOffset", &servoMotor->positionOffset_revolutions) != XML_SUCCESS) return Logger::warn("Could not find position offset attribute");
	updateEncoderWorkingRange();
	
	XMLElement* holdingBrakeXML = xml->FirstChildElement("HoldingBrake");
	if(holdingBrakeXML == nullptr) return Logger::warn("Could not find holding brake attribute");
	if(holdingBrakeXML->QueryBoolAttribute("HasHoldingBrake", &servoMotor->actuatorConfig.b_supportsHoldingBrakeControl) != XML_SUCCESS) return Logger::warn("could not find has holdin brake attribute");
	 
    return true;
}
