#include "EipServoDrive.h"

#include <iostream>
#include <bitset>

std::vector<uint8_t>& EipServoDrive::getOutputData() {
    outputData.clear();
    eipScanner::utils::Buffer outputBuffer;
    outputBuffer << PCTRLms
        << PVms
        << dmControl
        << RefA32
        << RefB32
        << Ramp_v_acc
        << Ramp_v_dec
        << EthOptMapOut1
        << EthOptMapOut2
        << EthOptMapOut3;
    outputData = outputBuffer.data();
    return outputData;
}


void EipServoDrive::receiveInputData(eipScanner::utils::Buffer& buffer) {

    //get new data from buffer
    buffer >> PCTRLsm
        >> PVsm
        >> driveStat
        >> mfStat
        >> motionStat
        >> driveInput
        >> _p_act
        >> _v_act
        >> _i_act
        >> EthOptMapInp1
        >> EthOptMapInp2
        >> EthOptMapInp3;

    uint8_t readByteCtrl = PCTRLsm >> 24;
    if (readByteCtrl == 0x13 || readByteCtrl == 0x23 || readByteCtrl == 0x73) {
        uint16_t index = PCTRLsm & 0x0000FFFF;
        uint8_t subindex = (PCTRLsm & 0x00FF0000) >> 16;
        std::cout << std::dec << "Read Parameter " << std::dec << (int)index << ".1." << (int)subindex << ": ";
        if (readByteCtrl == 0x13) {
            uint16_t parameterValue = PVsm;
            if (index == 128 && subindex == 5) {
                std::cout << "Error Code: " << std::hex << parameterValue << std::endl;
                _LastError = parameterValue;
            }
            else if (index == 128 && subindex == 9) {
                std::cout << "Warning Code: " << std::hex << parameterValue << std::endl;
                _LastError = parameterValue;
            }
            else {
                std::cout << (int)parameterValue << std::endl;
            }
        }
        else if (readByteCtrl == 0x23) {
            uint32_t parameterValue = PVsm;
            std::cout << (int)parameterValue << std::endl;
        }
        else if (readByteCtrl == 0x73) {
            uint32_t error = PVsm;
            std::cout << "READ ERROR: " << error << std::endl;
        }
    }



    uint8_t stateBits = driveStat & 0xF;
    switch (stateBits) {
    case 1: 
        stateChar = "Start";
        state = State::START;
        break;
    case 2: 
        stateChar = "Not Ready to Switch On";
        state = State::NOT_READY_TO_SWITCH_ON;
        break;
    case 3:
        stateChar = "Switch On Disabled";
        state = State::SWITCH_ON_DISABLED;
        break;
    case 4:
        stateChar = "Ready To Switch On";
        state = State::READY_TO_SWITCH_ON;
        break;
    case 5:
        stateChar = "Switched On";
        state = State::SWITCHED_ON;
        break;
    case 6:
        stateChar = "Operation Enabled";
        state = State::OPERATION_ENABLED;
        break;
    case 7:
        stateChar = "Quick Stop Active";
        state = State::QUICK_STOP_ACTIVE;
        break;
    case 8:
        stateChar = "Fault Reaction Active";
        state = State::FAULT_REATION_ACTIVE;
        break;
    case 9:
        stateChar = "Fault";
        state = State::FAULT;
        break;
    default:
        stateChar = "unknow state...";
        state = State::UNKNOWN_STATE;
    }

    error = driveStat & (1 << 6);
    warning = driveStat & (1 << 7);
    halted = driveStat & (1 << 8);
    homingValid = driveStat & (1 << 9);
    quickstopActive = driveStat & (1 << 10);
    operatingModeSpecificInfo = driveStat & (1 << 13);
    operatingModeTerminated = driveStat & (1 << 14);
    operatingModeTerminatedWithError = driveStat & (1 << 15);


    uint8_t modebits = mfStat & 0x1F;
    switch (modebits) {
    case 0x01:
        modeChar = "Profile Position";
        mode = Mode::PROFILE_POSITION;
        break;
    case 0x03:
        modeChar = "Profile Velocity";
        mode = Mode::PROFILE_VELOCITY;
        break;
    case 0x04:
        modeChar = "Profile Torque";
        mode = Mode::PROFILE_TORQUE;
        break;
    case 0x06:
        modeChar = "Homing";
        mode = Mode::HOMING;
        break;
    case 0x1D:
        modeChar = "Motion Sequence";
        mode = Mode::MOTION_SEQUENCE;
        break;
    case 0x1E:
        modeChar = "Electronic Gear";
        mode = Mode::ELETRONIC_GEAR;
        break;
    case 0x1F:
        modeChar = "Jog";
        mode = Mode::JOG;
        break;
    default:
        modeChar = "unknown mode...";
        mode = Mode::UNKNOW_MODE;
    }

    dataError = mfStat & (1 << 5);
    modeError = mfStat & (1 << 6);
    modeToggle = mfStat & (1 << 7);
    cap1 = (mfStat >> 8) & 0x3;
    cap2 = (mfStat >> 10) & 0x3;

    bool positiveLimit = motionStat & (0x1 << 0);
    bool negativeLimit = motionStat & (0x1 << 1);
    bool motorStandstill = motionStat & (0x1 << 6);
    bool motorPositiveMovement = motionStat & (0x1 << 7);
    bool motorNegativeMovement = motionStat & (0x1 << 8);
    bool DS402intLim = motionStat & (0x1 << 9);
    bool DPL_intLim = motionStat & (0x1 << 10);
    bool profileGeneratorStandstill = motionStat & (0x1 << 11);
    bool profileGeneratorDecelerates = motionStat & (0x1 << 12);
    bool profileGeneratorAccelerates = motionStat & (0x1 << 13);
    bool profileGeneratorConstantVelocity = motionStat & (0x1 << 14);

    
    std::cout << "State: ==" << stateChar << "== bin: (" << std::bitset<4>(stateBits) << ")";
    if (error) std::cout << " Error! ";
    if (warning) std::cout << " Warning! ";
    if (halted) std::cout << " Halted! ";
    if (homingValid) std::cout << " homingValid ";
    if (quickstopActive) std::cout << " QuickstopActive ";
    if (operatingModeSpecificInfo) std::cout << " operatingModeSpecificInfo ";
    if (operatingModeTerminated) std::cout << " operatingModeTerminated ";
    if (operatingModeTerminatedWithError) std::cout << " operatingModeTerminatedWithError ";
    std::cout << "Mode: ==" << modeChar << "== (" << std::bitset<5>(modebits) << ") ";
    if (dataError) std::cout << " DataError! ";
    if (modeError) std::cout << " ModeError! ";
    if (modeToggle) std::cout << " ModeToggle! ";
    if (cap1) std::cout << " cap1: " << cap1;
    if (cap2) std::cout << " cap2: " << cap2;
    if (positiveLimit) std::cout << "positiveLimit ";
    if (negativeLimit) std::cout << "negativeLimit ";
    if (motorStandstill) std::cout << "motorStandstill ";
    if (motorPositiveMovement) std::cout << "motorPositiveMovement ";
    if (motorNegativeMovement) std::cout << "motorNegativeMovement ";
    if (DS402intLim) std::cout << "DS402intLim ";
    if (DPL_intLim) std::cout << "DPL_intLim ";
    if (profileGeneratorStandstill) std::cout << "profileGeneratorStandstill ";
    if (profileGeneratorDecelerates) std::cout << "profileGeneratorDecelerates ";
    if (profileGeneratorAccelerates) std::cout << "profileGeneratorAccelerates ";
    if (profileGeneratorConstantVelocity) std::cout << "profileGeneratorConstantVelocity ";
    std::cout << "Inputs: " << std::bitset<16>(driveInput);
    std::cout << "Position: " << std::dec << _p_act << " ";
    std::cout << "Velocity: " << std::dec << _v_act << " ";
    std::cout << "Current: " << std::dec << _i_act << " ";
    std::cout << std::endl;
}

void EipServoDrive::process() {
    
    //bool disablePowerStage = false;
    //bool performQuickStop = false;

    static int counter = 0;
    counter++;

    //by default no special data is read or written
    PCTRLms = 0x03000000;
    PVms = 0x00000000;

    //reset state and mode control word
    dmControl = 0x0000;

    if (state == State::FAULT || error) {
        PCTRLms = 0x13050080; //read (0x13) subindex 5 (0x05) of object index 128 (0x0080) (unt16_t _LastError)
        //perform fault reset
        dmControl |= 0x1 << 11;
    }
    else if (warning) {
        PCTRLms = 0x13090080; //read (0x13) subindex 5 (0x09) of object index 128 (0x0080) (unt16_t _LastWarning)
        //enable power stage
        dmControl = 0x1 << 9;
    }
    else if (state == State::READY_TO_SWITCH_ON || state == State::OPERATION_ENABLED || state == State::SWITCHED_ON) {
        //enable power stage
        dmControl |= 0x1 << 9;
    }

    //get the current state of the mode toggle bit
    bool toggleBit = modeToggle;
    if (mode != Mode::JOG) {
        mode = Mode::JOG;
        RefA32 = 0;
        toggleBit = !toggleBit;
    }
    else if (counter % 20 < 10 && RefA32 == 5) {
        RefA32 = 0;
        toggleBit = !toggleBit;
    }
    else if(counter % 20 > 10 && RefA32 == 0){
        RefA32 = 5;
        toggleBit = !toggleBit;
    }
    dmControl |= mode;
    dmControl |= toggleBit << 7;

    Ramp_v_acc = 10000;
    Ramp_v_dec = 10000;

    EthOptMapOut1 = 0x0;
    EthOptMapOut2 = 0x0;
    EthOptMapOut3 = 0x0;

    //if (halted) dmControl |= 0x1 << 14;
    //if (disablePowerStage) dmControl |= 0x1 << 8;
    //if (performQuickStop) dmControl |= 0x1 << 10;
}