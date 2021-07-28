#include "EipServoDrive.h"

#include <IdentityObject.h>

#include <iostream>
#include <bitset>

#include "EthernetIpFieldbus.h"
#include "StatusCodePrint.h"


void EipServoDrive::connect() {
    std::cout << "===== Connecting to " << identity.host << ":" << std::dec << identity.port << std::endl;
    session = std::make_shared<eipScanner::SessionInfo>(identity.host, identity.port);
    b_connected = (session == nullptr) ? false : true;
    if (b_connected) {
        std::cout << "===== Connection Success !" << std::endl;
        startImplicitMessaing();
    }
    else {
        std::cout << "===== Connection Failed..." << std::endl;
    }
}

void EipServoDrive::disconnect() {
    std::cout << "===== Disconnecting from " << identity.host << ":" << std::dec << identity.port << std::endl;
    if (b_implicitMessaging) {
        stopImplicitMessaging();
    }
    session = nullptr;
    b_connected = false;
    std::cout << "===== Disconnnected !" << std::endl;
}

void EipServoDrive::startImplicitMessaing() {
    eipScanner::ConnectionManager& connectionManager = EthernetIPFieldbus::getConnectionManager();
    
    using namespace eipScanner::cip::connectionManager;

    //forward open request timeout parameters
    connectionParameters.priorityTimeTick |= 0x10;    //defines priority for the forward open request
    connectionParameters.priorityTimeTick |= 4;       //defines length of a tick in ms (=2^x) (max 15)
    connectionParameters.timeoutTicks = 8;          //number of ticks for forward request timeout(max 255)
    //2^4 ms * 8 ticks = 128ms

    //connectionParameters.o2tNetworkConnectionId = 1;                                                //useless, defined by originator
    //connectionParameters.t2oNetworkConnectionId = 2;                                                //useless, defined by target
    //connectionParameters.connectionSerialNumber = 0xFFFF;                                           //uselesss, defined by static library counter

    //connectionParameters.originatorVendorId = 333;                                                  //useless
    //connectionParameters.originatorSerialNumber = 0x333;                                            //useless

    //amount of messages that can be missed before timeout trigger (4*2^x messages) 
    connectionParameters.connectionTimeoutMultiplier = 3; //4*2^3 = 32 messages

    //output assembly (103)
    connectionParameters.o2tRPI = 10000;                                                             //IMPORTANT (in microseconds)
    connectionParameters.o2tNetworkConnectionParams |= NetworkConnectionParams::OWNED;                //IMPORTANT
    connectionParameters.o2tNetworkConnectionParams |= NetworkConnectionParams::P2P;                  //IMPORTANT
    connectionParameters.o2tNetworkConnectionParams |= NetworkConnectionParams::SCHEDULED_PRIORITY;   //IMPORTANT
    connectionParameters.o2tNetworkConnectionParams |= 38;                                            //IMPORTANT
    connectionParameters.o2tRealTimeFormat = true;                                                    //IMPORTANT

    //input assembly (113)
    connectionParameters.t2oRPI = 10000;                                                             //IMPORTANT (in microseconds)
    connectionParameters.t2oNetworkConnectionParams |= NetworkConnectionParams::OWNED;                //IMPORTANT
    connectionParameters.t2oNetworkConnectionParams |= NetworkConnectionParams::P2P;
    connectionParameters.t2oNetworkConnectionParams |= NetworkConnectionParams::SCHEDULED_PRIORITY;   //IMPORTANT
    connectionParameters.t2oNetworkConnectionParams |= 38;                                            //IMPORTANT
    connectionParameters.t2oRealTimeFormat = false;                                                   //IMPORTANT

    connectionParameters.transportTypeTrigger |= NetworkConnectionParams::TRIG_CYCLIC;                //IMPORTANT
    connectionParameters.transportTypeTrigger |= NetworkConnectionParams::CLASS1;                     //IMPORTANT

    //parameters.connectionPathSize = 8;                                                              //useless, automatically calculated from vector
    connectionParameters.connectionPath = { 0x20, 0x04, 0x24, 0x05, 0x2C, 0x67, 0x2C, 0x71 };         //IMPORTANT

    ioConnection = connectionManager.forwardOpen(session, connectionParameters);

    if (auto ptr = ioConnection.lock()) {
        b_implicitMessaging = true;
        std::cout << "===== Starting Implicit I/O Message Receiving And Sending" << std::endl;

        ptr->setDataToSend(getOutputData());

        ptr->setReceiveDataListener([this](eipScanner::cip::CipUdint realtimeHeader, eipScanner::cip::CipUint sequence, const std::vector<uint8_t>& data) {
            receiveInputData(eipScanner::utils::Buffer(data));
        });

        ptr->setSendDataListener([this](std::vector<uint8_t>& data) {
            process();
            data = getOutputData();
        });

        ptr->setCloseListener([this]() {
            std::cout << "===== Connection Lost, Disconnecting..." << std::endl;
            disconnect();
        });
    }
    else {
        b_implicitMessaging = false;
        std::cout << "===== Failed to start Implicit I/O Messaging..." << std::endl;
    }
}


void EipServoDrive::stopImplicitMessaging() {
    eipScanner::ConnectionManager& connectionManager = EthernetIPFieldbus::getConnectionManager();
    connectionManager.forwardClose(session, ioConnection);
    b_implicitMessaging = false;
}



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
        //std::cout << std::dec << "Read Parameter " << std::dec << (int)index << ".1." << (int)subindex << ": ";
        if (readByteCtrl == 0x13) {
            uint16_t parameterValue = PVsm;
            if (index == 128 && subindex == 5) {
                lastErrorCode = parameterValue;
            }
            else if (index == 128 && subindex == 9) {
                lastWarningCode = parameterValue;
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
            //std::cout << "READ ERROR: " << error << std::endl;
        }
    }



    uint8_t stateBits = driveStat & 0xF;
    switch (stateBits) {
    case 1: 
        stateChar = "Start";
        state = State::START;
        b_powerStageEnabled = false;
        break;
    case 2: 
        stateChar = "Not Ready to Switch On";
        state = State::NOT_READY_TO_SWITCH_ON;
        b_powerStageEnabled = false;
        break;
    case 3:
        stateChar = "Switch On Disabled";
        state = State::SWITCH_ON_DISABLED;
        b_powerStageEnabled = false;
        break;
    case 4:
        stateChar = "Ready To Switch On";
        state = State::READY_TO_SWITCH_ON;
        b_powerStageEnabled = false;
        break;
    case 5:
        stateChar = "Switched On";
        state = State::SWITCHED_ON;
        b_powerStageEnabled = true;
        break;
    case 6:
        stateChar = "Operation Enabled";
        state = State::OPERATION_ENABLED;
        b_powerStageEnabled = true;
        break;
    case 7:
        stateChar = "Quick Stop Active";
        state = State::QUICK_STOP_ACTIVE;
        b_powerStageEnabled = true;
        break;
    case 8:
        stateChar = "Fault Reaction Active";
        state = State::FAULT_REATION_ACTIVE;
        b_powerStageEnabled = false;
        break;
    case 9:
        stateChar = "Fault";
        state = State::FAULT;
        b_powerStageEnabled = false;
        break;
    default:
        stateChar = "unknow state...";
        state = State::UNKNOWN_STATE;
        b_powerStageEnabled = false;
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

    positiveLimit = motionStat & (0x1 << 0);
    negativeLimit = motionStat & (0x1 << 1);
    motorStandstill = motionStat & (0x1 << 6);
    motorPositiveMovement = motionStat & (0x1 << 7);
    motorNegativeMovement = motionStat & (0x1 << 8);
    DS402intLim = motionStat & (0x1 << 9);
    DPL_intLim = motionStat & (0x1 << 10);
    profileGeneratorStandstill = motionStat & (0x1 << 11);
    profileGeneratorDecelerates = motionStat & (0x1 << 12);
    profileGeneratorAccelerates = motionStat & (0x1 << 13);
    profileGeneratorConstantVelocity = motionStat & (0x1 << 14);
    
    /*
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
    */
}

void EipServoDrive::process() {
    
    //reset state and mode control word (disables the power stage if left in this state)
    dmControl = 0x0000;

    //set the disable bit if that command was executed
    if (disablePowerStage) {
        dmControl |= 0x1 << 8;
        requestedVelocity = 0;
        disablePowerStage = false;
    } //set the b_powerStageEnable flag to permanently have the enable bit high
    else if (enablePowerStage) {
        enablePowerStage = false;
        b_powerStageEnabled = true;
        requestedVelocity = 0;
    }
    //enable the bit to the turn power stage on (or keep it on, since having it at zero might disable the power stage)
    if (b_powerStageEnabled) {
        dmControl |= 0x1 << 9;
    }

    if (enableHalt) {
        dmControl |= 0x1 << 13;
        enableHalt = false;
        requestedVelocity = 0;
    }
    if (clearHalt) {
        dmControl |= 0x1 << 14;
        clearHalt = false;
    }
    if (performFaultReset) {
        dmControl |= 0x1 << 11;
        performFaultReset = false;
    }

    if (performQuickStop) {
        dmControl |= 0x1 << 10;
        performQuickStop = false;
        requestedVelocity = 0;
    }

    //by default no special data is read or written
    PCTRLms = 0x03000000;
    PVms = 0x00000000;
    if (state == State::FAULT || error) {
        PCTRLms = 0x13050080; //read (0x13) subindex 5 (0x05) of object index 128 (0x0080) (unt16_t _LastError)
    }
    else {
        lastErrorCode = 0;
    }
    if (warning) {
        PCTRLms = 0x13090080; //read (0x13) subindex 5 (0x09) of object index 128 (0x0080) (unt16_t _LastWarning)
    }
    else {
        lastWarningCode = 0;
    }

    //get the current state of the mode toggle bit
    bool toggleBit = modeToggle;

    if (mode != Mode::PROFILE_VELOCITY) {
        mode = Mode::PROFILE_VELOCITY;
        toggleBit = !toggleBit;
    }
    else if (mode == Mode::PROFILE_VELOCITY && (operatingModeTerminated && requestedVelocity != 0) || (requestedVelocity != executedVelocity)) {
        toggleBit = !toggleBit;
        executedVelocity = requestedVelocity;
    }

    RefA32 = requestedVelocity;
    dmControl |= 0x23;
    dmControl |= toggleBit << 7;

    EthOptMapOut1 = 0x0;
    EthOptMapOut2 = 0x0;
    EthOptMapOut3 = 0x0;
}





void EipServoDrive::printNetworkConfiguration() {
    using namespace eipScanner;
    using namespace eipScanner::cip;
    using namespace eipScanner::utils;

    eipScanner::cip::MessageRouterResponse response;

    eipScanner::IdentityObject identityObject(1, session);
    std::cout << identityObject.getProductName() << std::endl;

    uint16_t ipMode = -1;
    uint16_t ipA = -1, ipB = -1, ipC = -1, ipD = -1;
    uint16_t mskA = -1, mskB = -1, mskC = -1, mskD = -1;

    std::cout << "===== Downloading IP Address settings from the Device..." << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 5));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> ipMode;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 7));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> ipA;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 8));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> ipB;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 9));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> ipC;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 10));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> ipD;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 11));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> mskA;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 12));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> mskB;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 13));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> mskC;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(168, 1, 14));
    if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
        Buffer buffer(response.getData());
        buffer >> mskD;
    }
    else std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    std::cout << "IpMode: " << ((ipMode == 0) ? "Manual" : ((ipMode == 1) ? "BootP" : "DHCP")) << std::endl;
    std::cout << "DeviceIP: " << ipA << "." << ipB << "." << ipC << "." << ipD << std::endl;
    std::cout << "DeviceNetMask: " << mskA << "." << mskB << "." << mskC << "." << mskD << std::endl;

    /*
    std::cout << "===== Uploading Motor Current Parameters to the device..." << std::endl;

    uint16_t current = 854; //increments of 0.01 Ampere RMS
    Buffer currentBuffer;
    currentBuffer << current;
    response = messageRouter.sendRequest(session, ServiceCodes::SET_ATTRIBUTE_SINGLE, EPath(117, 1, 12), currentBuffer.data());
    std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    //save parameter
    uint16_t save = 1;
    Buffer saveBuffer;
    saveBuffer << save;
    response = messageRouter.sendRequest(session, ServiceCodes::SET_ATTRIBUTE_SINGLE, EPath(104, 1, 1), saveBuffer.data());
    std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;

    std::cout << "===== Saving Parameters To EEPROM";
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    uint16_t saveValue = 1;
    while (saveValue != 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        response = messageRouter.sendRequest(session, ServiceCodes::GET_ATTRIBUTE_SINGLE, EPath(104, 1, 1));
        //std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;
        if (response.getGeneralStatusCode() == GeneralStatusCodes::SUCCESS) {
            Buffer saveReturnBuffer(response.getData());
            saveReturnBuffer >> saveValue;
            std::cout << ".";
        }
    }
    std::chrono::duration saveDelay = std::chrono::steady_clock::now() - start;
    long long saveDelayMillis = std::chrono::duration_cast<std::chrono::milliseconds>(saveDelay).count();
    std::cout << std::endl << "===== Saved Paramters to EEPROM (took " << saveDelayMillis << "ms)" << std::endl;
    */
}

void EipServoDrive::reboot() {
    std::cout << "===== Rebooting Device..." << std::endl;
    MessageRouterResponse response;
    eipScanner::IdentityObject identityObject(1, session);
    response = messageRouter.sendRequest(session, 0x05, EPath(identityObject.getClassId(), 1));
    //std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;
}