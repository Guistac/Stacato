#include <pch.h>

#include "Lexium32.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

bool Lexium32::hasDeviceError() {
    return lastErrorCode != 0x0;
}

const char* Lexium32::getDeviceErrorString() {
    static char errorString[128];
    sprintf(errorString, "Lexium32 Error Code: %X", lastErrorCode);
    return errorString;
}

void Lexium32::clearDeviceError() {
    b_faultReset = true;
}

bool Lexium32::isDeviceReady() {
    switch (state) {
        case State::OperationEnabled: return true;
        case State::SwitchedOn: return true;
        case State::ReadyToSwitchOn: return true;
        default: return false;
    }
}

void Lexium32::enable() {
    b_enableOperation = true;
}

void Lexium32::disable() {
    b_shutdown = true;
}

bool Lexium32::isEnabled() { 
    switch (state) {
        case State::OperationEnabled: return true;
        default: return false;
    }
}

void Lexium32::assignIoData() {
    motorDevice->setParentDevice(shared_from_this());
    motorLink->set(motorDevice);
    encoderDevice->setParentDevice(shared_from_this());
    encoderLink->set(encoderDevice);
    gpioDevice->setParentDevice(shared_from_this());
    gpioLink->set(gpioDevice);

    //node input data
    addIoData(positionCommand);
    addIoData(velocityCommand);
    addIoData(digitalOut0);
    addIoData(digitalOut1);
    addIoData(digitalOut2);
    
    //node output data
    addIoData(motorLink);
    addIoData(actualTorque);
    addIoData(encoderLink);
    addIoData(actualPosition);
    addIoData(actualVelocity);
    addIoData(gpioLink);
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
    rxPdoAssignement.addEntry(0x6071, 0x0, 2, "PTtq_target", &PTtq_target);
    rxPdoAssignement.addEntry(0x3008, 0x11, 2, "IO_DQ_set", &IO_DQ_set);

    txPdoAssignement.addNewModule(0x1A03);
    txPdoAssignement.addEntry(0x6041, 0x0, 2, "_DCOMstatus", &_DCOMstatus);
    txPdoAssignement.addEntry(0x6061, 0x0, 1, "_DCOMopmd_act", &_DCOMopmd_act);
    txPdoAssignement.addEntry(0x6064, 0x0, 4, "_p_act", &_p_act);
    txPdoAssignement.addEntry(0x606C, 0x0, 4, "_v_act", &_v_act);
    txPdoAssignement.addEntry(0x6077, 0x0, 2, "_tq_act", &_tq_act);
    txPdoAssignement.addEntry(0x603F, 0x0, 2, "_LastError", &_LastError);
    txPdoAssignement.addEntry(0x3008, 0x1, 2, "_IO_act", &_IO_act);
}


bool Lexium32::startupConfiguration() {

    Logger::info("===== Begin Lexium32 Startup Configuration");

    Logger::debug("    = Begin Setting Startup Parameters...");
    if (!setStartupParameters()) {
        Logger::error("    = Failed to set startup Parameters...");
        return false;
    }
    Logger::debug("    = Successfully set startup parameters");

    Logger::debug("    = Begin Lexium32 PDO assignement...");
    if (!assignPDOs()) {
        Logger::error("    = Lexium32 PDO assignement failed...");
        return false;
    }
    Logger::debug("    = PDO assignement successfull !");

    //set interrupt routine for cyclic synchronous position mode
    //interval should be the same as the frame cycle time, and offset should be zero
    //the frame cycle time is offset 50% from dc_sync time (which is a integer multiple of the interval time)
    //by setting the sync0 event at 0 offset, maximum time offset is garanteed between the sync event and the frame receive time
    uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0L;
    uint32_t sync0offset_nanoseconds = 0;
    ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);
    //TODO: does this still apply with a lot of slaves ?
    //if propagation delays add up, might the last slaves have their sync event happen at the same time as their frame receive time?

    Logger::info("===== Finished Lexium32 Startup Configuration");

    return true;
}

bool Lexium32::setStartupParameters() {

    //ramp settingts (useless in cyclic synchronous position mode for some reason...)
    uint32_t RAMP_v_acc_set = 600;
    if (!writeSDO(0x6083, 0x0, RAMP_v_acc_set)) return false;
    uint32_t RAMP_v_dec_set = 600;
    if (!writeSDO(0x6084, 0x0, RAMP_v_dec_set)) return false;


    uint16_t _ModuleSlot2;
    if (!readSDO(0x3002, 0x1A, _ModuleSlot2)) return false;
    Logger::warn("Module in slot 2: {}", _ModuleSlot2);

//#define SET_SSI_PARAM
#ifdef SET_SSI_PARAM
    uint16_t ENC_abs_source = 1;        //encoder source: encoder module (encoder 2)
    uint16_t ENC2_type = 10;            //encoder type: SSI Rotary
    uint16_t ENC2_usage = 2;            //encoder usage: Motor Encoder
    uint16_t ENCDigPowSupply = 12;      //12V power supply
    uint16_t ENCDigResMulUsed = 0;      //use all multiturn bits
    uint16_t ENCDigSSICoding = 1;       //gray encoding
    uint16_t ENCDigSSIMaxFreq = 200;    //200Khz SSI
    uint16_t ENCDigSSIResMult = 12;     //12 bits multiturn
    uint16_t ENCDigSSIResSgl = 17;      //17 bits singleturn
    uint32_t ResolENC2 = 131072;        //raw resolution of the external encoder
    int32_t ResolENC2Num = 131072;      //encoder increments per
    int32_t ResolENC2Denom = 1;         //per motor shaft revolution

    if (!writeSDO(0x3005, 0x25, ENC_abs_source)) return false;
    if (!writeSDO(0x3050, 0x3, ENC2_type)) return false;
    if (!writeSDO(0x3050, 0x1, ENC2_usage)) return false;
    if (!writeSDO(0x3052, 0x4, ENCDigPowSupply)) return false;
    if (!writeSDO(0x3052, 0xB, ENCDigResMulUsed)) return false;
    if (!writeSDO(0x3052, 0x3, ENCDigSSICoding)) return false;
    if (!writeSDO(0x3052, 0x5, ENCDigSSIMaxFreq)) return false;
    if (!writeSDO(0x3052, 0x2, ENCDigSSIResMult)) return false;
    if (!writeSDO(0x3052, 0x1, ENCDigSSIResSgl)) return false;

    uint16_t PAReeprSave = 1;
    if (!writeSDO(0x3004, 0x1, PAReeprSave)) return false;

    //should these parameters also be saved to eeprom ?
    if (!writeSDO(0x3050, 0xF, ResolENC2)) return false;
    if (!writeSDO(0x3050, 0x5, ResolENC2Denom)) return false;
    if (!writeSDO(0x3050, 0x6, ResolENC2Num)) return false;

    if (!readSDO(0x3005, 0x25, ENC_abs_source)) return false;
    if (!readSDO(0x3050, 0x3, ENC2_type)) return false;
    if (!readSDO(0x3050, 0x1, ENC2_usage)) return false;
    if (!readSDO(0x3052, 0x4, ENCDigPowSupply)) return false;
    if (!readSDO(0x3052, 0xB, ENCDigResMulUsed)) return false;
    if (!readSDO(0x3052, 0x3, ENCDigSSICoding)) return false;
    if (!readSDO(0x3052, 0x5, ENCDigSSIMaxFreq)) return false;
    if (!readSDO(0x3052, 0x2, ENCDigSSIResMult)) return false;
    if (!readSDO(0x3052, 0x1, ENCDigSSIResSgl)) return false;
    if (!readSDO(0x3050, 0xF, ResolENC2)) return false;
    if (!readSDO(0x3050, 0x5, ResolENC2Denom)) return false;
    if (!readSDO(0x3050, 0x6, ResolENC2Num)) return false;
    Logger::warn("ENC_abs_source: {}", ENC_abs_source);
    Logger::warn("ENC2_type: {}", ENC2_type);
    Logger::warn("ENC2_usage: {}", ENC2_usage);
    Logger::warn("ENCDigPowSupply: {}", ENCDigPowSupply);
    Logger::warn("ENCDigResMulUsed: {}", ENCDigResMulUsed);
    Logger::warn("ENCDigSSICoding: {}", ENCDigSSICoding);
    Logger::warn("ENCDigSSIMaxFreq: {}", ENCDigSSIMaxFreq);
    Logger::warn("ENCDigSSIResMult: {}", ENCDigSSIResMult);
    Logger::warn("ENCDigSSIResSgl: {}", ENCDigSSIResSgl);
    Logger::warn("ResolENC2: {}", ResolENC2);
    Logger::warn("Encoder 2 Resolution= {}  ratio : {} encoder increments / {} motor revolutions", ResolENC2, ResolENC2Num, ResolENC2Denom);
#endif
    
//#define SET_NO_ENCODER_PARAMS
#ifdef SET_NO_ENCODER_PARAMS
    uint16_t ENC_abs_source = 0;    //encoder source: interal encoder (encoder 1)
    uint16_t ENC2_type = 0;         //encoder type: None
    uint16_t ENC2_usage = 0;        //encoder usage: Motor Encoder
    if (!writeSDO(0x3005, 0x25, ENC_abs_source)) return false;
    if (!writeSDO(0x3050, 0x3, ENC2_type)) return false;
    if (!writeSDO(0x3050, 0x1, ENC2_usage)) return false;
    uint16_t PAReeprSave = 1;
    if (!writeSDO(0x3004, 0x1, PAReeprSave)) return false;
#endif



    //Cia DS402 mandatory Startup Settings (According to Lexium32 EtherCAT module documentation)
    uint16_t CompParSyncMot_set = 1;
    if (!writeSDO(0x3006, 0x3D, CompParSyncMot_set)) return false;
    uint16_t MOD_enable_set = 0;
    if (!writeSDO(0x3006, 0x38, MOD_enable_set)) return false;
    int16_t LIM_QStopReact_set = -1;
    if (!writeSDO(0x3006, 0x18, LIM_QStopReact_set)) return false;
    uint16_t IOsigRespOfPS_set = 1;
    if (!writeSDO(0x3006, 0x6, IOsigRespOfPS_set)) return false;
    int32_t ScalePOSdenom_set = 131072;
    if (!writeSDO(0x3006, 0x7, ScalePOSdenom_set)) return false;
    int32_t ScalePOSnum_set = 1;
    if (!writeSDO(0x3006, 0x8, ScalePOSnum_set)) return false;
    uint16_t CTRL1_KFPp_set = 1000;
    if (!writeSDO(0x3012, 0x6, CTRL1_KFPp_set)) return false;
    uint16_t CTRL2_KFPp_set = 1000;
    if (!writeSDO(0x3013, 0x6, CTRL2_KFPp_set)) return false;
    int8_t DCOMopmode_set = 8;
    if (!writeSDO(0x6060, 0x0, DCOMopmode_set)) return false;
    uint32_t ECATinpshifttime_set = 250000;
    if (!writeSDO(0x1C33, 0x3, ECATinpshifttime_set)) return false;

    return true;
}

bool Lexium32::assignPDOs() {

    //variables applied to the zero subindex of an object dictionnary entry
    uint8_t zero = 0x00;
    uint8_t one = 0x01;

    //Sync Manager (SM2, SM3) registers that store the mapping objects (modules) which decribe PDO data
    uint16_t RxPDO = 0x1C12;
    uint16_t TxPDO = 0x1C13;

    //mapping object (module) to be stored in each pdo register
    uint16_t RxPDOmodule = 0x1603;
    uint16_t TxPDOmodule = 0x1A03;

    //turn the pdo off by writing a zero to the 0 index, set the mapping object at subindex 1, enable the pdo by writing a 1 (module count) to the index
    if (!writeSDO(RxPDO, 0x0, zero)) return false;
    if (!writeSDO(RxPDO, 0x1, RxPDOmodule)) return false;
    if (!writeSDO(RxPDO, 0x0, one)) return false;

    //do the same for the TxPDO but modify one parameter
    if (!writeSDO(TxPDO, 0x0, zero)) return false;

    //we are going to edit one parameter entry of the PDO module
    //disable TxPDO module before modifying it (set entry count to zero)
    if (!writeSDO(TxPDOmodule, 0x0, zero)) return false;

    //replace default parameter 4 (_p_dif) by current velocity (_v_act 0x606C 0x00 int32_t)
    uint32_t TxPDOparameter4 = 0x606C0020;
    if (!writeSDO(TxPDOmodule, 0x4, TxPDOparameter4)) return false;

    //actual position of encoder (encoder 1 or 2 depends on assignement)
    //uint32_t TxPDOParameter3 = 0x301E0F20;
    //uint32_t TxPDOParameter3 = 0x301E1A20; //module encoder
    //uint32_t TxPDOParameter3 = 0x301E1920;  //module encode internal units
    //uint32_t TxPDOParameter3 = 0x301E2720;  //internal encoder 
    //if (!writeSDO(TxPDOmodule, 0x3, TxPDOParameter3)) return false;

    //update parameter count at subindex 0 of pdo object
    uint8_t TxPDOparameterCount = 7;
    if (!writeSDO(TxPDOmodule, 0x0, TxPDOparameterCount)) return false;

    //assign pdo module object to sync manager
    if (!writeSDO(TxPDO, 0x1, TxPDOmodule)) return false;
    if (!writeSDO(TxPDO, 0x0, one)) return false;

    return true;
}

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
    _DCOMstatus = inByte[0] | inByte[1] << 8; //State Machine Status   (ex: ready to switch on)
    _DCOMopmd_act = inByte[2];                  //Current Operating Mode (ex: cyclic synchronous position)
    _p_act = inByte[3] | inByte[4] << 8 | inByte[5] << 16 | inByte[6] << 24;
    _v_act = inByte[7] | inByte[8] << 8 | inByte[9] << 16 | inByte[10] << 24;
    _tq_act = inByte[11] | inByte[12] << 8;
    _LastError = inByte[13] | inByte[14] << 8;
    _IO_act = inByte[15] | inByte[16] << 8;

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
     modeID = _DCOMopmd_act;

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

    lastErrorCode = _LastError;

    //assign public input data
    //actualPosition = _p_act;
    actualPosition->set((double)_p_act / 131072.0);
    actualVelocity->set((double)_v_act);
    actualTorque->set((double)_tq_act);
    digitalIn0->set((_IO_act & 0x1) != 0x0);
    digitalIn1->set((_IO_act & 0x2) != 0x0);
    digitalIn2->set((_IO_act & 0x4) != 0x0);
    digitalIn3->set((_IO_act & 0x8) != 0x0);
    digitalIn4->set((_IO_act & 0x10) != 0x0);
    digitalIn5->set((_IO_act & 0x20) != 0x0);



    //set subdevice status

    motorDevice->b_ready = (state == State::ReadyToSwitchOn || state == State::SwitchedOn || state == State::OperationEnabled);
    motorDevice->b_enabled = state == State::OperationEnabled;

    encoderDevice->b_ready = (state == State::ReadyToSwitchOn || state == State::SwitchedOn || state == State::OperationEnabled);
    //encoderDevice->b_inRange = TODO: detect out of range encoder values

    gpioDevice->b_ready = (state == State::ReadyToSwitchOn || state == State::SwitchedOn || state == State::OperationEnabled);

}

void Lexium32::process() {
    
    //TODO: process method seems useless for this kind of device / node

}


void Lexium32::prepareOutputs(){

    //handle commands from subdevices

    if (motorDevice->b_clearError) {
        motorDevice->b_clearError = false;
        b_faultReset = true;
    }
    if (motorDevice->b_setEnabled) {
        motorDevice->b_setEnabled = false;
        b_enableOperation = true;
    }
    if (motorDevice->b_setDisabled) {
        motorDevice->b_setDisabled = false;
        b_shutdown = true;
    }

    if (encoderDevice->b_clearError) {
        encoderDevice->b_clearError = false;
        b_faultReset = true;
    }
    if (encoderDevice->b_reset) {
        encoderDevice->b_reset = false;
        //TODO: reset encoder from here ??
    }

    if (gpioDevice->b_clearError) {
        gpioDevice->b_clearError = false;
        b_faultReset = true;
    }


    //handle state transition commands

    if (b_disableVoltage) {
        b_disableVoltage = false;
        b_voltageEnabled = false;
    }
    if (b_enableVoltage) {
        b_enableVoltage = false;
        b_voltageEnabled = true;
    }
    if (b_shutdown) {
        b_shutdown = false;
        b_switchedOn = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
    }
    if (b_switchOn) {
        b_switchOn = false;
        b_operationEnabled = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
    }
    if (b_quickStop) {
        b_quickStop = false;
        b_quickStopActive = false;
        b_voltageEnabled = true;
    }
    if (b_faultReset) {
        b_faultReset = false;
        b_faultResetState = true; //reset this to zero after execution !
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
        b_switchedOn = true;
    }

    if (positionCommand->isConnected()) positionCommand->set(positionCommand->getLinks().front()->getInputData()->getReal());
    if (velocityCommand->isConnected()) velocityCommand->set(velocityCommand->getLinks().front()->getInputData()->getReal());
    if (digitalOut0->isConnected()) digitalOut0->set(digitalOut0->getLinks().front()->getInputData()->getBoolean());
    if (digitalOut1->isConnected()) digitalOut1->set(digitalOut1->getLinks().front()->getInputData()->getBoolean());
    if (digitalOut2->isConnected()) digitalOut2->set(digitalOut2->getLinks().front()->getInputData()->getBoolean());


    //RxPDO (outputs)
    //DCOMcontrol   (uint16_t)  2
    //DCOMopmode    (int8_t)    1
    //PPp_target    (int32_t)   4
    //PVv_target    (int32_t)   4
    //PTtq_target   (int32_t)   2
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
    if (b_faultResetState)  DCOMcontrol |= 0x80;
    if (b_halted)           DCOMcontrol |= 0x100;
    if (opModeSpec9)        DCOMcontrol |= 0x200;
    //bits 10 to 15 have to be 0
    b_faultResetState = false; //always reset bit after performing a fault reset

    DCOMopmode = modeIDCommand;

    PPp_target = (int32_t)(positionCommand->getReal() * 131072.0L);
    PVv_target = velocityCommand->getReal();

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
    outByte[11] = (PTtq_target >> 0) & 0xFF;
    outByte[12] = (PTtq_target >> 8) & 0xFF;
    outByte[13] = (IO_DQ_set >> 0) & 0xFF;
    outByte[14] = (IO_DQ_set >> 8) & 0xFF;
}




bool Lexium32::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool Lexium32::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }


/*
useful modes:
Cyclic Synchronous Position
Cyclic Synchronous Velocity
Manual/Auto Tuning

*/

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
};

std::vector<Lexium32::OperatingMode> Lexium32::availableOperatingModes = {
    {8, OperatingMode::Mode::CYCLIC_SYNCHRONOUS_POSITION, "Position Control"},
    {9, OperatingMode::Mode::CYCLIC_SYNCHRONOUS_VELOCITY,"Manual Velocity Control"},
    {-6,OperatingMode::Mode::TUNING, "Tuning"}
};

Lexium32::OperatingMode* Lexium32::getOperatingMode() {
    for (OperatingMode& availableOperatingMode : availableOperatingModes) {
        if (modeID == availableOperatingMode.id) return &availableOperatingMode;
    }
    for (OperatingMode& operatingMode : operatingModes) {
        if (modeID == operatingMode.id) return &operatingMode;
    }
    return nullptr;
}