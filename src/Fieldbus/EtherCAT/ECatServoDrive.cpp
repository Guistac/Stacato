#include "ECatServoDrive.h"
#include "EtherCatFieldbus.h"

#include <iostream>
#include <bitset>
#include <ethercat.h>

std::map<int, std::string> ECatServoDrive::modelist = {
    {-6, "Manual/Auto Tuning"},
    {-5, "-5"},
    {-4, "-4"},
    {-3, "Motion Sequence"},
    {-2, "Electronic Gear"},
    {-1, "Jog"},
    {0, "0"},
    {1, "Profile Position"},
    {2, "2"},
    {3, "Profile Velocity"},
    {4, "Profile Torque"},
    {5, "5"},
    {6, "Homing"},
    {7, "Interpolated Position"},
    {8, "Cyclic Synchronous Position"},
    {9, "Cyclic Synchronous Velocity"},
    {10, "Cyclic Synchronous Torque"},
    {11, "11"}
};

void ECatServoDrive::process() {

    static int counter = 0;
    positionCommand += std::sin((double)counter / 100.0) * 1000;
    counter++;


    //============== READ INPUTS

    //TxPDO (inputs)
    //_DCOMstatus   (uint16_t)  2
    //_DCOMopmd_act (int8_t)    1
    //_p_act        (int32_t)   4
    //_p_dif        (int32_t)   4
    //_tq_act       (int32_t)   2
    //_LastError    (uint16_t)  2
    //_IO_act       (uint16_t)  2

    uint8_t* inByte = identity.slave_ptr->inputs;

    //TxPDO Data
    uint16_t _DCOMstatus =  inByte[0] | inByte[1] << 8;
    int8_t _DCOMopmd_act = inByte[2];
    position =      inByte[3] | inByte[4] << 8 | inByte[5] << 16 | inByte[6] << 24;
    positionError = inByte[7] | inByte[8] << 8 | inByte[9] << 16 | inByte[10] << 24;
    torque =        inByte[11] | inByte[12] << 8;
    lastErrorCode = inByte[13] | inByte[14] << 8;
    uint16_t _IO_act = inByte[15] | inByte[16] << 8;

    DI0 = _IO_act & 0x1;
    DI1 = _IO_act & 0x2;
    DI2 = _IO_act & 0x4;
    DI3 = _IO_act & 0x8;
    DI4 = _IO_act & 0x10;
    DI5 = _IO_act & 0x20;

    //state bits (0,1,2,3,5,6)
    bool readyToSwitchOn =  _DCOMstatus & 0x1;
    bool switchedOn =       _DCOMstatus & 0x2;
    bool operationEnabled = _DCOMstatus & 0x4;
    bool fault =            _DCOMstatus & 0x8;
    bool quickStop =        _DCOMstatus & 0x20;
    bool switchOnDisabled = _DCOMstatus & 0x40;
    //other DCOM bits
    motorVoltagePresent =       _DCOMstatus & 0x10;
    class0error =               _DCOMstatus & 0x80;
    halted =                    _DCOMstatus & 0x100;
    fieldbusControlActive =     _DCOMstatus & 0x200;
    targetReached =             _DCOMstatus & 0x400;  //Operating mode specifig information (b10) 
    internalLimitActive =       _DCOMstatus & 0x800; //DS402intLim
    operatingModeSpecificFlag = _DCOMstatus & 0x1000;
    stoppedByError =            _DCOMstatus & 0x2000;
    operatingModeFinished =     _DCOMstatus & 0x4000; //Operating mode specific information (b12)
    validPositionReference =    _DCOMstatus & 0x8000;

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

    if (_DCOMopmd_act != mode) {
        mode = _DCOMopmd_act;
        modeChar = modelist[mode].c_str();
    }

    //============== WRITE OUTPUTS

    //RxPDO (outputs)
    //DCOMcontrol   (uint16_t)  2
    //DCOMopmode    (int8_t)    1
    //PPp_target    (int32_t)   4
    //PVv_target    (int32_t)   4
    //PTtq_target   (int32_t)   2
    //IO_DQ_set     (uint16_t)  2

    uint8_t* outByte = identity.slave_ptr->outputs;

    static bool faultResetOnPreviousCycle = false;

    //Drive State Commands
    if (disableVoltage) {
        disableVoltage = false;
        b_voltageEnabled = false;
        b_faultResetState = false;
    }
    if (enableVoltage) {
        enableVoltage = false;
        b_voltageEnabled = true;
        b_faultResetState = false;
    }
    if (shutdown) {
        shutdown = false;
        b_switchedOn = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_faultResetState = false;
    }
    if (switchOn) {
        switchOn = false;
        b_operationEnabled = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
        b_faultResetState = false;
    }
    if (performQuickStop) {
        performQuickStop = false;
        b_quickStopActive = false;
        b_voltageEnabled = true;
        b_faultResetState = false;
    }
    if (performFaultReset && !faultResetOnPreviousCycle) {
        performFaultReset = false;
        b_faultResetState = true; //reset this to zero after execution !
        faultResetOnPreviousCycle = true;
    }
    if (enableOperation) {
        enableOperation = false;
        b_operationEnabled = true;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
        b_faultResetState = false;
    }
    if (disableOperation) {
        disableOperation = false;
        b_operationEnabled = false;
        b_quickStopActive = true;
        b_voltageEnabled = true;
        b_switchedOn = true;
        b_faultResetState = false;
    }

    //state control word
    uint16_t DCOMcontrol = 0x0000;
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

    if (!b_faultResetState) faultResetOnPreviousCycle = false; //this prevents two fault reset bits to be sent after one another
    b_faultResetState = false; //reset bit after performing a fault reset

    int8_t DCOMopmode = modeCommand;

    uint16_t IO_DQ_set = 0;
    if (DQ0) IO_DQ_set |= 0x1;
    if (DQ1) IO_DQ_set |= 0x2;
    if (DQ2) IO_DQ_set |= 0x4;

    outByte[0] = (DCOMcontrol >> 0) & 0xFF;
    outByte[1] = (DCOMcontrol >> 8) & 0xFF;
    
    outByte[2] = DCOMopmode;

    outByte[3] = (positionCommand >> 0) & 0xFF;
    outByte[4] = (positionCommand >> 8) & 0xFF;
    outByte[5] = (positionCommand >> 16) & 0xFF;
    outByte[6] = (positionCommand >> 24) & 0xFF;

    outByte[7] = (velocityCommand >> 0) & 0xFF;
    outByte[8] = (velocityCommand >> 8) & 0xFF;
    outByte[9] = (velocityCommand >> 16) & 0xFF;
    outByte[10] = (velocityCommand >> 24) & 0xFF;

    outByte[11] = (torqueCommand >> 0) & 0xFF;
    outByte[12] = (torqueCommand >> 8) & 0xFF;

    outByte[13] = (IO_DQ_set >> 0) & 0xFF;
    outByte[14] = (IO_DQ_set >> 8) & 0xFF;

    //============== OTHER COMMANDS

    if (jog) {
        uint16_t command = 0;
        if (direction) command |= 0x1;
        else command |= 0x2;
        if (fast) command |= 0x4;
        int wc = ec_SDOwrite(identity.index, 0x301B, 0x9, false, 2, &command, EC_TIMEOUTSAFE);
        direction = false;
        fast = false;
        jog = false;
    }
    if (stop) {
        uint16_t command = 0;
        int wc = ec_SDOwrite(identity.index, 0x301B, 0x9, false, 2, &command, EC_TIMEOUTSAFE);
        stop = false;
    }

}

void ECatServoDrive::writeStartupParameters() {
    int size;
    int wc;
    int i = identity.index;

    std::cout << "-----------WRITE-----------" << std::endl;

    size = 2;
    uint16_t CompParSyncMot_set = 1;
    wc = ec_SDOwrite(i, 0x3006, 0x3D, false, size, &CompParSyncMot_set, EC_TIMEOUTSAFE);
    std::cout << "CompParSyncMot_set: " << wc << std::endl;

    size = 2;
    uint16_t MOD_enable_set = 0;
    wc = ec_SDOwrite(i, 0x3006, 0x38, false, size, &MOD_enable_set, EC_TIMEOUTSAFE);
    std::cout << "MOD_enable_set: " << wc << std::endl;

    size = 2;
    int16_t LIM_QStopReact_set = -1;
    wc = ec_SDOwrite(i, 0x3006, 0x18, false, size, &LIM_QStopReact_set, EC_TIMEOUTSAFE);
    std::cout << "LIM_QStopReact_set: " << wc << std::endl;

    size = 2;
    uint16_t IOsigRespOfPS_set = 1;
    wc = ec_SDOwrite(i, 0x3006, 0x6, false, size, &IOsigRespOfPS_set, EC_TIMEOUTSAFE);
    std::cout << "IOsigRespOfPS_set: " << wc << std::endl;

    size = 4;
    int32_t ScalePOSdenom_set = 131072;
    wc = ec_SDOwrite(i, 0x3006, 0x7, false, size, &ScalePOSdenom_set, EC_TIMEOUTSAFE);
    std::cout << "ScalePOSdenom_set: " << wc << std::endl;

    size = 4;
    int32_t ScalePOSnum_set = 1;
    wc = ec_SDOwrite(i, 0x3006, 0x8, false, size, &ScalePOSnum_set, EC_TIMEOUTSAFE);
    std::cout << "ScalePOSnum_set: " << wc << std::endl;

    size = 2;
    uint16_t CTRL1_KFPp_set = 1000;
    wc = ec_SDOwrite(i, 0x3012, 0x6, false, size, &CTRL1_KFPp_set, EC_TIMEOUTSAFE);
    std::cout << "CTRL1_KFPp_set: " << wc << std::endl;

    size = 2;
    uint16_t CTRL2_KFPp_set = 1000;
    wc = ec_SDOwrite(i, 0x3013, 0x6, false, size, &CTRL2_KFPp_set, EC_TIMEOUTSAFE);
    std::cout << "CTRL2_KFPp_set: " << wc << std::endl;

    size = 1;
    int8_t DCOMopmode_set = 8;
    wc = ec_SDOwrite(i, 0x6060, 0x0, false, size, &DCOMopmode_set, EC_TIMEOUTSAFE);
    std::cout << "DCOMopmode_set: " << wc << std::endl;

    size = 4;
    //uint32_t ECATinpshifttime_set = 250000;
    uint32_t ECATinpshifttime_set = 5000000;
    wc = ec_SDOwrite(i, 0x1C33, 0x3, false, size, &ECATinpshifttime_set, EC_TIMEOUTSAFE);
    std::cout << "ECATinpshifttime_set: " << wc << std::endl;
}

void ECatServoDrive::readStartupParameters() {
    int size;
    int wc;
    int i = identity.index;

    std::cout << "-----------READ-----------" << std::endl;

    size = 2;
    uint16_t CompParSyncMot;
    wc = ec_SDOread(i, 0x3006, 0x3D, false, &size, &CompParSyncMot, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " compParSyncMot: " << (int)CompParSyncMot << " size: " << size << std::endl;

    size = 2;
    uint16_t MOD_enable;
    wc = ec_SDOread(i, 0x3006, 0x38, false, &size, &MOD_enable, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " MOD_enable: " << (int)MOD_enable << " size: " << size << std::endl;

    size = 2;
    int16_t LIM_QStopReact;
    wc = ec_SDOread(i, 0x3006, 0x18, false, &size, &LIM_QStopReact, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " LIM_QStopReact: " << (int)LIM_QStopReact << " size: " << size << std::endl;

    size = 2;
    uint16_t IOsigRespOfPS;
    wc = ec_SDOread(i, 0x3006, 0x6, false, &size, &IOsigRespOfPS, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " IOsigRespOfPS: " << (int)IOsigRespOfPS << " size: " << size << std::endl;

    size = 4;
    int32_t ScalePOSdenom;
    wc = ec_SDOread(i, 0x3006, 0x7, false, &size, &ScalePOSdenom, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " ScalePOSdenom: " << (int)ScalePOSdenom << " size: " << size << std::endl;

    size = 4;
    int32_t ScalePOSnum;
    wc = ec_SDOread(i, 0x3006, 0x8, false, &size, &ScalePOSnum, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " ScalePOSnum: " << (int)ScalePOSnum << " size: " << size << std::endl;

    size = 2;
    uint16_t CTRL1_KFPp;
    wc = ec_SDOread(i, 0x3012, 0x6, false, &size, &CTRL1_KFPp, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " CTRL1_KFPp: " << (int)CTRL1_KFPp << " size: " << size << std::endl;

    size = 2;
    uint16_t CTRL2_KFPp;
    wc = ec_SDOread(i, 0x3013, 0x6, false, &size, &CTRL2_KFPp, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " CTRL2_KFPp: " << (int)CTRL2_KFPp << " size: " << size << std::endl;

    size = 1;
    int8_t DCOMopmode;
    wc = ec_SDOread(i, 0x6060, 0x0, false, &size, &DCOMopmode, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " DCOMopmode: " << (int)DCOMopmode << " size: " << size << std::endl;

    size = 4;
    uint32_t ECATinpshifttime;
    wc = ec_SDOread(i, 0x1C33, 0x3, false, &size, &ECATinpshifttime, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " ECATinpshifttime: " << ECATinpshifttime << " size: " << size << std::endl;

    size = 4;
    int32_t _p_act;
    wc = ec_SDOread(i, 0x6064, 0x0, false, &size, &_p_act, EC_TIMEOUTSAFE);
    std::cout << "wc: " << wc << " _p_act: " << _p_act << " size: " << size << std::endl;
    positionCommand = _p_act;
}

void ECatServoDrive::configurePDOs() {


    ec_dcsync0(identity.index, true, EtherCatFieldbus::processInterval_microseconds*1000, EtherCatFieldbus::processInterval_microseconds * 1000 / 2);

    if (strcmp(identity.name, "LXM32M EtherCAT") == 0) {
        /*
        std::cout << "===== Remapping TxPDO Parameters..." << std::endl;
        //remap used TxPDO to two arbitray parameters of the drive (iMax and vMax)
        uint16_t TxPDOmodule = 0x1A03;
        uint8_t zero = 0;
        uint8_t TxPDOparameterCount = 2;
        uint32_t TxPDOparameter1 = 0x30110C10; //parameter 3011, index 0C, size 10 (16 bits)
        uint32_t TxPDOparameter2 = 0x30111020; //parameter 3011, index 10, size 20 (32 bits)
        //disable pdo by setting the sub-index size to zero
        workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &zero, EC_TIMEOUTSAFE);
        //add first parameter at first index of PDO
        workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x1, false, 4, &TxPDOparameter1, EC_TIMEOUTSAFE);
        //add second parameter at second index of PDO
        workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x2, false, 4, &TxPDOparameter2, EC_TIMEOUTSAFE);
        //enable pdo by setting the index equal to the number of parameters in the pdo
        workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &TxPDOparameterCount, EC_TIMEOUTSAFE);
        if (workingCounter == TxPDOparameterCount + 2) std::cout << "===== Successfully set custom pdo mapping !" << std::endl;
        else std::cout << "===== Failed to set custom pdo mapping..." << std::endl;
        */
        std::cout << "===== Begin PDO assignement..." << std::endl;
        int workingCounter = 0;
        uint8_t PDOoff = 0x00;
        uint8_t PDOon = 0x01;
        //Sync Manager (SM2, SM3) registers that store the mapping objects (modules) which decribe PDO data
        uint16_t RxPDO = 0x1C12;
        uint16_t TxPDO = 0x1C13;
        //mapping object (module) to be stored in each pdo register
        uint16_t RxPDOmodule = 0x1603;
        uint16_t TxPDOmodule = 0x1A03;
        //turn the pdo off by writing a zero to the 0 index, set the mapping object at subindex 1, enable the pdo by writing a 1 (module count) to the index
        workingCounter += ec_SDOwrite(identity.index, RxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
        workingCounter += ec_SDOwrite(identity.index, RxPDO, 0x1, false, 2, &RxPDOmodule, EC_TIMEOUTSAFE);
        workingCounter += ec_SDOwrite(identity.index, RxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
        //do the same for the TxPDO
        workingCounter += ec_SDOwrite(identity.index, TxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
        workingCounter += ec_SDOwrite(identity.index, TxPDO, 0x1, false, 2, &TxPDOmodule, EC_TIMEOUTSAFE);
        workingCounter += ec_SDOwrite(identity.index, TxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
        if (workingCounter == 6) std::cout << "===== PDO assignement successfull !" << std::endl;
        else std::cout << "===== PDO assignement failed..." << std::endl;
    }
}