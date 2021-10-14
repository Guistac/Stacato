#include <pch.h>

#include "Lexium32.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

bool Lexium32::isDeviceReady() {
    switch (state) {
        case State::OperationEnabled:
        case State::SwitchedOn:
        case State::ReadyToSwitchOn: return true;
        default: return false;
    }
}

void Lexium32::enable() {
    b_enableOperation = true;
    manualVelocityCommand_rps = 0.0;
    profileVelocity_rps = 0.0;
    profilePosition_r = 0.0;
}

void Lexium32::disable() {
    b_disableOperation = true;
    manualVelocityCommand_rps = 0.0;
    profileVelocity_rps = 0.0;
    profilePosition_r = 0.0;
}

bool Lexium32::isEnabled() { 
    return state == State::OperationEnabled;
}

void Lexium32::onConnection() {
    resetData();
}

void Lexium32::onDisconnection() {
    resetData();
}

void Lexium32::resetData() {
    actualOperatingMode = OperatingMode::Mode::UNKNOWN;
    state = State::SwitchOnDisabled;
    manualVelocityCommand_rps = 0.0;
    profilePosition_r = 0.0;
    profileVelocity_rps = 0.0;
    b_emergencyStopActive = false;
    encoderDevice->positionRaw_positionUnits = 0.0;
    actualPosition->set(0.0);
    actualVelocity->set(0.0);
    actualLoad->set(0.0);
    digitalIn0->set(false);
    digitalIn1->set(false);
    digitalIn2->set(false);
    digitalIn3->set(false);
    digitalIn4->set(false);
    digitalIn5->set(false);
    b_switchedOn = false;
    b_voltageEnabled = false;
    b_quickStopActive = true; //quickstop is active when bit is low
    b_operationEnabled = false;
    opModeSpec4 = false;
    opModeSpec5 = false;
    opModeSpec6 = false;
    b_faultResetState = false;
    b_halted = false;
    opModeSpec9 = false;
    encoderDevice->b_detected = false;
    encoderDevice->b_online = false;
    encoderDevice->b_ready = false;
    motorDevice->b_detected = false;
    motorDevice->b_online = false;
    motorDevice->b_ready = false;
    motorDevice->b_enabled = false;
    gpioDevice->b_detected = false;
    gpioDevice->b_online = false;
    gpioDevice->b_ready = false;
}

void Lexium32::assignIoData() {
    std::shared_ptr<Device> thisDevice = std::dynamic_pointer_cast<Device>(shared_from_this());
    
    motorDevice->setParentDevice(thisDevice);
    motorDevice->velocityLimit_positionUnitsPerSecond = 10.0;
    motorDevice->accelerationLimit_positionUnitsPerSecondSquared = 1.0;
    
    motorLink->set(motorDevice);
    encoderDevice->setParentDevice(thisDevice);
    encoderLink->set(encoderDevice);
    float lowEncoderRange, highEncoderRange;
    getEncoderWorkingRange(lowEncoderRange, highEncoderRange);
    encoderDevice->rangeMin_positionUnits = lowEncoderRange;
    encoderDevice->rangeMax_positionUnits = highEncoderRange;

    gpioDevice->setParentDevice(thisDevice);
    gpNodeLink->set(gpioDevice);

    //node input data
    addIoData(positionCommand);
    addIoData(velocityCommand);
    addIoData(digitalOut0);
    addIoData(digitalOut1);
    addIoData(digitalOut2);
    
    //node output data
    addIoData(motorLink);
    addIoData(actualLoad);
    addIoData(encoderLink);
    addIoData(actualPosition);
    addIoData(actualVelocity);
    addIoData(gpNodeLink);
    addIoData(digitalIn0);
    addIoData(digitalIn1);
    addIoData(digitalIn2);
    addIoData(digitalIn3);
    addIoData(digitalIn4);
    addIoData(digitalIn5);

    rxPdoAssignement.addNewModule(0x1603);
    rxPdoAssignement.addEntry(0x6040, 0x0, 2, "DCOMcontrol", &DCOMcontrol);
    rxPdoAssignement.addEntry(0x6060, 0x0, 1, "DCOMopmode", &DCOMopmode);
    rxPdoAssignement.addEntry(0x607A, 0x0, 4, "PPp_target", &PPp_target);
    rxPdoAssignement.addEntry(0x60FF, 0x0, 4, "PVv_target", &PVv_target);
    rxPdoAssignement.addEntry(0x3008, 0x11, 2, "IO_DQ_set", &IO_DQ_set);

    txPdoAssignement.addNewModule(0x1A03);
    txPdoAssignement.addEntry(0x6041, 0x0, 2, "_DCOMstatus", &_DCOMstatus);
    txPdoAssignement.addEntry(0x6061, 0x0, 1, "_DCOMopmd_act", &_DCOMopmd_act);
    txPdoAssignement.addEntry(0x6064, 0x0, 4, "_p_act", &_p_act);
    txPdoAssignement.addEntry(0x606C, 0x0, 4, "_v_act", &_v_act);
    txPdoAssignement.addEntry(0x301E, 0x3, 2, "_I_act", &_I_act);
    txPdoAssignement.addEntry(0x603F, 0x0, 2, "_LastError", &_LastError);
    txPdoAssignement.addEntry(0x3008, 0x1, 2, "_IO_act", &_IO_act);
    txPdoAssignement.addEntry(0x3008, 0x26, 2, "_IO_STO_act", &_IO_STO_act);
}






//==============================================================
//==================== STARTUP CONFIGURATION ===================
//==============================================================

bool Lexium32::startupConfiguration() {

    //Cia DS402 mandatory Startup Settings (According to Lexium32 EtherCAT module documentation)

    uint16_t CompParSyncMot_set = 1;
    if (!writeSDO_U16(0x3006, 0x3D, CompParSyncMot_set)) return false;

    uint16_t MOD_enable_set = 0;
    if (!writeSDO_U16(0x3006, 0x38, MOD_enable_set)) return false;

    int16_t LIM_QStopReact_set = -1;
    if (!writeSDO_S16(0x3006, 0x18, LIM_QStopReact_set)) return false;

    uint16_t IOSigRespOfPS_set = 1;
    if (!writeSDO_U16(0x3006, 0x6, IOSigRespOfPS_set)) return false;

    int32_t ScalePOSdenom_set = positionUnitsPerRevolution;
    if (!writeSDO_S32(0x3006, 0x7, ScalePOSdenom_set)) return false;

    int32_t ScalePOSnum_set = 1;
    if (!writeSDO_S32(0x3006, 0x8, ScalePOSnum_set)) return false;

    uint16_t CTRL1_KFPp = 1000;
    if (!writeSDO_U16(0x3012, 0x6, CTRL1_KFPp)) return false;

    uint16_t CTRL2_KFPp = 1000;
    if (!writeSDO_U16(0x3013, 0x6, CTRL2_KFPp)) return false;

    int8_t DCOMopmode_set = 8;
    if (!writeSDO_S8(0x6060, 0x0, DCOMopmode_set)) return false;

    uint32_t ECATinpshifttime_set = 250000;
    if (!writeSDO_U32(0x1C33, 0x3, ECATinpshifttime_set)) return false;

    //Upload velocity unit scaling

    int32_t ScaleVELdenom = velocityUnitsPerRpm;
    if (!writeSDO_S32(0x3006, 0x21, ScaleVELdenom)) return false;

    int32_t ScaleVELnom = 1;
    if (!writeSDO_S32(0x3006, 0x22, ScaleVELnom)) return false;

    //get max velocity and set as velocity limit

    uint16_t _M_n_max; //max motor spec velocity in rpm
    if (!readSDO_U16(0x300D, 0x4, _M_n_max)) return false;
    maxMotorVelocity_rps = (double)_M_n_max / 60.0;

    uint32_t CTRL_v_max = _M_n_max * velocityUnitsPerRpm; //Velocity Limit in usr_v units
    if (!writeSDO_U32(0x3011, 0x10, CTRL_v_max)) return false;

    //=============== PROCESS DATA ASSIGNEMENT =============== 

    uint16_t RxPDOmodule = 0x1603;
    if (!writeSDO_U8(RxPDOmodule, 0x0, 0)) return false;            //disable PDO Module
    if (!writeSDO_U32(RxPDOmodule, 0x1, 0x60400010)) return false;  //DCOMcontrol    (uint16_t)
    if (!writeSDO_U32(RxPDOmodule, 0x2, 0x60600008)) return false;  //DCOMopmode     (int8_t)
    if (!writeSDO_U32(RxPDOmodule, 0x3, 0x607A0020)) return false;  //PPp_target     (int32_t)
    if (!writeSDO_U32(RxPDOmodule, 0x4, 0x60FF0020)) return false;  //PVv_target     (int32_t)
    if (!writeSDO_U32(RxPDOmodule, 0x5, 0x30081110)) return false;  //IO_DQ_set      (uint16_t)
    if (!writeSDO_U8(RxPDOmodule, 0x0, 5)) return false;            //5 parameters

    uint16_t TxPDOmodule = 0x1A03;
    if (!writeSDO_U8(TxPDOmodule, 0x0, 0)) return false;            //disable PDO Module
    if (!writeSDO_U32(TxPDOmodule, 0x1, 0x60410010)) return false;  //_DCOMstatus   (uint16_t)
    if (!writeSDO_U32(TxPDOmodule, 0x2, 0x60610008)) return false;  //_DCOMopmd_act (uint8_t) 
    if (!writeSDO_U32(TxPDOmodule, 0x3, 0x60640020)) return false;  //_p_act        (int32_t) 
    if (!writeSDO_U32(TxPDOmodule, 0x4, 0x606C0020)) return false;  //_v_act        (int32_t) 
    if (!writeSDO_U32(TxPDOmodule, 0x5, 0x301E0310)) return false;  //_I_act        (int16_t) 
    if (!writeSDO_U32(TxPDOmodule, 0x6, 0x603F0010)) return false;  //_LastError    (uint16_t)
    if (!writeSDO_U32(TxPDOmodule, 0x7, 0x30080110)) return false;  //_IO_act       (uint16_t)  
    if (!writeSDO_U32(TxPDOmodule, 0x8, 0x30082610)) return false;  //_IO_STO_act   (uint16_t)
    if (!writeSDO_U8(TxPDOmodule, 0x0, 8)) return false;            //8 parameters

    uint16_t RxPDO = 0x1C12;
    if (!writeSDO_U8(RxPDO, 0x0, 0)) return false;                  //disable Sync Manager
    if (!writeSDO_U16(RxPDO, 0x1, RxPDOmodule)) return false;       //assign pdo module object to sync manager
    if (!writeSDO_U8(RxPDO, 0x0, 1)) return false;                  //1 PDO Module

    uint16_t TxPDO = 0x1C13;
    if (!writeSDO_U8(TxPDO, 0x0, 0)) return false;                  //disable Sync Manager
    if (!writeSDO_U16(TxPDO, 0x1, TxPDOmodule)) return false;       //assign pdo module object to sync manager
    if (!writeSDO_U8(TxPDO, 0x0, 1)) return false;                  //1 PDO Module

    //=========================== TIMING AND SYNC CONFIGURATION ============================

    //set interrupt routine for cyclic synchronous position mode
    //interval should be the same as the frame cycle time, and offset should be zero
    //the frame cycle time is offset 50% from dc_sync time (which is a integer multiple of the interval time)
    //by setting the sync0 event at 0 offset, maximum time offset is garanteed between the sync event and the frame receive time
    uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0;
    uint32_t sync0offset_nanoseconds = sync0Interval_nanoseconds / 2;
    ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);



    //TODO: does this still apply with a lot of slaves ?
    //if propagation delays add up, might the last slaves have their sync event happen at the same time as their frame receive time?

    return true;
}




//==============================================================
//======================= READING INPUTS =======================
//==============================================================

void Lexium32::readInputs() {
    //TxPDO (input data)
    //_DCOMstatus   (uint16_t)  2
    //_DCOMopmd_act (uint8_t)   1
    //_p_act        (int32_t)   4
    //_v_act        (int32_t)   4
    //_tq_act       (int16_t)   2
    //_LastError    (uint16_t)  2
    //_IO_act       (uint16_t)  2

    //TxPDO Data
    uint8_t* inByte = identity->inputs;
    _DCOMstatus =   inByte[0] | inByte[1] << 8; //State Machine Status   (ex: ready to switch on)
    _DCOMopmd_act = inByte[2];                  //Current Operating Mode (ex: cyclic synchronous position)
    _p_act =        inByte[3] | inByte[4] << 8 | inByte[5] << 16 | inByte[6] << 24;
    _v_act =        inByte[7] | inByte[8] << 8 | inByte[9] << 16 | inByte[10] << 24;
    _I_act =        inByte[11] | inByte[12] << 8;
    _LastError =    inByte[13] | inByte[14] << 8;
    _IO_act =       inByte[15] | inByte[16] << 8;
    _IO_STO_act =   inByte[17] | inByte[18] << 8;

    //state machine bits (0,1,2,3,5,6)
    bool readyToSwitchOn =      _DCOMstatus & 0x1;
    bool switchedOn =           _DCOMstatus & 0x2;
    bool operationEnabled =     _DCOMstatus & 0x4;
    bool fault =                _DCOMstatus & 0x8;
    bool quickStop =            _DCOMstatus & 0x20;
    bool switchOnDisabled =     _DCOMstatus & 0x40;

    //Other State Information
    motorVoltagePresent =       _DCOMstatus & 0x10;     //is the voltage for the motor connected
    class0error =               _DCOMstatus & 0x80;     //is there a critical error
    halted =                    _DCOMstatus & 0x100;    //is the motor in a halt state
    fieldbusControlActive =     _DCOMstatus & 0x200;    //is the drive controlled by the fieldbus
    targetReached =             _DCOMstatus & 0x400;    //Operating mode specifig information (b10) 
    internalLimitActive =       _DCOMstatus & 0x800;    //DS402intLim
    operatingModeSpecificFlag = _DCOMstatus & 0x1000;   //varies by operatin mode
    stoppedByError =            _DCOMstatus & 0x2000;   //drive is stopped because of an error
    operatingModeFinished =     _DCOMstatus & 0x4000;   //Operating mode specific information (b12)
    validPositionReference =    _DCOMstatus & 0x8000;   //drive has a valid position reference

    //retrieve the operating mode id
    OperatingMode::Mode previousOperatingMode = actualOperatingMode;
    OperatingMode* operatingMode = getOperatingMode(_DCOMopmd_act);
    if (operatingMode != nullptr) actualOperatingMode = operatingMode->mode;
    else actualOperatingMode = OperatingMode::Mode::UNKNOWN;

    //if we switched operating modes, reset the profile generator and manual velocity command
    if (previousOperatingMode != actualOperatingMode) {
        manualVelocityCommand_rps = 0.0;
        profileVelocity_rps = 0.0;
        velocityCommand->set(0.0);
    }

    State previousState = state;

    //find the state using the state bits
    if (!readyToSwitchOn) {
        if (switchOnDisabled)       state = State::SwitchOnDisabled;
        else if (fault)             state = State::Fault;
        else                        state = State::NotReadyToSwitchOn;
    }
    else {
        if (fault)                  state = State::FaultReactionActive;
        else if (!quickStop)        state = State::QuickStopActive;
        else if (operationEnabled)  state = State::OperationEnabled;
        else if (switchedOn)        state = State::SwitchedOn;
        else                        state = State::ReadyToSwitchOn;
    }

    if (state != previousState) {
        switch (state) {
            case State::SwitchOnDisabled:       pushEvent("StateChange: Switch on Disabled", true); break;
            case State::Fault:                  break;
            case State::NotReadyToSwitchOn:     pushEvent("StateChange: Not Ready To Switch On", true); break;
            case State::FaultReactionActive:    pushEvent("StateChange: Fault Reaction Active", true); break;
            case State::QuickStopActive:        pushEvent("StateChange: QuickStop Active", true); break;
            case State::ReadyToSwitchOn:        pushEvent("StateChange: Ready To Switch On", false); break;
            case State::SwitchedOn:             pushEvent("StateChange: Switched On", false); break;
            case State::OperationEnabled:       pushEvent("StateChange: Operation Enabled", false); break;
        }
    }

    //if an error appeared, push it to the error list
    //if the error is the same as the previous one, don't log it again
    if (_LastError != previousErrorCode && _LastError != 0) {
        pushEvent(_LastError);
        disable();
    }
    previousErrorCode = _LastError;

    //emergency stop state
    bool actualEstop = _IO_STO_act == 0;
    if (actualEstop != b_emergencyStopActive) {
        if (actualEstop) {
            disable();
            pushEvent("Emergency Stop Triggered", true);
        }
        else pushEvent("Emergency Stop Released", false);
        b_emergencyStopActive = actualEstop;
    }

    //set the encoder position in revolution units and velocity in revolutions per second
    encoderDevice->positionRaw_positionUnits = (double)_p_act / (double)positionUnitsPerRevolution;
    encoderDevice->velocity_positionUnitsPerSecond = (double)_v_act / ((double)velocityUnitsPerRpm * 60.0);

    //set motor device load
    motorDevice->load = ((double)_I_act / (double)currentUnitsPerAmp) / maxCurrent_amps;

    //assign public input data
    actualPosition->set(encoderDevice->getPosition());
    actualVelocity->set(encoderDevice->getVelocity());
    actualLoad->set(motorDevice->getLoad());
    digitalIn0->set((_IO_act & 0x1) != 0x0);
    digitalIn1->set((_IO_act & 0x2) != 0x0);
    digitalIn2->set((_IO_act & 0x4) != 0x0);
    digitalIn3->set((_IO_act & 0x8) != 0x0);
    digitalIn4->set((_IO_act & 0x10) != 0x0);
    digitalIn5->set((_IO_act & 0x20) != 0x0);

    //set actuator subdevice
    motorDevice->b_ready = (state == State::ReadyToSwitchOn || state == State::SwitchedOn || state == State::OperationEnabled);
    motorDevice->b_enabled = state == State::OperationEnabled;
    motorDevice->b_online = isOnline() && motorVoltagePresent;
    motorDevice->b_emergencyStopActive = b_emergencyStopActive;
    //set encoder subdevice status
    encoderDevice->b_ready = (state == State::ReadyToSwitchOn || state == State::SwitchedOn || state == State::OperationEnabled);
    encoderDevice->b_online = isOnline();
    //set gpio subdevice status
    gpioDevice->b_ready = (state == State::ReadyToSwitchOn || state == State::SwitchedOn || state == State::OperationEnabled);
    gpioDevice->b_online = isOnline();
}





//==============================================================
//====================== PREPARING OUTPUTS =====================
//==============================================================

void Lexium32::prepareOutputs(){

    //get data from connected nodes
    if (positionCommand->isConnected()) positionCommand->set(positionCommand->getLinks().front()->getInputData()->getReal());
    else positionCommand->set(actualPosition->getReal());
    if (velocityCommand->isConnected()) velocityCommand->set(velocityCommand->getLinks().front()->getInputData()->getReal());
    if (digitalOut0->isConnected()) digitalOut0->set(digitalOut0->getLinks().front()->getInputData()->getBoolean());
    if (digitalOut1->isConnected()) digitalOut1->set(digitalOut1->getLinks().front()->getInputData()->getBoolean());
    if (digitalOut2->isConnected()) digitalOut2->set(digitalOut2->getLinks().front()->getInputData()->getBoolean());

    double now_seconds = EtherCatFieldbus::getReferenceClock_seconds();
    double deltaT_seconds = now_seconds - previousProfilePointTime_seconds;
    previousProfilePointTime_seconds = now_seconds;

    //internal profile generator
    if (actualOperatingMode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY) {

        if (manualVelocityCommand_rps > profileVelocity_rps) {
            double deltaV_rps = manualAcceleration_rpsps * deltaT_seconds;
            profileVelocity_rps += deltaV_rps * 60.0;
            if (profileVelocity_rps > manualVelocityCommand_rps) profileVelocity_rps = manualVelocityCommand_rps;
        }
        else if (manualVelocityCommand_rps < profileVelocity_rps) {
            double deltaV_rps = manualAcceleration_rpsps * deltaT_seconds;
            profileVelocity_rps -= deltaV_rps * 60.0;
            if (profileVelocity_rps < manualVelocityCommand_rps) profileVelocity_rps = manualVelocityCommand_rps;
        }
        profilePosition_r = actualPosition->getReal();

        velocityCommand->set(profileVelocity_rps);
    }
    else if (actualOperatingMode == OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION) {
        //in this operating mode, we verify that the input velocity and acceleration don't exceed internal values

    }
    else {
        //in tuning mode don't use the profile generator
    }


    //handle commands from subdevices
    if (motorDevice->b_setEnabled) {
        motorDevice->b_setEnabled = false;
        b_enableOperation = true;
    }
    if (motorDevice->b_setDisabled) {
        motorDevice->b_setDisabled = false;
        b_disableOperation = true;
    }
    if (motorDevice->b_setQuickstop) {
        motorDevice->b_setQuickstop = false;
        b_quickStop = true;
    }

    //if there is an error present, automatically clear it
    //the fault reset event only happens on a transition from 0 to 1
    //we have to clear the fault reset flag after every activation
    //to be able to use it again
    if (b_faultReset) b_faultReset = false;
    else if (_LastError != 0) b_faultReset = true;

    //handle state transition commands
    if (b_quickStop) {
        b_quickStop = false;
        b_quickStopActive = false;
    }
    if (b_enableOperation) {
        b_enableOperation = false;
        b_operationEnabled = true;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
    }
    if (b_disableOperation) {
        b_disableOperation = false;
        b_operationEnabled = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = false;
    }


    //========== PREPARE RXPDO OUTPUTS ==========
    //DCOMcontrol   (uint16_t)  2
    //DCOMopmode    (int8_t)    1
    //PPp_target    (int32_t)   4
    //PVv_target    (int32_t)   4
    //IO_DQ_set     (uint16_t)  2

    //state control word
    DCOMcontrol = 0x0000;
    if (b_switchedOn)       DCOMcontrol |= 0x1;
    if (b_voltageEnabled)   DCOMcontrol |= 0x2;
    if (b_quickStopActive)  DCOMcontrol |= 0x4;
    if (b_operationEnabled) DCOMcontrol |= 0x8;
    if (opModeSpec4)        DCOMcontrol |= 0x10;
    if (opModeSpec5)        DCOMcontrol |= 0x20;
    if (opModeSpec6)        DCOMcontrol |= 0x40;
    if (b_faultReset)       DCOMcontrol |= 0x80;
    if (b_halted)           DCOMcontrol |= 0x100;
    if (opModeSpec9)        DCOMcontrol |= 0x200;
    //bits 10 to 15 have to be 0

    OperatingMode* operatingMode = getOperatingMode(requestedOperatingMode);
    int operatingModeID = 0;
    if (operatingMode != nullptr) operatingModeID = operatingMode->id;
    DCOMopmode = operatingModeID;

    PPp_target = (int32_t)(positionCommand->getReal() * positionUnitsPerRevolution);
    //don't forget to convert from rotations per second to rpm
    PVv_target = (int32_t)(velocityCommand->getReal() * velocityUnitsPerRpm * 60.0);

    IO_DQ_set = 0;
    if (digitalOut0->getBoolean()) IO_DQ_set |= 0x1;
    if (digitalOut1->getBoolean()) IO_DQ_set |= 0x2;
    if (digitalOut2->getBoolean()) IO_DQ_set |= 0x4;

    //format and copy output data to iomap
    uint8_t* outByte = identity->outputs;
    outByte[0] = (DCOMcontrol >> 0) & 0xFF;
    outByte[1] = (DCOMcontrol >> 8) & 0xFF;
    outByte[2] = DCOMopmode;
    outByte[3] = (PPp_target >> 0) & 0xFF;
    outByte[4] = (PPp_target >> 8) & 0xFF;
    outByte[5] = (PPp_target >> 16) & 0xFF;
    outByte[6] = (PPp_target >> 24) & 0xFF;
    outByte[7] = (PVv_target >> 0) & 0xFF;
    outByte[8] = (PVv_target >> 8) & 0xFF;
    outByte[9] = (PVv_target >> 16) & 0xFF;
    outByte[10] = (PVv_target >> 24) & 0xFF;
    outByte[11] = (IO_DQ_set >> 0) & 0xFF;
    outByte[12] = (IO_DQ_set >> 8) & 0xFF;
}






//==============================================================
//======================= I/O ASSIGNEMENT ======================
//==============================================================


void Lexium32::uploadPinAssignements() {

    pinAssignementUploadState = DataTransferState::State::TRANSFERRING;

    for (auto& inputPin : getInputPins()) {
        if (inputPin.pin == InputPin::Pin::NONE) continue;
        EtherCatCoeData IOfunct_DIx(0x3007, inputPin.CoeSubindex, EtherCatData::Type::UINT16_T);
        if (negativeLimitSwitchPin == inputPin.pin) IOfunct_DIx.setU16(23);
        else if (positiveLimitSwitchPin == inputPin.pin) IOfunct_DIx.setU16(22);
        else IOfunct_DIx.setU16(1);
        if (!IOfunct_DIx.write(getSlaveIndex())) goto transferfailed;
    }

    {
    EtherCatCoeData IOsigLIMN(0x3006, 0xF, EtherCatData::Type::UINT16_T);
    EtherCatCoeData IOsigLIMP(0x3006, 0x10, EtherCatData::Type::UINT16_T);

    if (negativeLimitSwitchPin == InputPin::Pin::NONE) IOsigLIMN.setU16(0);
    else if (b_negativeLimitSwitchNormallyClosed) IOsigLIMN.setU16(1);
    else IOsigLIMN.setU16(2);
    
    if (positiveLimitSwitchPin == InputPin::Pin::NONE) IOsigLIMP.setU16(0);
    else if (b_positiveLimitSwitchNormallyClosed) IOsigLIMP.setU16(1);
    else IOsigLIMP.setU16(2);

    if(!IOsigLIMN.write(getSlaveIndex())) goto transferfailed;
    if(!IOsigLIMP.write(getSlaveIndex())) goto transferfailed;
    }
        
    {
    //set all output pins to freely available by default
    EtherCatCoeData IOfunct_DQ0(0x3007, 0x9, EtherCatData::Type::UINT16_T);
    EtherCatCoeData IOfunct_DQ1(0x3007, 0xA, EtherCatData::Type::UINT16_T);
    EtherCatCoeData IOfunct_DQ2(0x3007, 0xB, EtherCatData::Type::UINT16_T);
    IOfunct_DQ0.setU16(1);
    IOfunct_DQ1.setU16(1);
    IOfunct_DQ2.setU16(1);
    if(!IOfunct_DQ0.write(getSlaveIndex())) goto transferfailed;
    if(!IOfunct_DQ1.write(getSlaveIndex())) goto transferfailed;
    if(!IOfunct_DQ2.write(getSlaveIndex())) goto transferfailed;
    }
        
    pinAssignementUploadState = DataTransferState::State::SAVING;
    if (!saveToEEPROM()) goto transferfailed;
    pinAssignementUploadState = DataTransferState::State::SAVED;
    Logger::warn("Pin Assignement Successfull");
    return;

transferfailed:
    pinAssignementUploadState = DataTransferState::State::FAILED;
    Logger::warn("Pin Assignement Failed");
    return;
}


void Lexium32::downloadPinAssignements() {
    pinAssignementDownloadState = DataTransferState::State::NO_TRANSFER;

    for (auto& inputPin : getInputPins()) {
        if (inputPin.pin == InputPin::Pin::NONE) continue;
        EtherCatCoeData IOfunct_DIx(0x3007, inputPin.CoeSubindex, EtherCatData::Type::UINT16_T);
        if (!IOfunct_DIx.read(getSlaveIndex())) goto transferfailed;
        uint16_t pinFunction = IOfunct_DIx.getU16();
        //for each pin, read the function assignement stored on the drive
        switch (pinFunction) {
            case 23:
                negativeLimitSwitchPin = inputPin.pin;
                break;
            case 22:
                positiveLimitSwitchPin = inputPin.pin;
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
        
    pinAssignementDownloadState = DataTransferState::State::SUCCEEDED;
    return;

transferfailed:
    pinAssignementDownloadState = DataTransferState::State::FAILED;
    return;
}





//==============================================================
//===================== GENERAL PARAMETERS =====================
//==============================================================

void Lexium32::uploadGeneralParameters() {
    generalParameterUploadState = DataTransferState::State::TRANSFERRING;

    {
    EtherCatCoeData CTRL_I_max(0x3011, 0xC, EtherCatData::Type::UINT16_T);
    CTRL_I_max.setU16(maxCurrent_amps * 100.0);
    if (!CTRL_I_max.write(getSlaveIndex())) goto transferfailed;
    }
     
    {
    EtherCatCoeData LIM_I_maxQSTP(0x3011, 0xD, EtherCatData::Type::UINT16_T);
    LIM_I_maxQSTP.setU16(maxQuickstopCurrent_amps * 100.0);
    if (!LIM_I_maxQSTP.write(getSlaveIndex())) goto transferfailed;
    }
     
    {
    EtherCatCoeData InvertDirOfMove(0x3006, 0xC, EtherCatData::Type::UINT16_T);
    InvertDirOfMove.setU16((b_invertDirectionOfMotorMovement ? 1 : 0));
    if (!InvertDirOfMove.write(getSlaveIndex())) goto transferfailed;
    }
        
    generalParameterUploadState = DataTransferState::State::SUCCEEDED;
    Logger::warn("Max Current assignement Successfull");
    return;

transferfailed:
    generalParameterUploadState = DataTransferState::State::FAILED;
    Logger::warn("Max Current Assignement Failed");
    return;
}


void Lexium32::downloadGeneralParameters() {
    generalParameterDownloadState = DataTransferState::State::TRANSFERRING;

    {
    EtherCatCoeData CTRL_I_max(0x3011, 0xC, EtherCatData::Type::UINT16_T);
    if(!CTRL_I_max.read(getSlaveIndex())) goto transferfailed;
    maxCurrent_amps = (float)CTRL_I_max.getU16() / 100.0;
    }
        
    {
    EtherCatCoeData LIM_I_maxQSTP(0x3011, 0xD, EtherCatData::Type::UINT16_T);
    if (!LIM_I_maxQSTP.read(getSlaveIndex())) goto transferfailed;
    maxQuickstopCurrent_amps = (float)LIM_I_maxQSTP.getU16() / 100.0;
    }
        
    {
    EtherCatCoeData InvertDirOfMove(0x3006, 0xC, EtherCatData::Type::UINT16_T);
    if (!InvertDirOfMove.read(getSlaveIndex())) goto transferfailed;
    uint16_t dir = InvertDirOfMove.getU16();
    if (dir == 1) b_invertDirectionOfMotorMovement = true;
    else if (dir == 0) b_invertDirectionOfMotorMovement = false;
    else goto transferfailed;
    }
        
    generalParameterDownloadState = DataTransferState::State::SUCCEEDED;
    return;

transferfailed:
    generalParameterDownloadState = DataTransferState::State::FAILED;
    return;
}






//==============================================================
//====================== ENCODER SETTINGS ======================
//==============================================================

void Lexium32::detectEncoderModule() {
    encoderModuleType = EncoderModule::Type::NONE;
    EtherCatCoeData _ModuleSlot2(0x3002, 0x1A, EtherCatData::Type::UINT16_T);
    if (!_ModuleSlot2.read(getSlaveIndex())) return;
    if (getEncoderModule(_ModuleSlot2.getU16()) == nullptr) {
        encoderModuleType = EncoderModule::Type::NONE;
        return;
    }
    encoderModuleType = getEncoderModule(_ModuleSlot2.getU16())->type;
}

void Lexium32::uploadEncoderSettings() {

    encoderSettingsUploadState = DataTransferState::State::TRANSFERRING;

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

    case EncoderAssignement::Type::INTERNAL_ENCODER:
        {
        ENC_abs_source.setU16(0); //INTERNAL MOTOR ENCODER
        ENC2_type.setU16(0);      //ENCODER 2 TYPE (none)
        ENC2_usage.setU16(0);     //ENCODER 2 USAGE(none)
        if (!ENC_abs_source.write(getSlaveIndex())) goto transferfailed;
        if (!ENC2_type.write(getSlaveIndex())) goto transferfailed;
        if (!ENC2_usage.write(getSlaveIndex())) goto transferfailed;
        }
        break;
            
    case EncoderAssignement::Type::ENCODER_MODULE:
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

        case EncoderModule::Type::DIGITAL_MODULE:
            {
            //digital encoder module setting
            switch (encoderVoltage) { //which power supply the digital encoder uses
            case EncoderVoltage::Voltage::V5: ENCDigPowSupply.setU16(5); break;
            case EncoderVoltage::Voltage::V12: ENCDigPowSupply.setU16(12); break;
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
            case EncoderType::Type::SSI_ROTARY:
                {
                ENC2_type.setU16(10); //encoder type is ssi rotary
                switch (encoderCoding) { //bit encoding of the encoder
                case EncoderCoding::Type::BINARY: ENCDigSSICoding.setU16(0); break;
                case EncoderCoding::Type::GRAY: ENCDigSSICoding.setU16(1); break;
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
        
    encoderSettingsUploadState = DataTransferState::State::SAVING;
    if (!saveToEEPROM()) encoderSettingsUploadState = DataTransferState::State::FAILED;
    encoderSettingsUploadState = DataTransferState::State::SAVED;
    Logger::warn("Encoder assignement success");
    return;

transferfailed:
    encoderSettingsUploadState = DataTransferState::State::FAILED;
    Logger::warn("Transfer Failed");
    return;

}


void Lexium32::downloadEncoderSettings() {

    encoderSettingsDownloadState = DataTransferState::State::TRANSFERRING;
     
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
    if (getEncoderAssignement(ENC_abs_source.getU16()) == nullptr) goto downloadfailed;
    encoderAssignement = getEncoderAssignement(ENC_abs_source.getU16())->type;
    }
    
    switch (encoderAssignement) {
    case EncoderAssignement::Type::INTERNAL_ENCODER:
        //no additionnal settings for internal encoder
        break;
    case EncoderAssignement::Type::ENCODER_MODULE:
        detectEncoderModule();

        {
        if (!ENC2_type.read(getSlaveIndex())) goto downloadfailed;
        else if (getEncoderType(ENC2_type.getU16()) == nullptr) goto downloadfailed;
        encoderType = getEncoderType(ENC2_type.getU16())->type;
        if (!InvertDirOfMaEnc.read(getSlaveIndex())) goto downloadfailed;
        encoder2_invertDirection = InvertDirOfMaEnc.getU16() == 1;
        if (!p_MaxDifToENC2.read(getSlaveIndex())) goto downloadfailed;
        encoder2_maxDifferenceToMotorEncoder_rotations = (float)p_MaxDifToENC2.getS32() / (float)(0x1 << encoder1_singleTurnResolutionBits);
        }
            
        switch (encoderModuleType) {
        case EncoderModule::Type::ANALOG_MODULE:
        case EncoderModule::Type::RESOLVER_MODULE:
        case EncoderModule::Type::NONE: break;
        case EncoderModule::Type::DIGITAL_MODULE:

            {
            if (!ENCDigPowSupply.read(getSlaveIndex())) goto downloadfailed;
            else if (getEncoderVoltage(ENCDigPowSupply.getU16()) == nullptr) goto downloadfailed;
            encoderVoltage = getEncoderVoltage(ENCDigPowSupply.getU16())->voltage;
            }
                
            switch (encoderType) {
            case EncoderType::Type::NONE: break;
            case EncoderType::Type::SSI_ROTARY:
                {
                if (!ENCDigSSICoding.read(getSlaveIndex())) goto downloadfailed;
                else if (getEncoderCoding(ENCDigSSICoding.getU16()) == nullptr) goto downloadfailed;
                encoderCoding = getEncoderCoding(ENCDigSSICoding.getU16())->type;
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
    encoderDevice->rangeMin_positionUnits = lowEncoderRange;
    encoderDevice->rangeMax_positionUnits = highEncoderRange;
        
    encoderSettingsDownloadState = DataTransferState::State::SUCCEEDED;
    return;

downloadfailed:
    encoderSettingsDownloadState = DataTransferState::State::FAILED;
    return;

}


void Lexium32::uploadManualAbsoluteEncoderPosition() {
    encoderAbsolutePositionUploadState = DataTransferState::State::TRANSFERRING;
    int absolutePositionEncoderIncrements;
    EtherCatCoeData ENC1_adjustment(0x3005, 0x16, EtherCatData::Type::INT32_T);
    EtherCatCoeData ENC2_adjustement(0x3005, 0x24, EtherCatData::Type::INT32_T);
    switch (encoderAssignement) {
    case EncoderAssignement::Type::INTERNAL_ENCODER:
        absolutePositionEncoderIncrements = manualAbsoluteEncoderPosition_revolutions * (float)(0x1 << encoder1_singleTurnResolutionBits);
        ENC1_adjustment.setS32(absolutePositionEncoderIncrements);
        if (ENC1_adjustment.write(getSlaveIndex())) goto saving;
        else goto failed;
        break;
    case EncoderAssignement::Type::ENCODER_MODULE:
        absolutePositionEncoderIncrements = manualAbsoluteEncoderPosition_revolutions * (float)(0x1 << encoder2_singleTurnResolutionBits);
        ENC2_adjustement.setS32(absolutePositionEncoderIncrements);
        if (ENC2_adjustement.write(getSlaveIndex())) goto saving;
        else goto failed;
        break;
    }

saving:

    encoderAbsolutePositionUploadState = DataTransferState::State::SAVING;
    if (saveToEEPROM()) encoderAbsolutePositionUploadState = DataTransferState::State::SAVED;
    else goto failed;
    return;

failed:

    encoderAbsolutePositionUploadState = DataTransferState::State::FAILED;
    return;
}



void Lexium32::getEncoderWorkingRange(float& low, float& high) {
    switch (encoderAssignement) {
    case EncoderAssignement::Type::INTERNAL_ENCODER:
        if (b_encoderRangeShifted) {
            low = -(float)(0x1 << encoder1_multiTurnResolutionBits) / 2.0;
            high = (float)(0x1 << encoder1_multiTurnResolutionBits) / 2.0;
        }
        else {
            low = 0.0;
            high = (float)(0x1 << encoder1_multiTurnResolutionBits);
        }
        break;
    case EncoderAssignement::Type::ENCODER_MODULE:
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
        autoTuningSaveState = DataTransferState::State::NO_TRANSFER;

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
            autoTuningSaveState = DataTransferState::State::SAVING;
            Logger::info("Saving Tuning parameters to EEPROM {}", getName());
            if (saveToEEPROM()) {
                autoTuningSaveState = DataTransferState::State::SAVED;
                Logger::info("Saving Succeeded");
            }
            else {
                autoTuningSaveState = DataTransferState::State::FAILED;
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
    factoryResetTransferState = DataTransferState::State::TRANSFERRING;
    EtherCatCoeData PARuserReset(0x3004, 0x8, EtherCatData::Type::UINT16_T);
    PARuserReset.setU16(65535);
    if (PARuserReset.write(getSlaveIndex())) factoryResetTransferState = DataTransferState::State::SUCCEEDED;
    else factoryResetTransferState = DataTransferState::State::FAILED;
}

void Lexium32::setStationAlias(uint16_t a) {
    stationAliasUploadState = DataTransferState::State::TRANSFERRING;
    EtherCatCoeData ECAT2ndaddress(0x3045, 0x6, EtherCatData::Type::UINT16_T);
    ECAT2ndaddress.setU16(a);
    if (ECAT2ndaddress.write(getSlaveIndex())) {
        stationAliasUploadState = DataTransferState::State::SAVING;
        if (saveToEEPROM())  stationAliasUploadState = DataTransferState::State::SAVED;
        else  stationAliasUploadState = DataTransferState::State::FAILED;
    }
    else stationAliasUploadState = DataTransferState::State::FAILED;
}












//==============================================================
//=========================== TYPES ============================
//==============================================================

//============================= SAVING AND LOADING DEVICE DATA ============================

bool Lexium32::saveDeviceData(tinyxml2::XMLElement* xml) { 
    using namespace tinyxml2;

    XMLElement* kinematicLimitsXML = xml->InsertNewChildElement("KinematicLimits");
    kinematicLimitsXML->SetAttribute("velocityLimit_rps", motorDevice->velocityLimit_positionUnitsPerSecond);
    kinematicLimitsXML->SetAttribute("accelerationLimit_rpsps", motorDevice->accelerationLimit_positionUnitsPerSecondSquared);
    kinematicLimitsXML->SetAttribute("defaultManualAcceleration_rpsps", defaultManualAcceleration_rpsps);

    XMLElement* invertDirectionOfMovementXML = xml->InsertNewChildElement("InvertDirectionOfMovement");
    invertDirectionOfMovementXML->SetAttribute("Invert", b_invertDirectionOfMotorMovement);

    XMLElement* currentLimitsXML = xml->InsertNewChildElement("CurrentLimis");
    currentLimitsXML->SetAttribute("maxCurrent_amps", maxCurrent_amps);
    currentLimitsXML->SetAttribute("maxQuickstopCurrent_amps", maxQuickstopCurrent_amps);

    XMLElement* negativeLimitSwitchXML = xml->InsertNewChildElement("NegativeLimitSwitch");
    negativeLimitSwitchXML->SetAttribute("Pin", getInputPin(negativeLimitSwitchPin)->saveName);
    if (negativeLimitSwitchPin != InputPin::Pin::NONE) negativeLimitSwitchXML->SetAttribute("NormallyClosed", b_negativeLimitSwitchNormallyClosed);

    XMLElement* positiveLimitSwitchXML = xml->InsertNewChildElement("PositiveLimitSwitch");
    positiveLimitSwitchXML->SetAttribute("Pin", getInputPin(positiveLimitSwitchPin)->saveName);
    if (positiveLimitSwitchPin != InputPin::Pin::NONE) positiveLimitSwitchXML->SetAttribute("NormallyClosed", b_positiveLimitSwitchNormallyClosed);

    XMLElement* encoderSettingsXML = xml->InsertNewChildElement("EncoderSettings");
    encoderSettingsXML->SetAttribute("Assignement", getEncoderAssignement(encoderAssignement)->saveName);
    switch (encoderAssignement) {
        case EncoderAssignement::Type::INTERNAL_ENCODER:
            break;
        case EncoderAssignement::Type::ENCODER_MODULE:
            encoderSettingsXML->SetAttribute("EncoderModule", getEncoderModule(encoderModuleType)->saveName);
            encoderSettingsXML->SetAttribute("EncoderToMotorRatioNumerator", encoder2_EncoderToMotorRatioNumerator);
            encoderSettingsXML->SetAttribute("EncoderToMotorRatioDenominator", encoder2_EncoderToMotorRatioDenominator);
            encoderSettingsXML->SetAttribute("InvertDirection", encoder2_invertDirection);
            encoderSettingsXML->SetAttribute("MaxDifferenceToMotorEncoder_revolutions", encoder2_maxDifferenceToMotorEncoder_rotations);
            switch (encoderModuleType) {
                case EncoderModule::Type::ANALOG_MODULE: break;
                case EncoderModule::Type::RESOLVER_MODULE: break;
                case EncoderModule::Type::NONE: break;
                case EncoderModule::Type::DIGITAL_MODULE:
                    encoderSettingsXML->SetAttribute("EncoderType", getEncoderType(encoderType)->saveName);
                    encoderSettingsXML->SetAttribute("Voltage", getEncoderVoltage(encoderVoltage)->saveName);
                    encoderSettingsXML->SetAttribute("SingleTurnBits", encoder2_singleTurnResolutionBits);
                    encoderSettingsXML->SetAttribute("MultiTurnBits", encoder2_multiTurnResolutionBits);
                    switch (encoderType) {
                        case EncoderType::Type::NONE: break;
                        case EncoderType::Type::SSI_ROTARY:
                            encoderSettingsXML->SetAttribute("SSIEncoding", getEncoderCoding(encoderCoding)->saveName);
                            break;
                    }
                    break;   
            }
            break;
    }
    encoderSettingsXML->SetAttribute("RangeShifted", b_encoderRangeShifted);

    return true;
}


bool Lexium32::loadDeviceData(tinyxml2::XMLElement* xml) { 
    
    using namespace tinyxml2;

    XMLElement* kinematicLimitsXML = xml->FirstChildElement("KinematicLimits");
    if (kinematicLimitsXML == nullptr) return Logger::warn("Could not find kinematic limits attribute");

    if (kinematicLimitsXML->QueryDoubleAttribute("velocityLimit_rps", &motorDevice->velocityLimit_positionUnitsPerSecond) != XML_SUCCESS) return Logger::warn("Could not read velocity limit attribute");
    if (kinematicLimitsXML->QueryDoubleAttribute("accelerationLimit_rpsps", &motorDevice->accelerationLimit_positionUnitsPerSecondSquared) != XML_SUCCESS) return Logger::warn("Could not read acceleration limit attribute");
    if (kinematicLimitsXML->QueryFloatAttribute("defaultManualAcceleration_rpsps", &defaultManualAcceleration_rpsps) != XML_SUCCESS) return Logger::warn("Could not read acceleration limit attribute");
    manualAcceleration_rpsps = defaultManualAcceleration_rpsps;

    XMLElement* invertDirectionOfMovementXML = xml->FirstChildElement("InvertDirectionOfMovement");
    if (invertDirectionOfMovementXML == nullptr) return Logger::warn("Could not find invert direction of movement attribute");
    if (invertDirectionOfMovementXML->QueryBoolAttribute("Invert", &b_invertDirectionOfMotorMovement) != XML_SUCCESS) return Logger::warn("Could not read direciton of movement attribute");

    XMLElement* currentLimitsXML = xml->FirstChildElement("CurrentLimis");
    if (currentLimitsXML == nullptr) return Logger::warn("Could not find current limits attribute");
    if (currentLimitsXML->QueryDoubleAttribute("maxCurrent_amps", &maxCurrent_amps) != XML_SUCCESS) return Logger::warn("Could not read Max Current Attribute");
    if (currentLimitsXML->QueryDoubleAttribute("maxQuickstopCurrent_amps", &maxQuickstopCurrent_amps) != XML_SUCCESS) return Logger::warn("Could not read max Quickstop Current Attribute");

    XMLElement* negativeLimitSwitchXML = xml->FirstChildElement("NegativeLimitSwitch");
    if (negativeLimitSwitchXML == nullptr) return Logger::warn("Could not find negative limit switch attribute");
    const char* negativeLimitSwitchPinString = "";
    negativeLimitSwitchXML->QueryStringAttribute("Pin", &negativeLimitSwitchPinString);
    if (getInputPin(negativeLimitSwitchPinString) == nullptr) return Logger::warn("Could not read negative limit switch pin attribute");
    negativeLimitSwitchPin = getInputPin(negativeLimitSwitchPinString)->pin;
    if (negativeLimitSwitchPin != InputPin::Pin::NONE) {
        if (negativeLimitSwitchXML->QueryBoolAttribute("NormallyClosed", &b_negativeLimitSwitchNormallyClosed) != XML_SUCCESS) return Logger::warn("Could not read normally closed attribute of negative limit switch");
    }

    XMLElement* positiveLimitSwitchXML = xml->FirstChildElement("PositiveLimitSwitch");
    if (positiveLimitSwitchXML == nullptr) return Logger::warn("Could not find positive limit switch attribute");
    const char* positiveLimitSwitchPinString = "";
    positiveLimitSwitchXML->QueryStringAttribute("Pin", &positiveLimitSwitchPinString);
    if (getInputPin(positiveLimitSwitchPinString) == nullptr) return Logger::warn("Could not read positive limit switch pin attribute");
    positiveLimitSwitchPin = getInputPin(positiveLimitSwitchPinString)->pin;
    if (positiveLimitSwitchPin != InputPin::Pin::NONE) {
        if (positiveLimitSwitchXML->QueryBoolAttribute("NormallyClosed", &b_positiveLimitSwitchNormallyClosed) != XML_SUCCESS) return Logger::warn("Could not read normally closed attribute of positive limit switch");
    }

    XMLElement* encoderSettingsXML = xml->FirstChildElement("EncoderSettings");
    if (encoderSettingsXML == nullptr) return Logger::warn("Could not find Encoder Settings Attribute");
    const char* encoderAssignementString = "";
    encoderSettingsXML->QueryStringAttribute("Assignement", &encoderAssignementString);
    if (getEncoderAssignement(encoderAssignementString) == nullptr) return Logger::warn("Could not read encoder assignement attribute");
    encoderAssignement = getEncoderAssignement(encoderAssignementString)->type;

    switch (encoderAssignement) {
        case EncoderAssignement::Type::INTERNAL_ENCODER:
            break;
        case EncoderAssignement::Type::ENCODER_MODULE:

            const char* encoderModuleString = "";
            encoderSettingsXML->QueryStringAttribute("EncoderModule", &encoderModuleString);
            if (getEncoderModule(encoderModuleString) == nullptr) return Logger::warn("Could not read encoder module attribute");
            encoderModuleType = getEncoderModule(encoderModuleString)->type;

            if (encoderSettingsXML->QueryIntAttribute("EncoderToMotorRatioNumerator", &encoder2_EncoderToMotorRatioNumerator) != XML_SUCCESS) return Logger::warn("Could not read Encoder to Motor Ratio Numerator Attribute");
            if(encoderSettingsXML->QueryIntAttribute("EncoderToMotorRatioDenominator", &encoder2_EncoderToMotorRatioDenominator) != XML_SUCCESS) return Logger::warn("Could not read Encoder to Motor Ratio Denominator Attribute");
            if(encoderSettingsXML->QueryBoolAttribute("InvertDirection", &encoder2_invertDirection) != XML_SUCCESS) return Logger::warn("Could not read Invert Encoder Direction Attribute");
            if(encoderSettingsXML->QueryDoubleAttribute("MaxDifferenceToMotorEncoder_revolutions", &encoder2_maxDifferenceToMotorEncoder_rotations) != XML_SUCCESS) return Logger::warn("Could not read Max Encoder to Motor Encoder Difference Attribute");

            switch (encoderModuleType) {
                case EncoderModule::Type::ANALOG_MODULE: break;
                case EncoderModule::Type::RESOLVER_MODULE: break;
                case EncoderModule::Type::NONE: break;
                case EncoderModule::Type::DIGITAL_MODULE:

                    const char* encoderTypeString = "";
                    encoderSettingsXML->QueryStringAttribute("EncoderType", &encoderTypeString);
                    if (getEncoderType(encoderTypeString) == nullptr) return Logger::warn("Could not read encoder type attribute");
                    encoderType = getEncoderType(encoderTypeString)->type;

                    const char* encoderVoltageString = "";
                    encoderSettingsXML->QueryStringAttribute("Voltage", &encoderVoltageString);
                    if (getEncoderVoltage(encoderVoltageString) == nullptr) return Logger::warn("Could not read encoder voltage attribute");
                    encoderVoltage = getEncoderVoltage(encoderVoltageString)->voltage;

                    if (encoderSettingsXML->QueryIntAttribute("SingleTurnBits", &encoder2_singleTurnResolutionBits) != XML_SUCCESS) return Logger::warn("Could not read encoder Single Turn bit count attribute");
                    if (encoderSettingsXML->QueryIntAttribute("MultiTurnBits", &encoder2_multiTurnResolutionBits) != XML_SUCCESS) return Logger::warn("Could not read encoder Multi Turn bit count attribute");

                    switch (encoderType) {
                        case EncoderType::Type::NONE: break;
                        case EncoderType::Type::SSI_ROTARY:
                            const char* SSIencodingString = "";
                            encoderSettingsXML->QueryStringAttribute("SSIEncoding", &SSIencodingString);
                            if (getEncoderCoding(SSIencodingString) == nullptr) return Logger::warn("Could not read SSI encoding attribute");
                            encoderCoding = getEncoderCoding(SSIencodingString)->type;
                            break;
                    }
                    break;
            }
            break;
    }
    if (encoderSettingsXML->QueryBoolAttribute("RangeShifted", &b_encoderRangeShifted) != XML_SUCCESS) return Logger::warn("Could not find encoder range shift attribute");

    float lowEncoderRange, highEncoderRange;
    getEncoderWorkingRange(lowEncoderRange, highEncoderRange);
    encoderDevice->rangeMin_positionUnits = lowEncoderRange;
    encoderDevice->rangeMax_positionUnits = highEncoderRange;
    
    return true;
}

//============================= DEVICE MODES =================================

std::vector<Lexium32::OperatingMode> Lexium32::operatingModes = {
    {-6, OperatingMode::Mode::TUNING , "Manual/Auto Tuning"},
    {-3, OperatingMode::Mode::MOTION_SEQUENCE , "Motion Sequence"},
    {-2, OperatingMode::Mode::ELECTRONIC_GEAR , "Electronic Gear"},
    {-1, OperatingMode::Mode::JOG , "Jog"},
    {1,  OperatingMode::Mode::PROFILE_POSITION ,"Profile Position"},
    {3,  OperatingMode::Mode::PROFILE_VELOCITY ,"Profile Velocity"},
    {4,  OperatingMode::Mode::PROFILE_TORQUE ,"Profile Torque"},
    {6,  OperatingMode::Mode::HOMING ,"Homing"},
    {7,  OperatingMode::Mode::INTERPOLATED_POSITION ,"Interpolated Position"},
    {8,  OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION ,"Cyclic Synchronous Position"},
    {9,  OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY ,"Cyclic Synchronous Velocity"},
    {10, OperatingMode::Mode::CYCLIC_SYNCHRONOUS_TORQUE , "Cyclic Synchronous Torque"},
    {333, OperatingMode::Mode::UNKNOWN, "Unknown Operating Mode"}
};

std::vector<Lexium32::OperatingMode> Lexium32::availableOperatingModes = {
    {8, OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION, "Position Control"},
    {9, OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY, "Manual Velocity Control"}
};

Lexium32::OperatingMode* Lexium32::getOperatingMode(OperatingMode::Mode opMode) {
    for (OperatingMode& operatingMode : availableOperatingModes) {
        if (opMode == operatingMode.mode) return &operatingMode;
    }
    for (OperatingMode& operatingMode : operatingModes) {
        if (opMode == operatingMode.mode) return &operatingMode;
    }
    return &operatingModes.back();
}

Lexium32::OperatingMode* Lexium32::getOperatingMode(const char* displayName) {
    for (OperatingMode& operatingMode : operatingModes) {
        if (strcmp(displayName, operatingMode.displayName) == 0) return &operatingMode;
    }
    return &operatingModes.back();
}

Lexium32::OperatingMode* Lexium32::getOperatingMode(int id) {
    for (OperatingMode& operatingMode : operatingModes) {
        if (id == operatingMode.id) return &operatingMode;
    }
    return &operatingModes.back();
}

//====================== INPUT PINS ===========================

std::vector<Lexium32::InputPin> Lexium32::inputPins = {
    {Lexium32::InputPin::Pin::DI0, 0x1, "Digital Input 0", "DI0"},
    {Lexium32::InputPin::Pin::DI1, 0x2, "Digital Input 1", "DI1"},
    {Lexium32::InputPin::Pin::DI2, 0x3, "Digital Input 2", "DI2"},
    {Lexium32::InputPin::Pin::DI3, 0x4, "Digital Input 3", "DI3"},
    {Lexium32::InputPin::Pin::DI4, 0x5, "Digital Input 4", "DI4"},
    {Lexium32::InputPin::Pin::DI5, 0x6, "Digital Input 5", "DI5"},
    {Lexium32::InputPin::Pin::NONE, 0x0, "Unassigned", "Unassigned"}
};
std::vector<Lexium32::InputPin>& Lexium32::getInputPins() {
    return inputPins;
}
Lexium32::InputPin* Lexium32::getInputPin(const char* saveName) {
    for (auto& inputPin : inputPins) {
        if (strcmp(saveName, inputPin.saveName) == 0) return &inputPin;
    }
    return nullptr;
}
Lexium32::InputPin* Lexium32::getInputPin(InputPin::Pin pin) {
    for (auto& inputPin : inputPins) {
        if (pin == inputPin.pin) return &inputPin;
    }
    return nullptr;
}
Lexium32::InputPin* Lexium32::getInputPin(uint8_t coeSubindex) {
    for (auto& inputPin : inputPins) {
        if (coeSubindex == inputPin.CoeSubindex) return &inputPin;
    }
    return nullptr;
}

//=========================== ENCODER ASSIGNEMENT ==============================

std::vector<Lexium32::EncoderAssignement> Lexium32::encoderAssignements = {
    {Lexium32::EncoderAssignement::Type::INTERNAL_ENCODER, 0, "Internal Motor Encoder", "Internal"},
    {Lexium32::EncoderAssignement::Type::ENCODER_MODULE, 1, "Encoder Module", "Module"}
};
Lexium32::EncoderAssignement* Lexium32::getEncoderAssignement(const char* saveName) {
    for (EncoderAssignement& assignement : encoderAssignements) {
        if (strcmp(saveName, assignement.saveName) == 0) return &assignement;
    }
    return nullptr;
}
Lexium32::EncoderAssignement* Lexium32::getEncoderAssignement(EncoderAssignement::Type assignementType) {
    for (EncoderAssignement& assignement : encoderAssignements) {
        if (assignementType == assignement.type) return &assignement;
    }
    return nullptr;
}
Lexium32::EncoderAssignement* Lexium32::getEncoderAssignement(uint16_t CoeData) {
    for (EncoderAssignement& assignement : encoderAssignements) {
        if (CoeData == assignement.CoeData) return &assignement;
    }
    return nullptr;
}


std::vector<Lexium32::EncoderModule> Lexium32::encoderModules = {
    {Lexium32::EncoderModule::Type::ANALOG_MODULE, 769, "Analog Encoder Module", "Analog"},
    {Lexium32::EncoderModule::Type::DIGITAL_MODULE, 770, "Digital Encoder Module", "Digital"},
    {Lexium32::EncoderModule::Type::RESOLVER_MODULE, 771, "Resolver Encoder Module", "Resolver"},
    {Lexium32::EncoderModule::Type::NONE, 0, "No Encoder Module", "None"}
};
Lexium32::EncoderModule* Lexium32::getEncoderModule(const char* saveName) {
    for (EncoderModule& module : encoderModules) {
        if (strcmp(saveName, module.saveName) == 0) return &module;
    }
    return nullptr;
}
Lexium32::EncoderModule* Lexium32::getEncoderModule(EncoderModule::Type moduleType) {
    for (EncoderModule& module : encoderModules) {
        if (moduleType == module.type) return &module;
    }
    return nullptr;
}
Lexium32::EncoderModule* Lexium32::getEncoderModule(uint16_t CoeData) {
    for (EncoderModule& module : encoderModules) {
        if (CoeData == module.CoeData) return &module;
    }
    return nullptr;
}



std::vector<Lexium32::EncoderType> Lexium32::encoderTypes = {
    {Lexium32::EncoderType::Type::NONE, 0, "None", "None"},
    {Lexium32::EncoderType::Type::SSI_ROTARY, 10, "SSI Absolute Rotary", "SSIRotary"}
};
Lexium32::EncoderType* Lexium32::getEncoderType(const char* saveName) {
    for (EncoderType& module : encoderTypes) {
        if (strcmp(saveName, module.saveName) == 0) return &module;
    }
    return nullptr;
}
Lexium32::EncoderType* Lexium32::getEncoderType(EncoderType::Type encoderType) {
    for (EncoderType& encoder : encoderTypes) {
        if (encoderType == encoder.type) return &encoder;
    }
    return nullptr;
}
Lexium32::EncoderType* Lexium32::getEncoderType(uint16_t CoeData) {
    for (EncoderType& encoder : encoderTypes) {
        if (CoeData == encoder.CoeData) return &encoder;
    }
    return nullptr;
}



std::vector<Lexium32::EncoderCoding> Lexium32::encoderCodings = {
    {Lexium32::EncoderCoding::Type::BINARY, 0, "Binary", "Binary"},
    {Lexium32::EncoderCoding::Type::GRAY, 1, "Gray", "Gray"}
};
Lexium32::EncoderCoding* Lexium32::getEncoderCoding(const char* saveName) {
    for (EncoderCoding& coding : encoderCodings) {
        if (strcmp(saveName, coding.saveName) == 0) return &coding;
    }
    return nullptr;
}
Lexium32::EncoderCoding* Lexium32::getEncoderCoding(EncoderCoding::Type encodingType) {
    for (EncoderCoding& coding : encoderCodings) {
        if (coding.type == encodingType) return &coding;
    }
    return nullptr;
}
Lexium32::EncoderCoding* Lexium32::getEncoderCoding(uint16_t CoeData) {
    for (EncoderCoding& coding : encoderCodings) {
        if (coding.CoeData == CoeData) return &coding;
    }
    return nullptr;
}


std::vector<Lexium32::EncoderVoltage> Lexium32::encoderVoltages = {
    {Lexium32::EncoderVoltage::Voltage::V5, 5, "5V", "5V"},
    {Lexium32::EncoderVoltage::Voltage::V12, 12, "12V", "12V"}
};
Lexium32::EncoderVoltage* Lexium32::getEncoderVoltage(const char* saveName) {
    for (EncoderVoltage& voltage : encoderVoltages) {
        if (strcmp(saveName, voltage.saveName) == 0) return &voltage;
    }
    return nullptr;
}
Lexium32::EncoderVoltage* Lexium32::getEncoderVoltage(EncoderVoltage::Voltage v) {
    for (EncoderVoltage& voltage : encoderVoltages) {
        if (voltage.voltage == v) return &voltage;
    }
    return nullptr;
}
Lexium32::EncoderVoltage* Lexium32::getEncoderVoltage(uint16_t CoeData) {
    for (EncoderVoltage& voltage : encoderVoltages) {
        if (voltage.CoeData == CoeData) return &voltage;
    }
    return nullptr;
}
