#pragma once

#include <stdint.h>
#include <vector>
#include <utils/Buffer.h>

class EipServoDrive{
public:

    void receiveInputData(eipScanner::utils::Buffer& data);
    void process();
    std::vector<uint8_t>& getOutputData();

    std::vector<uint8_t> outputData;

    enum State {
        UNKNOWN_STATE = 0,
        START = 1,
        NOT_READY_TO_SWITCH_ON = 2,
        SWITCH_ON_DISABLED = 3,
        READY_TO_SWITCH_ON = 4,
        SWITCHED_ON = 5,
        OPERATION_ENABLED = 6,
        QUICK_STOP_ACTIVE = 7,
        FAULT_REATION_ACTIVE = 8,
        FAULT = 9
    };

    enum Mode {
        UNKNOW_MODE = 0x00,
        PROFILE_POSITION = 0x01,
        PROFILE_VELOCITY = 0x03,
        PROFILE_TORQUE = 0x04,
        HOMING = 0x06,
        MOTION_SEQUENCE = 0x1D,
        ELETRONIC_GEAR = 0x1E,
        JOG = 0x1F
    };

private:

    //raw input data
    uint32_t PCTRLsm;       //parameter read or write feedback
    uint32_t PVsm;          //read value (if the command was to read)
    uint16_t driveStat;     //operating state
    uint16_t mfStat;        //operating mode
    uint16_t motionStat;    //motor and motion profile information
    uint16_t driveInput;    //actual state of digital inputs
    uint32_t _p_act;        //actual position
    int32_t  _v_act;        //actual velocity
    uint16_t _i_act;        //actual current
    uint32_t EthOptMapInp1; //selectable parameter
    uint32_t EthOptMapInp2; //selectable parameter
    uint32_t EthOptMapInp3; //selectable parameter

    State state = State::UNKNOWN_STATE;
    const char* stateChar = "";
    bool error = false;
    bool warning = false;
    bool halted = false;
    bool homingValid = false;
    bool quickstopActive = false;
    bool operatingModeSpecificInfo = false;
    bool operatingModeTerminated = false;
    bool operatingModeTerminatedWithError = false;

    Mode mode = Mode::UNKNOW_MODE;
    const char* modeChar;
    bool dataError;
    bool modeError;
    bool modeToggle;
    uint32_t cap1;
    uint32_t cap2;

    //output data
    uint32_t PCTRLms = 0;           //read (0x13) attribute at subindex 10 (0x0A) of parameter class at index 168 (0x00A8)
    uint32_t PVms = 0;              //parameter value to write (in case the command is write)
    uint16_t dmControl = 0;         //set operating state and operating mode
    uint32_t RefA32 = 0;            //operating mode specific value
    uint32_t RefB32 = 0;            //operating mode specific value
    uint32_t Ramp_v_acc = 0;        //acceleration ramp
    uint32_t Ramp_v_dec = 0;        //deceleration ramp
    uint32_t EthOptMapOut1 = 0;     //selectable parameter
    uint32_t EthOptMapOut2 = 0;     //selectable parameter
    uint32_t EthOptMapOut3 = 0;     //selectable parameter


    uint16_t _LastError = 0;
    uint16_t _LastWarning = 0;
};

