#pragma once

#include "Fieldbus/EtherCatSlave.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

class Lexium32 : public EtherCatSlave {
public:

    SLAVE_DEFINITION(Lexium32, "LXM32M EtherCAT")

    EtherCatData positionOutput;
    EtherCatData digitalOut0;
    EtherCatData digitalOut1;
    EtherCatData digitalOut2;

    EtherCatData positionInput;
    EtherCatData velocityInput;
    EtherCatData torqueInput;
    EtherCatData digitalIn0;
    EtherCatData digitalIn1;
    EtherCatData digitalIn2;
    EtherCatData digitalIn3;
    EtherCatData digitalIn4;
    EtherCatData digitalIn5;

    int counter = 0;

    bool setStartupParameters();
    bool assignPDOs();

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
    int32_t velocity = 0;
    int16_t torque = 0;
    uint16_t lastErrorCode = 0;
    bool DI0 = false;
    bool DI1 = false;
    bool DI2 = false;
    bool DI3 = false;
    bool DI4 = false;
    bool DI5 = false;

    bool b_inverted = false;
    ScrollingBuffer positions;

    //command data
    int32_t positionCommand = 0;
    int32_t velocityCommand = 0;
    int16_t torqueCommand = 0;
    bool DQ0 = false;
    bool DQ1 = false;
    bool DQ2 = false;

    //operating mode specific commands
    bool opModeSpec4 = false;
    bool opModeSpec5 = false;
    bool opModeSpec6 = false;
    bool opModeSpec9 = false;

    //modes

    static std::map<int, std::string> modelist;
    int mode = 8;
    int modeCommand = 8;
    const char* modeChar = "Cyclic Synchronous Position";

    //maybe do gui remapping of pdo objects later
    //uint16_t RxPDO;
    //uint16_t TxPDO;

    //drive status memory
    bool b_switchedOn = false;
    bool b_voltageEnabled = false;
    bool b_quickStopActive = true; //quickstop is active when bit is low
    bool b_operationEnabled = false;
    bool b_faultResetState = false;
    bool b_halted = false;

};