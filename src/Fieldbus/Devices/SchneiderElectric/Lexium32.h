#pragma once

#include "Fieldbus/EtherCatSlave.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

class Lexium32 : public EtherCatSlave {
public:

    SLAVE_DEFINITION(Lexium32, "LXM32M EtherCAT")

    double movementStartPosition = 0.0;
    int counter = 0;
    bool b_inverted = false;
    ScrollingBuffer positions;

    //fieldbus commands
    bool setStartupParameters();
    bool assignPDOs();

    //state machine commands
    void enableVoltage() { b_enableVoltage = true; }
    void disableVoltage() { b_disableVoltage = true; }
    void switchOn() { b_switchOn = true; }
    void shutDown() { b_shutdown = true; }
    void enableOperation() { b_enableOperation = true; }
    void disableOperation() { b_disableOperation = true; }
    void faultReset() { b_faultReset = true; }
    void quickStop() { b_quickStop = true; }

    //===== drive status =====
    
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

    State state = State::SwitchOnDisabled;
    uint16_t lastErrorCode = 0;

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

    //===== mode display and changing =====

    static std::map<int, std::string> modelist;
    int mode = 8;
    const char* modeChar = "Cyclic Synchronous Position";
    int modeCommand = 8;

private:
 
    //Rx PDO display Data
    uint16_t DCOMcontrol = 0;
    int8_t DCOMopmode = 0;
    int32_t PPp_target = 0;
    int32_t PVv_target = 0;
    int16_t PTtq_target = 0;
    uint16_t IO_DQ_set = 0;

    //Tx PDO display Data
    uint16_t _DCOMstatus = 0;
    int8_t _DCOMopmd_act = 0;
    int32_t _p_act = 0;
    int32_t _v_act = 0;
    int16_t _tq_act = 0;
    uint16_t _LastError = 0;
    uint16_t _IO_act = 0;

    //public output data
    ioData positionCommand =   ioData(DataType::REAL_VALUE,    DataDirection::NODE_INPUT, "position command");
    ioData velocityCommand =   ioData(DataType::REAL_VALUE,    DataDirection::NODE_INPUT, "velocity command");
    ioData torqueCommand =     ioData(DataType::REAL_VALUE,    DataDirection::NODE_INPUT, "torque command");
    ioData digitalOut0 =       ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ0");
    ioData digitalOut1 =       ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ1");
    ioData digitalOut2 =       ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "DQ2");

    //public input data
    ioData actualPosition =    ioData(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "actual position");
    ioData actualVelocity =    ioData(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "actual velocity");
    ioData actualTorque =      ioData(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "actual torque");
    ioData digitalIn0 =        ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI0");
    ioData digitalIn1 =        ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI1");
    ioData digitalIn2 =        ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI2");
    ioData digitalIn3 =        ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI3");
    ioData digitalIn4 =        ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI4");
    ioData digitalIn5 =        ioData(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "DI5");

    //command flags to control state machine
    bool b_enableVoltage = false;
    bool b_disableVoltage = false;
    bool b_switchOn = false;
    bool b_shutdown = false;
    bool b_disableOperation = false;
    bool b_enableOperation = false;
    bool b_faultReset = false;
    bool b_quickStop = false;

    //bits used to construct DCOM_control word
    bool b_switchedOn = false;
    bool b_voltageEnabled = false;
    bool b_quickStopActive = true; //quickstop is active when bit is low
    bool b_operationEnabled = false;
    bool opModeSpec4 = false;
    bool opModeSpec5 = false;
    bool opModeSpec6 = false;
    bool b_faultResetState = false;
    bool b_halted = false;
    bool opModeSpec9 = false;
};