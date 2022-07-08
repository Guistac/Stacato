#include <pch.h>

#include "Lexium32.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

void Lexium32::onConnection() {
    resetData();
}

void Lexium32::onDisconnection() {
    resetData();
	requestedPowerState = DS402::PowerState::QUICKSTOP_ACTIVE;
}

void Lexium32::resetData() {
    manualVelocityCommand_rps = 0.0;
    profilePosition_r = 0.0;
    profileVelocity_rps = 0.0;
    servoMotorDevice->positionRaw_positionUnits = 0.0;
    servoMotorDevice->velocity_positionUnitsPerSecond = 0.0;
    servoMotorDevice->b_moving = false;
	*positionPinValue = 0.0;
	*velocityPinValue = 0.0;
	*loadPinValue = 0.0;
	*digitalIn1PinValue = false;
	*digitalIn2PinValue = false;
	*digitalIn3PinValue = false;
	*digitalIn4PinValue = false;
	*digitalIn5PinValue = false;
	*digitalOut0PinValue = false;
	*digitalOut1PinValue = false;
	*digitalOut2PinValue = false;
    servoMotorDevice->b_detected = false;
    servoMotorDevice->b_online = false;
    servoMotorDevice->b_ready = false;
    servoMotorDevice->b_enabled = false;
    gpioDevice->b_detected = false;
    gpioDevice->b_online = false;
    gpioDevice->b_ready = false;
}

void Lexium32::initialize() {
    std::shared_ptr<Device> thisDevice = std::static_pointer_cast<Device>(shared_from_this());

	//servo device
    servoMotorDevice->setParentDevice(thisDevice);
    servoMotorDevice->velocityLimit_positionUnitsPerSecond = 10.0;
    servoMotorDevice->accelerationLimit_positionUnitsPerSecondSquared = 1.0;
    servoMotorDevice->setParentDevice(thisDevice);
	float lowEncoderRange, highEncoderRange;
	getEncoderWorkingRange(lowEncoderRange, highEncoderRange);
	servoMotorDevice->rangeMin_positionUnits = lowEncoderRange;
	servoMotorDevice->rangeMax_positionUnits = highEncoderRange;

	//gpio device
    gpioDevice->setParentDevice(thisDevice);

    //node input data
	digitalOut0->assignData(digitalOut0PinValue);
	digitalOut1->assignData(digitalOut1PinValue);
	digitalOut2->assignData(digitalOut2PinValue);
	
    addNodePin(digitalOut0);
    addNodePin(digitalOut1);
    addNodePin(digitalOut2);

    //node output data
    addNodePin(servoMotorLink);
    addNodePin(actualLoad);
    addNodePin(actualPosition);
    addNodePin(actualVelocity);
    addNodePin(gpioDeviceLink);
		
    addNodePin(digitalIn0);
    addNodePin(digitalIn1);
    addNodePin(digitalIn2);
    addNodePin(digitalIn3);
    addNodePin(digitalIn4);
    addNodePin(digitalIn5);

    rxPdoAssignement.addNewModule(0x1603);
    rxPdoAssignement.addEntry(0x6040, 0x0, 16, "DCOMcontrol", &ds402Control.controlWord);	//DS402 control word
    rxPdoAssignement.addEntry(0x6060, 0x0, 8, "DCOMopmode", &ds402Control.operatingMode);	//DS402 operating mode control
    rxPdoAssignement.addEntry(0x607A, 0x0, 32, "PPp_target", &PPp_target);					//Position Target
    rxPdoAssignement.addEntry(0x3008, 0x11, 16, "IO_DQ_set", &IO_DQ_set);					//Digital Outputs

    txPdoAssignement.addNewModule(0x1A03);
    txPdoAssignement.addEntry(0x6041, 0x0, 16, "_DCOMstatus", &ds402Status.statusWord);		//DS402 status word
    txPdoAssignement.addEntry(0x6061, 0x0, 8, "_DCOMopmd_act", &ds402Status.operatingMode);	//DS402 operating mode display
    txPdoAssignement.addEntry(0x6064, 0x0, 32, "_p_act", &_p_act);							//Actual Position
	txPdoAssignement.addEntry(0x301E, 0x14, 32, "_p_dif_usr", &_p_dif_usr);					//Following Error
    txPdoAssignement.addEntry(0x606C, 0x0, 32, "_v_act", &_v_act);							//Actual Velocity
    txPdoAssignement.addEntry(0x301E, 0x3, 16, "_I_act", &_I_act);							//Actual Current
    txPdoAssignement.addEntry(0x603F, 0x0, 16, "_LastError", &_LastError);					//Current Error
    txPdoAssignement.addEntry(0x3008, 0x1, 16, "_IO_act", &_IO_act);						//Digital Inputs
    txPdoAssignement.addEntry(0x3008, 0x26, 16, "_IO_STO_act", &_IO_STO_act);				//STO Status
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

    //on quickstop, come to a stop using a torque ramp (intensity defined by max quickstop current)
    //transition to operating state Quickstop, this allows us to go back to operationEnabled and move the motor while the limit signal is still triggered
    //if we choose to trigger the fault state, we would not be able to go back to operationEnabled while the limit signal is still high
    //int16_t LIM_QStopReact_set = 7;
    //if (!writeSDO_S16(0x3006, 0x18, LIM_QStopReact_set)) return false;
	//actually we dont need to set this, since it was uploaded as a configuration parameter

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

    //get max velocity and set as velocity limit

    uint16_t _M_n_max; //max motor spec velocity in rpm
    if (!readSDO_U16(0x300D, 0x4, _M_n_max)) return false;
    maxMotorVelocity_rps = (double)_M_n_max / 60.0;

    uint32_t CTRL_v_max = _M_n_max * velocityUnitsPerRpm; //Velocity Limit in usr_v units
    if (!writeSDO_U32(0x3011, 0x10, CTRL_v_max)) return false;

	int8_t HMmethod = 35;
	if(!writeSDO_S8(0x6098, 0x0, HMmethod)) return false;
	
	int32_t HMp_setP = 0;
	if(!writeSDO_S32(0x301B, 0x16, HMp_setP)) return false;

    //=============== PROCESS DATA ASSIGNEMENT ===============

    rxPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C12);
    txPdoAssignement.mapToSyncManager(getSlaveIndex(), 0x1C13);

    //=========================== TIMING AND SYNC CONFIGURATION ============================

    //set interrupt routine for cyclic synchronous position mode
    //interval should be the same as the frame cycle time, and offset should be zero
    //the frame cycle time is offset 50% from dc_sync time (which is a integer multiple of the interval time)
    //by setting the sync0 event at 0 offset, maximum time offset is garanteed between the sync event and the frame receive time
    uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0;
    uint32_t sync0offset_nanoseconds = sync0Interval_nanoseconds / 2;
    ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);

    return true;
}




//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void Lexium32::readInputs() {

	uint16_t previousError = _LastError;
	
    txPdoAssignement.pullDataFrom(identity->inputs);
	
	if(b_isHoming && _p_act == 0) b_isHoming = false;
	
	//read power state
	auto newPowerState = ds402Status.getPowerState();
	if(newPowerState != actualPowerState){
		std::string message = "Power State changed to " + std::string(Enumerator::getDisplayString(newPowerState));
		pushEvent(message.c_str(), !DS402::isNominal(newPowerState));
	}
	actualPowerState = newPowerState;
	
	//Read Error
	if(_LastError != previousError){
		if(_LastError == 0x0) pushEvent("Error Cleared", false);
		else{
			std::string message = "Error " + getErrorCodeString(_LastError);
			pushEvent(message.c_str(), true);
		}
	}
	b_hasFault = ds402Status.hasFault();
	if(b_hasFault && requestedPowerState == DS402::PowerState::OPERATION_ENABLED){
		requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
	}
	
	//read operating mode
	auto newOperatingMode = ds402Status.getOperatingMode();
	if(newOperatingMode != actualOperatingMode){
		std::string message = "Operating Mode changed to " + std::string(Enumerator::getDisplayString(newOperatingMode));
		pushEvent(message.c_str(), false);
	}
	actualOperatingMode = newOperatingMode;
	
	
	//read STO State
	bool newStoState = _IO_STO_act == 0;
	if (newStoState != servoMotorDevice->b_emergencyStopActive) {
		if (newStoState) pushEvent("STO Activated", true);
		else pushEvent("STO Released", false);
	}
	servoMotorDevice->b_emergencyStopActive = newStoState;
	
	
	actualFollowingError_r = (double)_p_dif_usr / (double)positionUnitsPerRevolution;
	
    //set the encoder position in revolution units and velocity in revolutions per second
    servoMotorDevice->positionRaw_positionUnits = (double)_p_act / (double)positionUnitsPerRevolution;
    servoMotorDevice->velocity_positionUnitsPerSecond = (double)_v_act / ((double)velocityUnitsPerRpm * 60.0);
    servoMotorDevice->b_moving = std::abs(servoMotorDevice->velocity_positionUnitsPerSecond) > 0.03;
    //set motor device load
    servoMotorDevice->load = ((double)_I_act / (double)currentUnitsPerAmp) / maxCurrent_amps;
	
    //assign public input data
	*positionPinValue = servoMotorDevice->getPosition();
	*velocityPinValue = servoMotorDevice->getVelocity();
	*loadPinValue = servoMotorDevice->getLoad();
	
    bool DI0 = (_IO_act & 0x1) != 0x0;
    bool DI1 = (_IO_act & 0x2) != 0x0;
    bool DI2 = (_IO_act & 0x4) != 0x0;
    bool DI3 = (_IO_act & 0x8) != 0x0;
    bool DI4 = (_IO_act & 0x10) != 0x0;
    bool DI5 = (_IO_act & 0x20) != 0x0;
	
	*digitalIn0PinValue = b_invertDI0 ? !DI0 : DI0;
	*digitalIn1PinValue = b_invertDI1 ? !DI1 : DI1;
	*digitalIn2PinValue = b_invertDI2 ? !DI2 : DI2;
	*digitalIn3PinValue = b_invertDI3 ? !DI3 : DI3;
	*digitalIn4PinValue = b_invertDI4 ? !DI4 : DI4;
	*digitalIn5PinValue = b_invertDI5 ? !DI5 : DI5;

    //set actuator subdevice
    servoMotorDevice->b_ready = (actualOperatingMode == DS402::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION ||
								 actualOperatingMode == DS402::OperatingMode::HOMING)
								&& (actualPowerState == DS402::PowerState::READY_TO_SWITCH_ON ||
									actualPowerState == DS402::PowerState::SWITCHED_ON ||
									actualPowerState == DS402::PowerState::OPERATION_ENABLED ||
									actualPowerState == DS402::PowerState::QUICKSTOP_ACTIVE);
    servoMotorDevice->b_enabled = actualPowerState == DS402::PowerState::OPERATION_ENABLED;
    servoMotorDevice->b_online = isConnected();
    //set gpio subdevice status
    gpioDevice->b_ready = isConnected(); //gpio is always ready when lexium is in ESM operational state
    gpioDevice->b_online = isConnected();
}





//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void Lexium32::writeOutputs() {
	
	if (digitalOut0->isConnected()) digitalOut0->copyConnectedPinValue();
	if (digitalOut1->isConnected()) digitalOut1->copyConnectedPinValue();
	if (digitalOut2->isConnected()) digitalOut2->copyConnectedPinValue();
	
	if(b_startHoming){
		b_startHoming = false;
		b_isHoming = true;
	}
	
	if(b_isHoming){
		servoMotorDevice->setPositionCommand(servoMotorDevice->getPosition(), servoMotorDevice->getVelocity(), 0.0);
	}
	else if(!servoMotorLink->isConnected()){
		if(!servoMotorDevice->isEnabled()){
			//------ motor profile follows actual data ------
			servoMotorDevice->setPositionCommand(servoMotorDevice->getPosition(), servoMotorDevice->getVelocity(), 0.0);
		}else{
			//------ internal profile generator ------
			double deltaT_seconds = EtherCatFieldbus::getCycleTimeDelta_seconds();
			double deltaV_rps = manualAcceleration_rpsps * deltaT_seconds;
			
			double previousVelocityCommand = servoMotorDevice->velocityCommand_deviceUnitsPerSecond;
			double newVelocityCommand;
			double accelerationCommand;
			if (manualVelocityCommand_rps > previousVelocityCommand) {
				newVelocityCommand = std::min(previousVelocityCommand + deltaV_rps, (double)manualVelocityCommand_rps);
				accelerationCommand = manualAcceleration_rpsps;
			}
			else if (manualVelocityCommand_rps < previousVelocityCommand) {
				newVelocityCommand = std::max(previousVelocityCommand - deltaV_rps, (double)manualVelocityCommand_rps);
				accelerationCommand = -manualAcceleration_rpsps;
			}else newVelocityCommand = previousVelocityCommand;
			double deltaP_r = deltaT_seconds * (previousVelocityCommand + newVelocityCommand) / 2.0;
			servoMotorDevice->setPositionCommand(servoMotorDevice->getPositionCommand() + deltaP_r,
												 newVelocityCommand,
												 accelerationCommand);
		}
	}
	
    //handle power state transitions
    if (servoMotorDevice->b_setEnabled) {
        servoMotorDevice->b_setEnabled = false;
		requestedPowerState = DS402::PowerState::OPERATION_ENABLED;
    }
    if (servoMotorDevice->b_setDisabled) {
        servoMotorDevice->b_setDisabled = false;
		requestedPowerState = DS402::PowerState::READY_TO_SWITCH_ON;
    }
    if (servoMotorDevice->b_setQuickstop) {
        servoMotorDevice->b_setQuickstop = false;
		requestedPowerState = DS402::PowerState::QUICKSTOP_ACTIVE;
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
	
	//if there is an error present, automatically clear it
	//the fault reset event only happens on a transition from 0 to 1
	//we have to clear the fault reset flag after every activation
	//to be able to use it again
	if(b_hasFault && !b_isResettingFault) {
		b_isResettingFault = true;
		ds402Control.performFaultReset();
	}else b_isResettingFault = false;
	ds402Control.updateControlWord();
	
	//DCOMopmode
	if(b_isHoming) ds402Control.setOperatingMode(DS402::OperatingMode::HOMING);
	else ds402Control.setOperatingMode(DS402::OperatingMode::CYCLIC_SYNCHRONOUS_POSITION);
	
	//PPp_target
	PPp_target = (int32_t)(servoMotorDevice->getPositionCommandRaw() * positionUnitsPerRevolution);
	
	//IO_DQ_set
    IO_DQ_set = 0;
    if (*digitalOut0PinValue) IO_DQ_set |= 0x1;
    if (*digitalOut1PinValue) IO_DQ_set |= 0x2;
    if (*digitalOut2PinValue) IO_DQ_set |= 0x4;

    rxPdoAssignement.pushDataTo(identity->outputs);
}






//==============================================================
//======================= I/O ASSIGNEMENT ======================
//==============================================================


void Lexium32::uploadPinAssignements() {

    pinAssignementUploadState = DataTransferState::TRANSFERRING;

    for (auto& type : Enumerator::getTypes<InputPin>()) {
        if (type.enumerator == InputPin::NONE) continue;
        EtherCatCoeData IOfunct_DIx(0x3007, getInputPinSubindex(type.enumerator), EtherCatData::Type::UINT16_T);
        if (negativeLimitSwitchPin == type.enumerator) IOfunct_DIx.setU16(23);
        else if (positiveLimitSwitchPin == type.enumerator) IOfunct_DIx.setU16(22);
        else IOfunct_DIx.setU16(1);
        if (!IOfunct_DIx.write(getSlaveIndex())) goto transferfailed;
    }

    {
        EtherCatCoeData IOsigLIMN(0x3006, 0xF, EtherCatData::Type::UINT16_T);
        EtherCatCoeData IOsigLIMP(0x3006, 0x10, EtherCatData::Type::UINT16_T);

        if (negativeLimitSwitchPin == InputPin::NONE) IOsigLIMN.setU16(0);
        else if (b_negativeLimitSwitchNormallyClosed) IOsigLIMN.setU16(1);
        else IOsigLIMN.setU16(2);

        if (positiveLimitSwitchPin == InputPin::NONE) IOsigLIMP.setU16(0);
        else if (b_positiveLimitSwitchNormallyClosed) IOsigLIMP.setU16(1);
        else IOsigLIMP.setU16(2);

        if (!IOsigLIMN.write(getSlaveIndex())) goto transferfailed;
        if (!IOsigLIMP.write(getSlaveIndex())) goto transferfailed;
    }

    {
        //set all output pins to freely available by default
        EtherCatCoeData IOfunct_DQ0(0x3007, 0x9, EtherCatData::Type::UINT16_T);
        EtherCatCoeData IOfunct_DQ1(0x3007, 0xA, EtherCatData::Type::UINT16_T);
        EtherCatCoeData IOfunct_DQ2(0x3007, 0xB, EtherCatData::Type::UINT16_T);
        IOfunct_DQ0.setU16(1);
        IOfunct_DQ1.setU16(1);
        IOfunct_DQ2.setU16(1);
        if (!IOfunct_DQ0.write(getSlaveIndex())) goto transferfailed;
        if (!IOfunct_DQ1.write(getSlaveIndex())) goto transferfailed;
        if (!IOfunct_DQ2.write(getSlaveIndex())) goto transferfailed;
    }

    pinAssignementUploadState = DataTransferState::SAVING;
    if (!saveToEEPROM()) goto transferfailed;
    pinAssignementUploadState = DataTransferState::SAVED;
    Logger::warn("Pin Assignement Successfull");
    return;

transferfailed:
    pinAssignementUploadState = DataTransferState::FAILED;
    Logger::warn("Pin Assignement Failed");
    return;
}


void Lexium32::downloadPinAssignements() {
    pinAssignementDownloadState = DataTransferState::NO_TRANSFER;

    for (auto& type :Enumerator::getTypes<InputPin>()) {
        if (type.enumerator == InputPin::NONE) continue;
        EtherCatCoeData IOfunct_DIx(0x3007, getInputPinSubindex(type.enumerator), EtherCatData::Type::UINT16_T);
        if (!IOfunct_DIx.read(getSlaveIndex())) goto transferfailed;
        uint16_t pinFunction = IOfunct_DIx.getU16();
        //for each pin, read the function assignement stored on the drive
        switch (pinFunction) {
        case 23:
            negativeLimitSwitchPin = type.enumerator;
            break;
        case 22:
            positiveLimitSwitchPin = type.enumerator;
            break;
        case 1:
        default:
            break;
        }
    }

    {
        EtherCatCoeData IOsigLIMN(0x3006, 0xF, EtherCatData::Type::UINT16_T);
        EtherCatCoeData IOsigLIMP(0x3006, 0x10, EtherCatData::Type::UINT16_T);
        if (!IOsigLIMN.read(getSlaveIndex())) goto transferfailed;
        if (!IOsigLIMP.read(getSlaveIndex())) goto transferfailed;

        if (IOsigLIMN.getU16() == 1) b_negativeLimitSwitchNormallyClosed = true;
        else if (IOsigLIMN.getU16() == 2) b_negativeLimitSwitchNormallyClosed = false;

        if (IOsigLIMP.getU16() == 1) b_positiveLimitSwitchNormallyClosed = true;
        else if (IOsigLIMP.getU16() == 2) b_positiveLimitSwitchNormallyClosed = false;
    }

    pinAssignementDownloadState = DataTransferState::SUCCEEDED;
    return;

transferfailed:
    pinAssignementDownloadState = DataTransferState::FAILED;
    return;
}





//==============================================================
//===================== GENERAL PARAMETERS =====================
//==============================================================

void Lexium32::uploadGeneralParameters() {
    generalParameterUploadState = DataTransferState::TRANSFERRING;

	{
		EtherCatCoeData MON_p_dif_load_usr(0x3006, 0x3E, EtherCatData::Type::INT32_T);
		MON_p_dif_load_usr.setS32(servoMotorDevice->maxfollowingError * positionUnitsPerRevolution);
		if(!MON_p_dif_load_usr.write(getSlaveIndex())) goto transferfailed;
	}
	
    {
        EtherCatCoeData CTRL_I_max(0x3011, 0xC, EtherCatData::Type::UINT16_T);
        CTRL_I_max.setU16(maxCurrent_amps * 100.0);
        if (!CTRL_I_max.write(getSlaveIndex())) goto transferfailed;
    }

    {
        EtherCatCoeData LIM_QStopReact(0x3006, 0x18, EtherCatData::Type::INT16_T);
        switch (quickstopReaction) {
            case QuickStopReaction::DECELERATION_RAMP:
                LIM_QStopReact.setS16(6);
                break;
            case QuickStopReaction::TORQUE_RAMP:
                LIM_QStopReact.setS16(7);
                break;
        }
        if (!LIM_QStopReact.write(getSlaveIndex())) goto transferfailed;
    }

    switch (quickstopReaction) {
        case QuickStopReaction::DECELERATION_RAMP: {
            EtherCatCoeData RAMPquickstop(0x3006, 0x12, EtherCatData::Type::UINT32_T);
            RAMPquickstop.setU32(quickStopDeceleration_revolutionsPerSecondSquared * 60.0);
            if (!RAMPquickstop.write(getSlaveIndex())) goto transferfailed;
        }break;
        case QuickStopReaction::TORQUE_RAMP: {
            EtherCatCoeData LIM_I_maxQSTP(0x3011, 0xD, EtherCatData::Type::UINT16_T);
            LIM_I_maxQSTP.setU16(maxQuickstopCurrent_amps * 100.0);
            if (!LIM_I_maxQSTP.write(getSlaveIndex())) goto transferfailed;
        }break;
    }

    {
        EtherCatCoeData InvertDirOfMove(0x3006, 0xC, EtherCatData::Type::UINT16_T);
        InvertDirOfMove.setU16((b_invertDirectionOfMotorMovement ? 1 : 0));
        if (!InvertDirOfMove.write(getSlaveIndex())) goto transferfailed;
    }

    generalParameterUploadState = DataTransferState::SAVING;
    if (!saveToEEPROM()) goto transferfailed;
    generalParameterUploadState = DataTransferState::SAVED;
    return;

transferfailed:
    generalParameterUploadState = DataTransferState::FAILED;
    return;
}


void Lexium32::downloadGeneralParameters() {
    generalParameterDownloadState = DataTransferState::TRANSFERRING;

	{
		EtherCatCoeData MON_p_dif_load_usr(0x3006, 0x3E, EtherCatData::Type::INT32_T);
		if(!MON_p_dif_load_usr.read(getSlaveIndex())) goto transferfailed;
		servoMotorDevice->maxfollowingError = (float)MON_p_dif_load_usr.getS32() / positionUnitsPerRevolution;
	}
	
    {
        EtherCatCoeData CTRL_I_max(0x3011, 0xC, EtherCatData::Type::UINT16_T);
        if (!CTRL_I_max.read(getSlaveIndex())) goto transferfailed;
        maxCurrent_amps = (float)CTRL_I_max.getU16() / 100.0;
    }

    {
        EtherCatCoeData LIM_QStopReact(0x3006, 0x18, EtherCatData::Type::INT16_T);
        if (!LIM_QStopReact.read(getSlaveIndex())) goto transferfailed;
        switch (LIM_QStopReact.getS16()) {
            case -1:
            case 6:
                quickstopReaction = QuickStopReaction::DECELERATION_RAMP;
                break;
            case -2:
            case 7:
                quickstopReaction = QuickStopReaction::TORQUE_RAMP;
                break;
            default:
                goto transferfailed;
        }
    }

    {
        EtherCatCoeData _M_n_max(0x300D, 0x4, EtherCatData::Type::UINT16_T);
        if (!_M_n_max.read(getSlaveIndex())) goto transferfailed;
        maxMotorVelocity_rps = (double)_M_n_max.getU16() / 60.0;
    }

    switch (quickstopReaction) {
        case QuickStopReaction::DECELERATION_RAMP: {
            EtherCatCoeData RAMPquickstop(0x3006, 0x12, EtherCatData::Type::UINT32_T);
            if (!RAMPquickstop.read(getSlaveIndex())) goto transferfailed;
            quickStopDeceleration_revolutionsPerSecondSquared = ((double)RAMPquickstop.getU32() / (double)accelerationUnitsPerRpmps) / 60.0;
        }break;
        case QuickStopReaction::TORQUE_RAMP: {
            EtherCatCoeData LIM_I_maxQSTP(0x3011, 0xD, EtherCatData::Type::UINT16_T);
            if (!LIM_I_maxQSTP.read(getSlaveIndex())) goto transferfailed;
            maxQuickstopCurrent_amps = (float)LIM_I_maxQSTP.getU16() / 100.0;
        }break;
    }

    {
        EtherCatCoeData InvertDirOfMove(0x3006, 0xC, EtherCatData::Type::UINT16_T);
        if (!InvertDirOfMove.read(getSlaveIndex())) goto transferfailed;
        uint16_t dir = InvertDirOfMove.getU16();
        if (dir == 1) b_invertDirectionOfMotorMovement = true;
        else if (dir == 0) b_invertDirectionOfMotorMovement = false;
        else goto transferfailed;
    }

    generalParameterDownloadState = DataTransferState::SUCCEEDED;
    return;

transferfailed:
    generalParameterDownloadState = DataTransferState::FAILED;
    return;
}






//==============================================================
//====================== ENCODER SETTINGS ======================
//==============================================================

void Lexium32::detectEncoderModule() {
    encoderModuleType = EncoderModule::NONE;
	
    EtherCatCoeData _ModuleSlot2(0x3002, 0x1A, EtherCatData::Type::UINT16_T);
    if (!_ModuleSlot2.read(getSlaveIndex())) return;
	
	encoderModuleType = getEncoderModule(_ModuleSlot2.getU16());
}

void Lexium32::uploadEncoderSettings() {

    encoderSettingsUploadState = DataTransferState::TRANSFERRING;

    //switch between internal motor encoder and encoder module
    EtherCatCoeData ENC_abs_source(0x3005, 0x25, EtherCatData::Type::UINT16_T);

    //general settings for encoder2 if encoder module is selected
    EtherCatCoeData ENC2_type(0x3050, 0x3, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENC2_usage(0x3050, 0x1, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENC_ModeOfMaEnc(0x3050, 0x2, EtherCatData::Type::UINT16_T);
    EtherCatCoeData InvertDirOfMaEnc(0x3050, 0x8, EtherCatData::Type::UINT16_T);
    EtherCatCoeData p_MaxDifToENC2(0x3050, 0x7, EtherCatData::Type::INT32_T);
    EtherCatCoeData ResolENC2(0x3050, 0xF, EtherCatData::Type::UINT32_T);
    EtherCatCoeData ResolENC2Num(0x3050, 0x6, EtherCatData::Type::INT32_T);
    EtherCatCoeData ResolENC2Denom(0x3050, 0x5, EtherCatData::Type::INT32_T);

    //general settings for the digital encoder module
    EtherCatCoeData ENCDigPowSupply(0x3052, 0x4, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENCDigResMulUsed(0x3052, 0xB, EtherCatData::Type::UINT16_T);

    //settings for digital SSI rotary encoders
    EtherCatCoeData ENCDigSSICoding(0x3052, 0x3, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENCDigSSIMaxFreq(0x3052, 0x5, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENCDigSSIResMult(0x3052, 0x2, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENCDigSSIResSgl(0x3052, 0x1, EtherCatData::Type::UINT16_T);

    switch (encoderAssignement) {

    case EncoderAssignement::INTERNAL_ENCODER:
    {
        ENC_abs_source.setU16(0); //INTERNAL MOTOR ENCODER
        ENC2_type.setU16(0);      //ENCODER 2 TYPE (none)
        ENC2_usage.setU16(0);     //ENCODER 2 USAGE(none)
        if (!ENC_abs_source.write(getSlaveIndex())) goto transferfailed;
        if (!ENC2_type.write(getSlaveIndex())) goto transferfailed;
        if (!ENC2_usage.write(getSlaveIndex())) goto transferfailed;
    }
    break;

    case EncoderAssignement::ENCODER_MODULE:
    {
        //encoder usage settings for encoder module
        ENC_abs_source.setU16(1); //absolute position reference is encoder 2 (encoder module)
        ENC2_usage.setU16(2); //use as machine encoder (as motor encoder doesn't work)
        ENC_ModeOfMaEnc.setU16(2); //use for position and velocity control
        InvertDirOfMaEnc.setU16((encoder2_invertDirection ? 1 : 0));
        p_MaxDifToENC2.setS32((encoder2_maxDifferenceToMotorEncoder_rotations * (float)(0x1 << encoder1_singleTurnResolutionBits))); //max difference between machine and motor encoder before error triggered
        if (!ENC_abs_source.write(getSlaveIndex())) goto transferfailed;
        if (!ENC2_usage.write(getSlaveIndex())) goto transferfailed;
        if (!ENC_ModeOfMaEnc.write(getSlaveIndex())) goto transferfailed;
        if (!InvertDirOfMaEnc.write(getSlaveIndex())) goto transferfailed;
        if (!p_MaxDifToENC2.write(getSlaveIndex())) goto transferfailed;
    }

    switch (encoderModuleType) {

    case EncoderModule::DIGITAL_MODULE:
    {
        //digital encoder module setting
        switch (encoderVoltage) { //which power supply the digital encoder uses
        case EncoderVoltage::V5: ENCDigPowSupply.setU16(5); break;
        case EncoderVoltage::V12: ENCDigPowSupply.setU16(12); break;
        }
        if (!ENCDigPowSupply.write(getSlaveIndex())) goto transferfailed;

        //general encoder 2 scaling settings
        ResolENC2.setU32(0x1 << encoder2_singleTurnResolutionBits);                                      //Increments per encoder rotation (singleturn resolution)
        ResolENC2Num.setS32((0x1 << encoder2_singleTurnResolutionBits) * encoder2_EncoderToMotorRatioNumerator); //encoder increments
        ResolENC2Denom.setS32(encoder2_EncoderToMotorRatioDenominator);                                              //per motor revolution
        if (!ResolENC2.write(getSlaveIndex())) goto transferfailed;
        if (!ResolENC2Denom.write(getSlaveIndex())) goto transferfailed;
        if (!ResolENC2Num.write(getSlaveIndex())) goto transferfailed;
    }

    switch (encoderType) {
    case EncoderType::SSI_ROTARY:
    {
        ENC2_type.setU16(10); //encoder type is ssi rotary
        switch (encoderCoding) { //bit encoding of the encoder
        case EncoderCoding::BINARY: ENCDigSSICoding.setU16(0); break;
        case EncoderCoding::GRAY: ENCDigSSICoding.setU16(1); break;
        }
        ENCDigSSIMaxFreq.setU16(200);                                //SSI communication frequency in KHz
        ENCDigSSIResMult.setU16(encoder2_multiTurnResolutionBits);   //Multiturn resolution bits
        ENCDigResMulUsed.setU16(0);                                  //amount of multiturn bits user (0 = all)
        ENCDigSSIResSgl.setU16(encoder2_singleTurnResolutionBits);   //Singleturn resolution bits

        if (!ENC2_type.write(getSlaveIndex())) goto transferfailed;
        if (!ENCDigSSICoding.write(getSlaveIndex())) goto transferfailed;
        if (!ENCDigSSIMaxFreq.write(getSlaveIndex())) goto transferfailed;
        if (!ENCDigSSIResMult.write(getSlaveIndex())) goto transferfailed;
        if (!ENCDigResMulUsed.write(getSlaveIndex())) goto transferfailed;
        if (!ENCDigSSIResSgl.write(getSlaveIndex())) goto transferfailed;
    }
    break;

    default: break;

    }//encoder type
    break;

    default: break;

    }//encoder module type


    };//internal encoder or encoder module

    {
        EtherCatCoeData ShiftEncWorkRange(0x3005, 0X21, EtherCatData::Type::UINT16_T);
        if (b_encoderRangeShifted) ShiftEncWorkRange.setU16(1);
        else ShiftEncWorkRange.setU16(0);
        if (!ShiftEncWorkRange.write(getSlaveIndex())) goto transferfailed;
    }

    encoderSettingsUploadState = DataTransferState::SAVING;
    if (!saveToEEPROM()) encoderSettingsUploadState = DataTransferState::FAILED;
    encoderSettingsUploadState = DataTransferState::SAVED;
    Logger::warn("Encoder assignement success");
    return;

transferfailed:
    encoderSettingsUploadState = DataTransferState::FAILED;
    Logger::warn("Transfer Failed");
    return;

}


void Lexium32::downloadEncoderSettings() {

    encoderSettingsDownloadState = DataTransferState::TRANSFERRING;

    //general settings for encoder2 if encoder module is selected
    EtherCatCoeData ENC2_type(0x3050, 0x3, EtherCatData::Type::UINT16_T);
    EtherCatCoeData InvertDirOfMaEnc(0x3050, 0x8, EtherCatData::Type::UINT16_T);
    EtherCatCoeData p_MaxDifToENC2(0x3050, 0x7, EtherCatData::Type::INT32_T);
    EtherCatCoeData ResolENC2Num(0x3050, 0x6, EtherCatData::Type::INT32_T);
    EtherCatCoeData ResolENC2Denom(0x3050, 0x5, EtherCatData::Type::INT32_T);

    //general settings for the digital encoder module
    EtherCatCoeData ENCDigPowSupply(0x3052, 0x4, EtherCatData::Type::UINT16_T);

    //settings for digital SSI rotary encoders
    EtherCatCoeData ENCDigSSICoding(0x3052, 0x3, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENCDigSSIResMult(0x3052, 0x2, EtherCatData::Type::UINT16_T);
    EtherCatCoeData ENCDigSSIResSgl(0x3052, 0x1, EtherCatData::Type::UINT16_T);

    {
        //main switch between internal motor encoder and encoder module
        EtherCatCoeData ENC_abs_source(0x3005, 0x25, EtherCatData::Type::UINT16_T);
        if (!ENC_abs_source.read(getSlaveIndex())) goto downloadfailed;
        encoderAssignement = getEncoderAssignement(ENC_abs_source.getU16());
    }

    switch (encoderAssignement) {
    case EncoderAssignement::INTERNAL_ENCODER:
        //no additionnal settings for internal encoder
        break;
    case EncoderAssignement::ENCODER_MODULE:
        detectEncoderModule();

        {
            if (!ENC2_type.read(getSlaveIndex())) goto downloadfailed;
            encoderType = getEncoderType(ENC2_type.getU16());
            if (!InvertDirOfMaEnc.read(getSlaveIndex())) goto downloadfailed;
            encoder2_invertDirection = InvertDirOfMaEnc.getU16() == 1;
            if (!p_MaxDifToENC2.read(getSlaveIndex())) goto downloadfailed;
            encoder2_maxDifferenceToMotorEncoder_rotations = (float)p_MaxDifToENC2.getS32() / (float)(0x1 << encoder1_singleTurnResolutionBits);
        }

        switch (encoderModuleType) {
        case EncoderModule::ANALOG_MODULE:
        case EncoderModule::RESOLVER_MODULE:
        case EncoderModule::NONE: break;
        case EncoderModule::DIGITAL_MODULE:

        {
            if (!ENCDigPowSupply.read(getSlaveIndex())) goto downloadfailed;
            encoderVoltage = getEncoderVoltage(ENCDigPowSupply.getU16());
        }

        switch (encoderType) {
			case EncoderType::NONE: break;
			case EncoderType::SSI_ROTARY:
        {
            if (!ENCDigSSICoding.read(getSlaveIndex())) goto downloadfailed;
            encoderCoding = getEncoderCoding(ENCDigSSICoding.getU16());
            if (!ENCDigSSIResMult.read(getSlaveIndex())) goto downloadfailed;
            encoder2_multiTurnResolutionBits = ENCDigSSIResMult.getU16();
            if (!ENCDigSSIResSgl.read(getSlaveIndex())) goto downloadfailed;
            encoder2_singleTurnResolutionBits = ENCDigSSIResSgl.getU16();
        }
        break;
        }

        break;
        }

        {
            if (!ResolENC2Num.read(getSlaveIndex())) goto downloadfailed;
            encoder2_EncoderToMotorRatioNumerator = ResolENC2Num.getS32() / (0x1 << encoder2_singleTurnResolutionBits);
            if (!ResolENC2Denom.read(getSlaveIndex())) goto downloadfailed;
            encoder2_EncoderToMotorRatioDenominator = ResolENC2Denom.getS32();
        }

        break;
    }

    {
        EtherCatCoeData ShiftEncWorkRange(0x3005, 0X21, EtherCatData::Type::UINT16_T);
        ShiftEncWorkRange.read(getSlaveIndex());
        if (ShiftEncWorkRange.getU16() == 0) b_encoderRangeShifted = false;
        else if (ShiftEncWorkRange.getU16() == 1) b_encoderRangeShifted = true;
        else goto downloadfailed;
    }

    float lowEncoderRange, highEncoderRange;
    getEncoderWorkingRange(lowEncoderRange, highEncoderRange);
    servoMotorDevice->rangeMin_positionUnits = lowEncoderRange;
    servoMotorDevice->rangeMax_positionUnits = highEncoderRange;

    encoderSettingsDownloadState = DataTransferState::SUCCEEDED;
    return;

downloadfailed:
    encoderSettingsDownloadState = DataTransferState::FAILED;
    return;

}


void Lexium32::uploadManualAbsoluteEncoderPosition() {
    encoderAbsolutePositionUploadState = DataTransferState::TRANSFERRING;
    int absolutePositionEncoderIncrements;
    EtherCatCoeData ENC1_adjustment(0x3005, 0x16, EtherCatData::Type::INT32_T);
    EtherCatCoeData ENC2_adjustement(0x3005, 0x24, EtherCatData::Type::INT32_T);
    switch (encoderAssignement) {
    case EncoderAssignement::INTERNAL_ENCODER:
        absolutePositionEncoderIncrements = manualAbsoluteEncoderPosition_revolutions * (float)(0x1 << encoder1_singleTurnResolutionBits);
        ENC1_adjustment.setS32(absolutePositionEncoderIncrements);
        if (ENC1_adjustment.write(getSlaveIndex())) goto saving;
        else goto failed;
        break;
    case EncoderAssignement::ENCODER_MODULE:
        absolutePositionEncoderIncrements = manualAbsoluteEncoderPosition_revolutions * (float)(0x1 << encoder2_singleTurnResolutionBits);
        ENC2_adjustement.setS32(absolutePositionEncoderIncrements);
        if (ENC2_adjustement.write(getSlaveIndex())) goto saving;
        else goto failed;
        break;
    }

saving:

    encoderAbsolutePositionUploadState = DataTransferState::SAVING;
    if (saveToEEPROM()) encoderAbsolutePositionUploadState = DataTransferState::SAVED;
    else goto failed;
    return;

failed:

    encoderAbsolutePositionUploadState = DataTransferState::FAILED;
    return;
}



void Lexium32::getEncoderWorkingRange(float& low, float& high) {
    switch (encoderAssignement) {
    case EncoderAssignement::INTERNAL_ENCODER:
        if (b_encoderRangeShifted) {
            low = -(float)(0x1 << encoder1_multiTurnResolutionBits) / 2.0;
            high = (float)(0x1 << encoder1_multiTurnResolutionBits) / 2.0;
        }
        else {
            low = 0.0;
            high = (float)(0x1 << encoder1_multiTurnResolutionBits);
        }
        break;
    case EncoderAssignement::ENCODER_MODULE:
        if (b_encoderRangeShifted) {
            low = -(float)(0x1 << encoder2_multiTurnResolutionBits) * (float)encoder2_EncoderToMotorRatioDenominator / ((float)encoder2_EncoderToMotorRatioNumerator * 2.0);
            high = -low;
        }
        else {
            low = 0.0;
            high = (float)(0x1 << encoder2_multiTurnResolutionBits) * (float)encoder2_EncoderToMotorRatioDenominator / (float)encoder2_EncoderToMotorRatioNumerator;
        }
        break;
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

    XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
    kinematicLimitsXML->SetAttribute("velocityLimit_rps", servoMotorDevice->velocityLimit_positionUnitsPerSecond);
    kinematicLimitsXML->SetAttribute("accelerationLimit_rpsps", servoMotorDevice->accelerationLimit_positionUnitsPerSecondSquared);
    kinematicLimitsXML->SetAttribute("defaultManualAcceleration_rpsps", defaultManualAcceleration_rpsps);

    XMLElement* invertDirectionOfMovementXML = xml->InsertNewChildElement("InvertDirectionOfMovement");
    invertDirectionOfMovementXML->SetAttribute("Invert", b_invertDirectionOfMotorMovement);
	
	XMLElement* maxFollowingErrorXML = xml->InsertNewChildElement("MaxFollowingError");
	maxFollowingErrorXML->SetAttribute("revolutions", servoMotorDevice->maxfollowingError);

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
    encoderSettingsXML->SetAttribute("Assignement", Enumerator::getSaveString(encoderAssignement));
    switch (encoderAssignement) {
    case EncoderAssignement::INTERNAL_ENCODER:
        break;
    case EncoderAssignement::ENCODER_MODULE:
        encoderSettingsXML->SetAttribute("EncoderModule", Enumerator::getSaveString(encoderModuleType));
        encoderSettingsXML->SetAttribute("EncoderToMotorRatioNumerator", encoder2_EncoderToMotorRatioNumerator);
        encoderSettingsXML->SetAttribute("EncoderToMotorRatioDenominator", encoder2_EncoderToMotorRatioDenominator);
        encoderSettingsXML->SetAttribute("InvertDirection", encoder2_invertDirection);
        encoderSettingsXML->SetAttribute("MaxDifferenceToMotorEncoder_revolutions", encoder2_maxDifferenceToMotorEncoder_rotations);
        switch (encoderModuleType) {
        case EncoderModule::ANALOG_MODULE: break;
        case EncoderModule::RESOLVER_MODULE: break;
        case EncoderModule::NONE: break;
        case EncoderModule::DIGITAL_MODULE:
            encoderSettingsXML->SetAttribute("EncoderType", Enumerator::getSaveString(encoderType));
            encoderSettingsXML->SetAttribute("Voltage", Enumerator::getSaveString(encoderVoltage));
            encoderSettingsXML->SetAttribute("SingleTurnBits", encoder2_singleTurnResolutionBits);
            encoderSettingsXML->SetAttribute("MultiTurnBits", encoder2_multiTurnResolutionBits);
            switch (encoderType) {
            case EncoderType::NONE: break;
            case EncoderType::SSI_ROTARY:
                encoderSettingsXML->SetAttribute("SSIEncoding", Enumerator::getSaveString(encoderCoding));
                break;
            }
            break;
        }
        break;
    }
    encoderSettingsXML->SetAttribute("RangeShifted", b_encoderRangeShifted);
    encoderSettingsXML->SetAttribute("PositionOffset_revolutions", servoMotorDevice->positionOffset_positionUnits);

    return true;
}


bool Lexium32::loadDeviceData(tinyxml2::XMLElement* xml) {

    using namespace tinyxml2;

    XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
    if (kinematicLimitsXML == nullptr) return Logger::warn("Could not find kinematic limits attribute");

    if (kinematicLimitsXML->QueryDoubleAttribute("velocityLimit_rps", &servoMotorDevice->velocityLimit_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not read velocity limit attribute");
    if (kinematicLimitsXML->QueryDoubleAttribute("accelerationLimit_rpsps", &servoMotorDevice->accelerationLimit_positionUnitsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not read acceleration limit attribute");
    if (kinematicLimitsXML->QueryFloatAttribute("defaultManualAcceleration_rpsps", &defaultManualAcceleration_rpsps) != XML_SUCCESS) return Logger::warn("Could not read acceleration limit attribute");
    manualAcceleration_rpsps = defaultManualAcceleration_rpsps;

    XMLElement* invertDirectionOfMovementXML = xml->FirstChildElement("InvertDirectionOfMovement");
    if (invertDirectionOfMovementXML == nullptr) return Logger::warn("Could not find invert direction of movement attribute");
    if (invertDirectionOfMovementXML->QueryBoolAttribute("Invert", &b_invertDirectionOfMotorMovement) != XML_SUCCESS) return Logger::warn("Could not read direciton of movement attribute");

	XMLElement* maxFollowingErrorXML = xml->FirstChildElement("MaxFollowingError");
	if(maxFollowingErrorXML == nullptr) return Logger::warn("Could not find max following error attribute");
	if(maxFollowingErrorXML->QueryAttribute("revolutions", &servoMotorDevice->maxfollowingError) != XML_SUCCESS) return Logger::warn("Could not read max following error attribute");
	 
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
    const char* encoderAssignementString = "";
    encoderSettingsXML->QueryStringAttribute("Assignement", &encoderAssignementString);
    if (!Enumerator::isValidSaveName<EncoderAssignement>(encoderAssignementString)) return Logger::warn("Could not read encoder assignement attribute");
	encoderAssignement = Enumerator::getEnumeratorFromSaveString<EncoderAssignement>(encoderAssignementString);

    switch (encoderAssignement) {
    case EncoderAssignement::INTERNAL_ENCODER:
        break;
    case EncoderAssignement::ENCODER_MODULE:

        const char* encoderModuleString = "";
        encoderSettingsXML->QueryStringAttribute("EncoderModule", &encoderModuleString);
        if (!Enumerator::isValidSaveName<EncoderModule>(encoderModuleString)) return Logger::warn("Could not read encoder module attribute");
		encoderModuleType = Enumerator::getEnumeratorFromSaveString<EncoderModule>(encoderModuleString);

        if (encoderSettingsXML->QueryIntAttribute("EncoderToMotorRatioNumerator", &encoder2_EncoderToMotorRatioNumerator) != XML_SUCCESS) return Logger::warn("Could not read Encoder to Motor Ratio Numerator Attribute");
        if (encoderSettingsXML->QueryIntAttribute("EncoderToMotorRatioDenominator", &encoder2_EncoderToMotorRatioDenominator) != XML_SUCCESS) return Logger::warn("Could not read Encoder to Motor Ratio Denominator Attribute");
        if (encoderSettingsXML->QueryBoolAttribute("InvertDirection", &encoder2_invertDirection) != XML_SUCCESS) return Logger::warn("Could not read Invert Encoder Direction Attribute");
        if (encoderSettingsXML->QueryDoubleAttribute("MaxDifferenceToMotorEncoder_revolutions", &encoder2_maxDifferenceToMotorEncoder_rotations) != XML_SUCCESS) return Logger::warn("Could not read Max Encoder to Motor Encoder Difference Attribute");

        switch (encoderModuleType) {
        case EncoderModule::ANALOG_MODULE: break;
        case EncoderModule::RESOLVER_MODULE: break;
        case EncoderModule::NONE: break;
        case EncoderModule::DIGITAL_MODULE:

            const char* encoderTypeString = "";
            encoderSettingsXML->QueryStringAttribute("EncoderType", &encoderTypeString);
            if (!Enumerator::isValidSaveName<EncoderType>(encoderTypeString)) return Logger::warn("Could not read encoder type attribute");
			encoderType = Enumerator::getEnumeratorFromSaveString<EncoderType>(encoderTypeString);

            const char* encoderVoltageString = "";
            encoderSettingsXML->QueryStringAttribute("Voltage", &encoderVoltageString);
            if (!Enumerator::isValidSaveName<EncoderVoltage>(encoderVoltageString)) return Logger::warn("Could not read encoder voltage attribute");
			encoderVoltage = Enumerator::getEnumeratorFromSaveString<EncoderVoltage>(encoderVoltageString);

            if (encoderSettingsXML->QueryIntAttribute("SingleTurnBits", &encoder2_singleTurnResolutionBits) != XML_SUCCESS) return Logger::warn("Could not read encoder Single Turn bit count attribute");
            if (encoderSettingsXML->QueryIntAttribute("MultiTurnBits", &encoder2_multiTurnResolutionBits) != XML_SUCCESS) return Logger::warn("Could not read encoder Multi Turn bit count attribute");

            switch (encoderType) {
            case EncoderType::NONE: break;
            case EncoderType::SSI_ROTARY:
                const char* SSIencodingString = "";
				if(encoderSettingsXML->QueryStringAttribute("SSIEncoding", &SSIencodingString) != XML_SUCCESS) return Logger::warn("Could not read ssi encoder encoding type");
                if (!Enumerator::isValidSaveName<EncoderCoding>(SSIencodingString)) return Logger::warn("Could not read SSI encoding attribute");
				encoderCoding = Enumerator::getEnumeratorFromSaveString<EncoderCoding>(SSIencodingString);
                break;
            }
            break;
        }
        break;
    }
    if (encoderSettingsXML->QueryBoolAttribute("RangeShifted", &b_encoderRangeShifted) != XML_SUCCESS) return Logger::warn("Could not find encoder range shift attribute");
    if (encoderSettingsXML->QueryDoubleAttribute("PositionOffset_revolutions", &servoMotorDevice->positionOffset_positionUnits) != XML_SUCCESS) return Logger::warn("Could not find position offset attribute");
    float lowEncoderRange, highEncoderRange;
    getEncoderWorkingRange(lowEncoderRange, highEncoderRange);
    servoMotorDevice->rangeMin_positionUnits = lowEncoderRange;
    servoMotorDevice->rangeMax_positionUnits = highEncoderRange;

    return true;
}
