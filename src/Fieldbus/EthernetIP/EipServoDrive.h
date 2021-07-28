#pragma once

#include <stdint.h>
#include <vector>

#include <SessionInfo.h>
#include <MessageRouter.h>
#include <ConnectionManager.h>
#include <IOConnection.h>
#include <cip/connectionManager/NetworkConnectionParams.h>
#include <utils/Buffer.h>

struct EipDevice {
    char productName[32];
    uint16_t productCode;
    uint8_t revisionMajor;
    uint8_t revisionMinor;
    uint32_t serialNumber;
    uint16_t vendorId;
    uint16_t status;
    char host[32];
    int port;
    char displayName[64];
};

class EipServoDrive{
public:

    void receiveInputData(eipScanner::utils::Buffer& data);
    void process();
    std::vector<uint8_t>& getOutputData();

    std::vector<uint8_t> outputData;

    void connect();
    void disconnect();

    bool b_connected = false;
    bool b_implicitMessaging = false;
    bool b_powerStageEnabled = false;

    void printNetworkConfiguration();
    void reboot();

    std::shared_ptr<eipScanner::SessionInfo> session;
    eipScanner::MessageRouter messageRouter;
    eipScanner::cip::connectionManager::ConnectionParameters connectionParameters;
    std::weak_ptr<eipScanner::IOConnection> ioConnection;

    EipDevice identity;

    //commands
    int32_t requestedVelocity = 0;
    int32_t executedVelocity = 0;
    bool disablePowerStage = false;
    bool enablePowerStage = false;
    bool performQuickStop = false;
    bool performFaultReset = false;
    bool enableHalt = false;
    bool clearHalt = false;

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

    void startImplicitMessaing();
    void stopImplicitMessaging();

    //raw input data
    uint32_t PCTRLsm = 0;       //parameter read or write feedback
    uint32_t PVsm = 0;          //read value (if the command was to read)
    uint16_t driveStat = 0;     //operating state
    uint16_t mfStat = 0;        //operating mode
    uint16_t motionStat = 0;    //motor and motion profile information
    uint16_t driveInput = 0;    //actual state of digital inputs
    uint32_t _p_act = 0;        //actual position
    int32_t  _v_act = 0;        //actual velocity
    uint16_t _i_act = 0;        //actual current
    uint32_t EthOptMapInp1 = 0; //selectable parameter
    uint32_t EthOptMapInp2 = 0; //selectable parameter
    uint32_t EthOptMapInp3 = 0; //selectable parameter

    uint16_t lastErrorCode = 0;
    uint16_t lastWarningCode = 0;

    //interpreted input data
    EipServoDrive::State state = State::UNKNOWN_STATE;
    const char* stateChar = "";
    bool error = false;
    bool warning = false;
    bool halted = false;
    bool homingValid = false;
    bool quickstopActive = false;
    bool operatingModeSpecificInfo = false;
    bool operatingModeTerminated = false;
    bool operatingModeTerminatedWithError = false;

    EipServoDrive::Mode mode = Mode::UNKNOW_MODE;
    const char* modeChar = "";
    bool dataError = false;
    bool modeError = false;
    bool modeToggle = false;
    uint32_t cap1 = 0;
    uint32_t cap2 = 0;

    bool positiveLimit = false;
    bool negativeLimit = false;
    bool motorStandstill = false;
    bool motorPositiveMovement = false;
    bool motorNegativeMovement = false;
    bool DS402intLim = false;
    bool DPL_intLim = false;
    bool profileGeneratorStandstill = false;
    bool profileGeneratorDecelerates = false;
    bool profileGeneratorAccelerates = false;
    bool profileGeneratorConstantVelocity = false;

    //raw output data
    uint32_t PCTRLms = 0;           //read (0x13) attribute at subindex 10 (0x0A) of parameter class at index 168 (0x00A8)
    uint32_t PVms = 0;              //parameter value to write (in case the command is write)
    uint16_t dmControl = 0;         //set operating state and operating mode
    uint32_t RefA32 = 0;            //operating mode specific value
    uint32_t RefB32 = 0;            //operating mode specific value
    uint32_t Ramp_v_acc = 1000;        //acceleration ramp
    uint32_t Ramp_v_dec = 1000;        //deceleration ramp
    uint32_t EthOptMapOut1 = 0;     //selectable parameter
    uint32_t EthOptMapOut2 = 0;     //selectable parameter
    uint32_t EthOptMapOut3 = 0;     //selectable parameter
};

