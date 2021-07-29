#pragma once

#include <ethercat.h>
#include <map>
#include <string>

struct EtherCatSlave {
    ec_slavet* slave_ptr;
    char name[128];
    char displayName[128];
    int index;
    int manualAddress;
    int address;
    bool b_configured = false;
};


class ECatServoDrive{
public:

    long long counter = 0;

    EtherCatSlave identity;

    void process();

    //fieldbus commands
    bool enableVoltage = false;
    bool disableVoltage = false;
    bool switchOn = false;
    bool shutdown = false;
    bool enableOperation = false;
    bool disableOperation = false;
    bool performFaultReset = false;
    bool performQuickStop = false;

    bool jog = false;
    bool direction = false;
    bool fast = false;
    bool stop = false;

    enum class State {
        NotReadyToSwitchOn,
        SwitchOnDisabled,
        ReadyToSwitchOn,
        SwitchedOn,
        OperationEnabled, 
        QuickStopActive,
        FaultReactionActive, 
        Fault
    };

    //===== drive status
    State state = State::SwitchOnDisabled;
    bool motorVoltagePresent = false;
    bool class0error = false;
    bool halted = false;
    bool fieldbusControlActive = false;
    bool targetReached = false;
    bool internalLimitActive = false; //DS402intLim
    bool operatingModeSpecificFlag = false;
    bool stoppedByError = false;
    bool operatingModeFinished = false;
    bool validPositionReference = false;

    //feedback data
    int32_t position = 0;
    int32_t positionError = 0;
    int16_t torque = 0;
    uint16_t lastErrorCode = 0;
    uint16_t digitalInputs = 0;

    //command data
    int32_t positionCommand = 0;
    int32_t velocityCommand = 0;
    int16_t torqueCommand = 0;
    uint16_t digitalOutputCommand = 0;

    static std::map<int, std::string> modelist;

    int mode = 33;
    int modeCommand = 33;
    const char* modeChar = "None";

    uint16_t RxPDO;
    uint16_t TxPDO;

    //drive status memory
    bool b_switchedOn = false;
    bool b_voltageEnabled = false;
    bool b_quickStopActive = true; //quickstop is active when bit is low
    bool b_operationEnabled = false;
    bool b_faultResetState = false;
    bool b_halted = false;
};

