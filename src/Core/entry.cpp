//#define ECAT_DEV
#define EIP_DEV

#ifdef ECAT_DEV

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>

const char* getStateString(uint16_t state) {
    switch (state) {
    case EC_STATE_NONE :
        return "NO STATE";
    case EC_STATE_INIT :
        return "INIT";
    case EC_STATE_PRE_OP :
        return "PRE-OPERATIONAL";
    case EC_STATE_BOOT :
        return "BOOT";
    case EC_STATE_SAFE_OP :
        return "SAFE-OPERATIONAL";
    case EC_STATE_OPERATIONAL :
        return "OPERATIONAL";
    case EC_STATE_ACK :
        return "STATE-ACK / ERROR";
    default:
        return "UNKNOWN STATE";
    }
}

void EtherCAT_dev(const char* nic) {

    if (ec_init(nic)) std::cout << "Initialized network interface card : " << nic << std::endl;
    else std::cout << "Failed to initialize network interface card " << nic << std::endl;

    int workingCounter = ec_config_init(FALSE);

    if (workingCounter > 0) {
        ec_slavet* slave = &ec_slave[1];
        std::cout << "Found and configured " << ec_slavecount << " EtherCAT slaves!" << std::endl;

        workingCounter = ec_readstate();
        std::cout << "==wc:" << workingCounter << " slave0 state: " << getStateString(ec_slave[0].state) << std::endl;

        char ioMap[128];
        int ioMapSize = ec_config_map(&ioMap);
        std::cout << "IO map configured, size: " << ioMapSize << std::endl;

        workingCounter = ec_readstate();
        std::cout << "==wc:" << workingCounter << " slave0 state: " << getStateString(ec_slave[0].state) << std::endl;

        ec_configdc();
        std::cout << "configured distributed clocks" << std::endl;

        workingCounter = ec_readstate();
        std::cout << "==wc:" << workingCounter << " slave0 state: " << getStateString(ec_slave[0].state) << std::endl;

        printf("Setting operational state..");
        /* Act on slave 0 (a virtual slave used for broadcasting) */
        ec_slavet* broadcastSlave = &ec_slave[0];
        broadcastSlave->state = EC_STATE_OPERATIONAL;
        ec_writestate(0);

        //wait for all slaves to reach OP state
        ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
        std::cout << "all slaves operational" << std::endl;

        workingCounter = ec_readstate();
        std::cout << "==wc:" << workingCounter << " slave0 state: " << getStateString(ec_slave[0].state) << std::endl;

        /*
        std::cout << "slave 1: " << slave->name
            << "\n activeports: " << slave->activeports
            << "\n aliasadr " << slave->aliasadr
            << "\n configadr " << slave->configadr
            << "\n configindex " << slave->configindex
            << "\n alstatuscode " << ec_ALstatuscode2string(slave->ALstatuscode)
            << "\n entryport " << slave->entryport
            << "\n itype " << slave->Itype
            << "\n state " << slave->state
            << "\n topology " << slave->topology
            << "\n ibytes " << slave->Ibytes
            << "\n ibits " << slave->Ibits
            << "\n obytes " << slave->Obytes
            << "\n obits " << slave->Obits
            << std::endl;
         */

        //ec_B--  broadcast addressing VV
        //ec_AP-  position addressing  VV
        //ec_FP-  node addressing      VV
        //ec_L--  logical addressing   VV
        
        /*
        uint16_t LXM_operatingState = 0;
        workingCounter = ec_BRD(0, 0x60410, 2, &LXM_operatingState, EC_TIMEOUTSTATE * 3);
        std::cout << "read result: " << workingCounter << " OperatingState: " << LXM_operatingState << std::endl;

        int16_t LXM_operatingMode = 0;
        workingCounter = ec_BRD(0, 0x60610, 2, &LXM_operatingMode, EC_TIMEOUTSTATE * 3);
        std::cout << "read result: " << workingCounter << " OperatingMode: " << LXM_operatingMode << std::endl;

        uint16_t opState = 0;
        workingCounter = ec_FPRD(4097, 0x60410, 2, &opState, EC_TIMEOUTSTATE * 3);
        std::cout << "read result: " << workingCounter << " opState: " << opState << std::endl;
        
        int16_t opMode = 0;
        workingCounter = ec_FPRD(4097, 0x60610, 2, &opMode, EC_TIMEOUTSTATE * 3);
        std::cout << "read result: " << workingCounter << " opMode: " << opMode << std::endl;
        */

        while (true) {
        
            static uint16_t DCOM = 0;
            static uint32_t target = 0;
            static uint16_t IO = 0;

            //DCOM = 1 << 6;
            target++;

            IO = 0;
            if ((target + 0) % 60 < 30) IO |= 1;
            if ((target + 15) % 60 < 30) IO |= 2;
            if ((target + 30) % 60 < 30) IO |= 4;

            uint8_t* ioMapOut = slave->outputs;
            ioMapOut[0] = (DCOM >> 0) & 0xFF;
            ioMapOut[1] = (DCOM >> 8) & 0xFF;
            ioMapOut[2] = (target >> 0) & 0xFF;
            ioMapOut[3] = (target >> 8) & 0xFF;
            ioMapOut[4] = (target >> 16) & 0xFF;
            ioMapOut[5] = (target >> 24) & 0xFF;
            ioMapOut[6] = (IO >> 0) & 0xFF;
            ioMapOut[7] = (IO >> 8) & 0xFF;

            ec_send_processdata();
            workingCounter = ec_receive_processdata(EC_TIMEOUTRET);

            if (workingCounter == EC_NOFRAME) {
                std::cout << "timeout" << std::endl;
                continue;
            }

            if (workingCounter != 3) {
                std::cout << "Wrong Working Counter" << std::endl;
                continue;
            }
   
            uint8_t* inByte = slave->inputs;
            uint16_t _DCOMstatus = inByte[0] | inByte[1] << 8;
            uint32_t _p_act = inByte[2] | inByte[3] << 8 | inByte[4] << 16 | inByte[5] << 24;
            uint16_t _LastError = inByte[6] | inByte[7] << 8;
            uint16_t _IO_act = inByte[8] | inByte[9] << 8;

            std::bitset<16> DCOMbits(_DCOMstatus);
            std::bitset<16> IObits(_IO_act);
            std::cout << "===Inputs=== OpState: " << DCOMbits << " ActualPosition: " << _p_act << " LastError: " << _LastError << " IO-inputs: " << IObits;

            uint8_t* outByte = slave->outputs;
            uint16_t _DCOMcontrol = outByte[0] | outByte[1] << 8;
            uint32_t PPp_target = outByte[2] | outByte[3] << 8 | outByte[4] << 16 | outByte[5] << 24;
            uint16_t IO_DQ_set = outByte[6] | outByte[7] << 8;

            std::bitset<16> DCOMcBits(_DCOMcontrol);
            std::bitset<16> IO_DQbits(IO_DQ_set);
            std::cout << "  ===Outputs=== OpStateCommand: " << DCOMcBits << " PositionTarget: " << PPp_target << " IO-ouputs " << IO_DQbits << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }


    } else {
        std::cout << "Did not find any EtherCAT slaves on the network" << std::endl;
    }
    ec_close();
}

struct NetworkInterfaceCard {
    char* description;
    char* name;
};

NetworkInterfaceCard* networkInterfaceCards;
size_t networkInterfaceCardCount;

int main() {
    ec_adaptert* nics = ec_find_adapters();
    if (nics != nullptr) {
        networkInterfaceCardCount = 0;
        ec_adapter* nic = nics;
        while (nic != nullptr) {
            networkInterfaceCardCount++;
            nic = nic->next;
        }
        nic = nics;
        networkInterfaceCards = new NetworkInterfaceCard[networkInterfaceCardCount];
        int i = 0;
        while (nic != nullptr) {
            networkInterfaceCards[i] = { nic->desc, nic->name };
            i++;
            nic = nic->next;
        }
    }
    std::cout << "===== Available Network Interface Cards =====" << std::endl;
    for (int i = 0; i < networkInterfaceCardCount; i++) std::cout << networkInterfaceCards[i].description << std::endl;
    NetworkInterfaceCard selectedNetworkInterfaceCard = networkInterfaceCards[0];

    EtherCAT_dev(selectedNetworkInterfaceCard.name);
}

#endif














#ifdef EIP_DEV







#include <MessageRouter.h>
#include <ConnectionManager.h>
#include <DiscoveryManager.h>
#include <cip/connectionManager/NetworkConnectionParams.h>
#include <IdentityObject.h>
#include <utils/Logger.h>
#include <utils/Buffer.h>

#include <iostream>
#include <bitset>
#include <thread>
#include <chrono>
#include <iomanip>

using eipScanner::SessionInfo;
using eipScanner::MessageRouter;
using eipScanner::ConnectionManager;
using eipScanner::DiscoveryManager;
using eipScanner::IdentityObject;
using eipScanner::cip::connectionManager::ConnectionParameters;
using eipScanner::cip::connectionManager::NetworkConnectionParams;
using namespace eipScanner::cip;
using namespace eipScanner::utils;

const char* generalStatusCodeToString(GeneralStatusCodes code) {
    switch (code) {
        case SUCCESS:                               return "SUCCESS: Service was successfully performed.";
        case CONNECTION_FAILURE:                    return "CONNECTION_FAILURE: A connection related service failed along the connection path.";
        case RESOURCE_UNAVAILABLE:                  return "RESOURCE_UNAVAILABLE: Resources needed for the object to perform the requested service were unavailable.";
        case INVALID_PARAMETER_VALUE:               return "INVALID_PARAMETER_VALUE: See CIPStatusCodes.InvalidParameter, which is the preferred value to use for this condition.";
        case PATH_SEGMENT_ERROR:                    return "PATH_SEGMENT_ERROR: The path segment identifier or the segment syntax was not understood by the processing node. Path processing shall stop when a path segment error is encountered.";
        case PATH_DESTINATION_UNKNOWN:              return "PATH_DESTINATION_UNKNOWN: The path is referencing an object class, instance, or structure element that is not known or is not contained in the processing node. Path processing shall stop when a path destination unknown error is encountered.";
        case PARTIAL_TRANSFER:                      return "PARTIAL_TRANSFER: Only part of the expected data was transferred.";
        case CONNECTION_LOST:                       return "CONNETION_LOST: The messaging connection was lost.";
        case SERVICE_NOT_SUPPORTED:                 return "SERVICE_NOT_SUPPORTED: The requested service was not implemented or was not defined for this object Class/Instance.";
    ;   case INVALID_ATTRIBUTE_VALUE:               return "INVALID_ATTRIBUTE_VALUE: Invalid attribute data detected.";
        case ATTRIBUTE_LIST_ERROR:                  return "ATTRIBUTE_LIST_ERROR: An attribute in the Get_Attribute_List or Set_Attribute_List response has a non-zero status.";
        case ALREADY_IN_REQUESTED_MODE_OR_STATE:    return "ALREADY_IN_REQUESTED_MODE_OR_STATE: The object is already in the mode/state being requested by the service.";
        case OBJECT_STATE_CONFLICT:                 return "OBJECT_STATE_CONFLICT: The object cannot perform the requested service in its current state/mode.";
        case OBJECT_ALREADY_EXISTS:                 return "OBJECT_ALREADY_EXISTS: The requested instance of object to be created already exists.";
        case ATTRIBUTE_NOT_SETTABLE:                return "ATTRIBUTE_NOT_SETTABLE: A request to modify a non-modifiable attribute was received.";
        case PRIVILEGE_VIOLATION:                   return "PRIVILEGE_VIOLATION: A permission/privilege check failed.";
        case DEVICE_STATE_CONFLICT:                 return "DEVICE_STATE_CONFLICT: The device's current mode/state prohibits the execution of the requested service.";
        case REPLY_DATA_TOO_LARGE:                  return "REPLY_DATA_TOO_LARGE: The data to be transmitted in the response buffer is larger than the allocated response buffer.";
        case FRAGMENTATION_OF_PRIMITIVE_VALUE:      return "FRAGMENTATION_OF_PRIMITIVE_VALUE: The service specified an operation that is going to fragment a primitive data value, i.e. half a REAL data type.";
        case NOT_ENOUGH_DATA:                       return "NOT_ENOUGH_DATA: The service did not supply enough data to perform the requested operation.";
        case ATTRIBUTE_NOT_SUPPORTED:               return "ATTRIBUTE_NOT_SUPPORTED: The attribute specified in the request is not supported.";
        case TOO_MUCH_DATA:                         return "TOO_MUCH_DATA: The service was supplied with more data than was expected.";
        case OBJECT_DOES_NOT_EXIST:                 return "OBJECT_DOES_NOT_EXIST: The object specified does not exist on the device.";
        case SVCFRAG_SEQNC_NOT_IN_PROGRESS:         return "SVCFRAG_SEQNC_NOT_IN_PROGRESS: The fragmentation sequence for this service is not currently active for this data.";
        case NO_STORED_ATTRIBUTE_DATA:              return "NO_STORED_ATTRIBUTE_DATA: The attribute data of this object was not saved prior to the requested service.";
        case STORE_OPERATION_FAILURE:               return "STORE_OPERATION_FAILURE: The attribute data of this object was not saved due to a failure following the attempt.";
        case ROUTING_FAILURE_REQUEST_SIZE:          return "ROUTING_FAILURE_REQUEST_SIZE: The service request packet was too large for transmission on a network in the path to the destination. The routing device was forced to abort the service.";
        case ROUTING_FAILURE_RESPONSE_SIZE:         return "ROUTING_FAILURE_RESPONSE_SIZE: The service response packet was too large for transmission on a network in the path from the destination. The routing device was forced to abort the service.";
        case MISSING_ATTRIBUTE_LIST_ENTRY:          return "MISSING_ATTRIBUTE_LIST_ENTRY: The service did not supply an attribute in a list of attributes that was needed by the service to perform the requested behavior.";
        case INVALID_ATTRIBUTE_LIST:                return "INVALID_ATTRIBUTE_LIST: The service is returning the list of attributes supplied with status information for those attributes that were invalid.";
        case EMBEDDED_SERVICE_ERROR:                return "EMBEDDED_SERVICE_ERROR: An embedded service resulted in an error.";
        case VENDOR_SPECIFIC:                       return "VENDOR_SPECIFIC: A vendor specific error has been encountered. The Additional Code Field of the Error Response defines the particular error encountered. Use of this General Error Code should only be performed when none of the Error Codes presented in this table or within an Object Class definition accurately reflect the error.";
        case INVALID_PARAMETER:                     return "INVALID_PARAMETER: A parameter associated with the request was invalid. This code is used when a parameter does not meet the requirements of this specification and/or the requirements defined in an Application Object Specification.";
        case WRITE_ONCE_WRITTEN:                    return "WRITE_ONCE_WRITTEN: An attempt was made to write to a write-once medium (e.g. WORM drive, PROM) that has already been written, or to modify a value that cannot be changed once established.";
        case INVALID_REPLY_RECEIVED:                return "INVALID_REPLY_RECEIVED: An invalid reply is received (e.g. reply service code does not match the request service code, or reply message is shorter than the minimum expected reply size). This status code can serve for other causes of invalid replies.";
        case KEY_FAILURE_IN_PATH:                   return "KEY_FAILURE_IN_PATH: The Key Segment that was included as the first segment in the path does not match the destination module. The object specific status shall indicate which part of the key check failed.";
        case PATH_SIZE_INVALID:                     return "PATH_SIZE_INVALID: The size of the path which was sent with the Service Request is either not large enough to allow the Request to be routed to an object or too much routing data was included.";
        case UNEXPECTED_ATTRIBUTE:                  return "UNEXPECTED_ATTRIBUTE: An attempt was made to set an attribute that is not able to be set at this time.";
        case INVALID_MEMBER_ID:                     return "INVALID_MEMBER_ID: The Member ID specified in the request does not exist in the specified Class/Instance/Attribute";
        case MEMBER_NOT_SETTABLE:                   return "MEMBER_NOT_SETTABLE: A request to modify a non-modifiable member was received";
        default:                                    return "UNKNOWN STATUS CODE";
    }
}

void printDeviceNetworkConfiguration(std::shared_ptr<SessionInfo> session) {
    MessageRouter messageRouter;
    MessageRouterResponse response;

    IdentityObject identityObject(1, session);
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
}

void rebootDevice(std::shared_ptr<SessionInfo> session) {
    std::cout << "===== Rebooting Device..." << std::endl;
    MessageRouter messageRouter;
    MessageRouterResponse response;
    IdentityObject identityObject(1, session);
    response = messageRouter.sendRequest(session, 0x05, EPath(identityObject.getClassId(),1));
    std::cout << generalStatusCodeToString(response.getGeneralStatusCode()) << std::endl;
}

void setupImplicitMessaging(std::shared_ptr<SessionInfo> session) {

    eipScanner::ConnectionManager connectionManager;

    connectionManager::ConnectionParameters parameters;

    //parameters.priorityTimeTick |= 0x10;    //Effect Unknown : defines priority = 1
    //parameters.priorityTimeTick |= 15;       //Effect Unknown : defines length of a tick in ms (=2^x)
    //parameters.timeoutTicks = 255;          //Effect Unknown : number of ticks

    //parameters.o2tNetworkConnectionId = 1;                                      //useless, defined by originator
    //parameters.t2oNetworkConnectionId = 2;                                       //useless, defined by target
    //parameters.connectionSerialNumber = 0xFFFF;                                           //uselesss, defined by static library counter

    //parameters.originatorVendorId = 333;                                                  //useless
    //parameters.originatorSerialNumber = 0x333;                                            //useless

    //parameters.connectionTimeoutMultiplier = 7; //Effect Unknown : =4*(x+1) (min=0, max=7)

    //output assembly (103)
    parameters.o2tRPI = 100000;                                                             //IMPORTANT (in microseconds)
    parameters.o2tNetworkConnectionParams |= NetworkConnectionParams::OWNED;                //IMPORTANT
    parameters.o2tNetworkConnectionParams |= NetworkConnectionParams::P2P;                  //IMPORTANT
    parameters.o2tNetworkConnectionParams |= NetworkConnectionParams::SCHEDULED_PRIORITY;   //IMPORTANT
    parameters.o2tNetworkConnectionParams |= 38;                                            //IMPORTANT
    parameters.o2tRealTimeFormat = true;                                                    //IMPORTANT

    //input assembly (113)
    parameters.t2oRPI = 100000;                                                             //IMPORTANT (in microseconds)
    parameters.t2oNetworkConnectionParams |= NetworkConnectionParams::OWNED;                //IMPORTANT
    parameters.t2oNetworkConnectionParams |= NetworkConnectionParams::P2P;
    parameters.t2oNetworkConnectionParams |= NetworkConnectionParams::SCHEDULED_PRIORITY;   //IMPORTANT
    parameters.t2oNetworkConnectionParams |= 38;                                            //IMPORTANT
    parameters.t2oRealTimeFormat = false;                                                   //IMPORTANT

    parameters.transportTypeTrigger |= NetworkConnectionParams::TRIG_CYCLIC;                //IMPORTANT
    parameters.transportTypeTrigger |= NetworkConnectionParams::CLASS1;                     //IMPORTANT

    //parameters.connectionPathSize = 8;                                                    //useless, automatically calculated from vector
    parameters.connectionPath = { 0x20, 0x04, 0x24, 0x05, 0x2C, 0x67, 0x2C, 0x71 };           //IMPORTANT

    auto io = connectionManager.forwardOpen(session, parameters);

    uint32_t PCTRLms = 0x130A00A8;  //read (0x13) parameter class 168 (0x00A8) attribute 10 (0x0A)
    uint32_t PVms = 0;              //parameter value to write (in case the command is write)
    uint16_t dmControl = 0;         //set operating state and operating mode
    uint32_t RefA32 = 0;            //operating mode specific value
    uint32_t RefB32 = 0;            //operating mode specific value
    uint32_t Ramp_v_acc = 0;        //acceleration ramp
    uint32_t Ramp_v_dec = 0;        //deceleration ramp
    uint32_t EthOptMapOut1 = 0;     //selectable parameter
    uint32_t EthOptMapOut2 = 0;     //selectable parameter
    uint32_t EthOptMapOut3 = 0;     //selectable parameter

    Buffer outputBuffer;
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

    std::vector<uint8_t> outputData(38, 0xFF);

    if (auto ptr = io.lock()) {
        std::cout << "FORWARD OPEN SUCCESS" << std::endl;
        std::cout << "Starting Implicit I/O Message Receiving And Sending" << std::endl;
        
        ptr->setDataToSend(std::vector<uint8_t>(outputBuffer.data()));
        
        ptr->setReceiveDataListener([](CipUdint realtimeHeader, CipUint sequence, const std::vector<uint8_t>& data) {
            //called each time the input assembly is received

            Buffer buffer(data);

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

            std::cout << "Motor Velocity: " << _v_act << std::endl;
        });

        ptr->setCloseListener([]() {
            std::cout << "CLOSED !!!" << std::endl;
         });
    }
    else {
        std::cout << "FORWARD OPEN FAILURE" << std::endl;
    }


    
    int count = 200;
    while (connectionManager.hasOpenConnections() && count-- > 0) {
        //each time this function gets executed the output assembly is sent
        connectionManager.handleConnections(std::chrono::milliseconds(100));
    }

    std::cout << "===== stopped implicit messaging" << std::endl;
    connectionManager.forwardClose(session, io);
    std::cout << "===== closed I/O socket" << std::endl;

}

void printDiscoveredDevices(const char* broadcastAddress, uint16_t port) {
    std::cout << "===== Discovering Ethernet/IP devices on the network " << broadcastAddress << std::endl;
    DiscoveryManager discoveryManager(broadcastAddress, port, std::chrono::seconds(1));
    std::vector<eipScanner::IdentityItem> devices = discoveryManager.discover();
    std::cout << "===== Found " << devices.size() << ((devices.size() == 1) ? "device :" : "devices :") << std::endl;
    for (eipScanner::IdentityItem& device : devices) {
        eipScanner::sockets::EndPoint& endpoint = device.socketAddress;
        eipScanner::IdentityObject& identityObject = device.identityObject;
        std::cout << "Product Name: " << identityObject.getProductName()
            << "\n = productCode: " << identityObject.getProductCode()
            << "\n = Revision: " << (uint32_t)identityObject.getRevision().getMajorRevision()
            << "." << (uint32_t)identityObject.getRevision().getMinorRevision()
            << "\n = SerialNumber: " << identityObject.getSerialNumber()
            << "\n = VendorID: " << identityObject.getVendorId()
            << "\n = Status: " << identityObject.getStatus()
            << "\n = Network Endpoint: " << endpoint.toString()
            << std::endl;
    }
}

int main() {
    Logger::setLogLevel(LogLevel::OFF);
    
    const char* broadcastAddress = "3.3.3.255";
    const char* deviceAddress = "3.3.3.103";
    uint16_t ethernetIpPort = 0xAF12;

    printDiscoveredDevices(broadcastAddress, ethernetIpPort);
    
    std::cout << "===== Starting Ethernet/IP Session with device at address " << deviceAddress << std::endl;
    std::shared_ptr<SessionInfo> session = std::make_shared<SessionInfo>(deviceAddress, ethernetIpPort);
    printDeviceNetworkConfiguration(session);
    setupImplicitMessaging(session);
    rebootDevice(session);
}

#endif