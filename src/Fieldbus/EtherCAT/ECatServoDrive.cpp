#include "ECatServoDrive.h"

#include <iostream>
#include <bitset>
#include <ethercat.h>

std::map<int, std::string> ECatServoDrive::modelist = {
    {-6, "Manual/Auto Tuning"},
    {-3, "Motion Sequence"},
    {-2, "Electronic Gear"},
    {-1, "Jog"},
    {0, "Reserved"},
    {1, "Profile Position"},
    {2, "Profile Velocity"},
    {4, "Profile Torque"},
    {6, "Homing"},
    {7, "Interpolated Position"},
    {8, "Cyclic Synchronous Position"},
    {9, "Cyclic Synchronous Velocity"},
    {10, "Cyclic Synchronous Torque"},
    {33, "None"}
};

void ECatServoDrive::process() {

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
    digitalInputs = inByte[15] | inByte[16] << 8;

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
    targetReached =             _DCOMstatus & 0x400;
    internalLimitActive =       _DCOMstatus & 0x800; //DS402intLim
    operatingModeSpecificFlag = _DCOMstatus & 0x1000;
    stoppedByError =            _DCOMstatus & 0x2000;
    operatingModeFinished =     _DCOMstatus & 0x4000;
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
    if (b_faultResetState)  DCOMcontrol |= 0x80;
    if (b_halted)           DCOMcontrol |= 0x100;
    if (!b_faultResetState) faultResetOnPreviousCycle = false; //this prevents two fault reset bits to be sent after one another
    b_faultResetState = false; //reset bit after performing a fault reset

    int8_t DCOMopmode = modeCommand;

    //std::cout << "actual mode: "<< std::bitset<8>(_DCOMopmd_act) << "  mode command: "  << std::bitset<8>(DCOMopmode) << std::endl;

    static int32_t target = 0;
    static uint16_t IO = 0;

    outByte[0] = (DCOMcontrol >> 0) & 0xFF;
    outByte[1] = (DCOMcontrol >> 8) & 0xFF;
    
    outByte[2] = (DCOMopmode >> 8) & 0xFF;

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

    outByte[13] = (digitalOutputCommand >> 0) & 0xFF;
    outByte[14] = (digitalOutputCommand >> 8) & 0xFF;

    //============== OTHER COMMANDS

    /*
        int wc = 0;

        int8_t DCOMopmode = 8;
        wc = ec_SDOwrite(slaves.front().index, 0x6060, 0x0, false, 1, &DCOMopmode, EC_TIMEOUTSAFE);
        std::cout << "set operating mode 8 wc: " << wc << std::endl;

        size = 1;
        int8_t _DCOMopmd_act = 67;
        wc = ec_SDOread(1, 0x6061, 0x0, false, &size, &_DCOMopmd_act, EC_TIMEOUTSAFE);
        std::cout << "slave1 operating Mode: " << std::bitset<8>(_DCOMopmd_act) << " " << std::dec << (int)_DCOMopmd_act << " wc: " << wc << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(100));
    */

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