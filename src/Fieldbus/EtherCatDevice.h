#pragma once

#include <ethercat.h>

#include "NodeGraph/Device.h"

#include "Utilities/EtherCatPDO.h"

#include "Utilities/EtherCatData.h"

#include "Motion/Subdevice.h"

//classDeviceName is a static string used to identify the device class when creating a new instance for a specific device
//the static method is for use by the identifying method which will check all available device classes for a match
//the non static method is an override to see what the subclass name is from a base class reference or pointer
//the rest of the functions are the basic mandatory interface members for an ethercat device (config and process)
//Device that are matched against a device class will return true for isDeviceKnown()
//Unknown devices will not and will be of the base type EtherCatDevice

#define INTERFACE_DEFINITION(className, deviceName, manufacturerName, category)     public:                                                                                                 \
                                                                                    /*Node Functions*/                                                                                      \
                                                                                    virtual Node::Type getType() { return Node::Type::IODEVICE; }                                           \
                                                                                    virtual Device::Type getDeviceType() { return Device::Type::ETHERCAT_DEVICE; }                          \
														                            virtual const char * getNodeName() { return deviceName; }			                                    \
                                                                                    virtual const char * getNodeCategory() { return category; }			                                    \
                                                                                    virtual const char* getManufacturerName(){ return manufacturerName; }                                   \
                                                                                    virtual std::shared_ptr<Node> getNewNodeInstance() { return nullptr; }                                  \
														                            className() { setName(deviceName); }												                    \
                                                                                    virtual void assignIoData(){}                                                                           \
                                                                                    virtual void process(){}                                                                                \
                                                                                    virtual bool save(tinyxml2::XMLElement* xml);                                                           \
                                                                                    virtual bool load(tinyxml2::XMLElement* xml);                                                           \
                                                                                    /*Device Functions*/                                                                                    \
                                                                                    virtual bool isDetected();              /*checks generic ethercat status first*/                        \
                                                                                    virtual bool isOnline();                /*checks generic ethercat status first*/                        \
                                                                                    virtual bool isReady();                 /*checks generic ethercat status first*/                        \
                                                                                    virtual void enable(){}																			        \
                                                                                    virtual void disable(){}																			    \
                                                                                    virtual bool isEnabled(){ return false; }																\
                                                                                    virtual void readInputs(){}                                                                             \
                                                                                    virtual void prepareOutputs(){}                                                                         \
                                                                                    virtual void onConnection(){}                                                                           \
                                                                                    virtual void onDisconnection(){}                                                                        \
                                                                                    /*EtherCAT Device Functions*/                                                                           \
                                                                                    virtual bool isDeviceReady(){ return false; }                                                           \
                                                                                    virtual bool isSlaveKnown(){ return false; }                                                            \
                                                                                    virtual bool startupConfiguration(){ return true; }                                                     \
                                                                                    virtual void resetData(){}                                                                              \
                                                                                    virtual void deviceSpecificGui(){}                                                                      \
                                                                                    virtual bool saveDeviceData(tinyxml2::XMLElement* xml){ return true; }                                  \
                                                                                    virtual bool loadDeviceData(tinyxml2::XMLElement* xml){ return true; }                                  \
                                                                                    virtual std::shared_ptr<EtherCatDevice> getNewDeviceInstance() { return std::make_shared<className>(); } \

//All Slave Device Classes Need to Implement this Macro 
#define ETHERCAT_DEVICE_DEFINITION(className, deviceName, manufacturerName, category)   public:                                                                                                 \
                                                                                        /*Node Functions*/                                                                                      \
                                                                                        virtual Node::Type getType() { return Node::Type::IODEVICE; }                                           \
                                                                                        virtual Device::Type getDeviceType() { return Device::Type::ETHERCAT_DEVICE; }                          \
                                                                                        virtual const char* getNodeName() { return deviceName; }			                                    \
                                                                                        virtual const char * getNodeCategory() { return category; }			                                    \
                                                                                        virtual const char* getManufacturerName() { return manufacturerName; }                                  \
                                                                                        virtual std::shared_ptr<Node> getNewNodeInstance() { return nullptr; }                                  \
                                                                                        className(){ setName(deviceName); }                                                                     \
                                                                                        virtual void assignIoData();                                                                            \
                                                                                        /*DeviceNode Functions*/                                                                                \
                                                                                        virtual void enable();                  														        \
                                                                                        virtual void disable();                 															    \
                                                                                        virtual bool isEnabled();               																\
                                                                                        virtual void readInputs();                                                                              \
                                                                                        virtual void prepareOutputs();                                                                          \
                                                                                        virtual void onConnection();                                                                            \
                                                                                        virtual void onDisconnection();                                                                         \
                                                                                        /*EtherCAT Device Functions*/                                                                           \
                                                                                        virtual bool isDeviceReady();                                                                           \
                                                                                        virtual bool isSlaveKnown(){ return true; }                                                             \
                                                                                        virtual bool startupConfiguration();                                                                    \
                                                                                        virtual void resetData();                                                                               \
                                                                                        virtual void deviceSpecificGui();                                                                       \
                                                                                        virtual bool saveDeviceData(tinyxml2::XMLElement* xml);                                                 \
                                                                                        virtual bool loadDeviceData(tinyxml2::XMLElement* xml);                                                 \
                                                                                        virtual std::shared_ptr<EtherCatDevice> getNewDeviceInstance() { return std::make_shared<className>(); } \

#define RETURN_ETHERCAT_DEVICE_IF_TYPE_MATCHING(name, className) if(strcmp(name, className::getNodeNameStatic()) == 0) return std::make_shared<className>()



struct EtherCatDeviceIdentification {
    enum class Type {
        STATION_ALIAS,
        EXPLICIT_DEVICE_ID
    };
    Type type;
    const char displayName[64];
    const char saveName[64];
};
extern std::vector<EtherCatDeviceIdentification> indentificationTypes;
std::vector<EtherCatDeviceIdentification>& getIdentificationTypes();
EtherCatDeviceIdentification* getIdentificationType(const char *);
EtherCatDeviceIdentification* getIdentificationType(EtherCatDeviceIdentification::Type t);



class EtherCatDevice : public Device {
public:

    //===== Base EtherCAT device
    //serves as device interface and as default device type for unknow devices
    INTERFACE_DEFINITION(EtherCatDevice, "Unknown Device", "Unknown manufacturer", "No Category");

    ec_slavet* identity = nullptr;
    EtherCatDeviceIdentification::Type identificationType = EtherCatDeviceIdentification::Type::STATION_ALIAS;
    int slaveIndex = -1;
    uint16_t stationAlias = 0;
    uint16_t explicitDeviceID = 0;
    bool b_supportsExplicitDeviceID = false;

    //basic info
    uint32_t getManufacturer() { return identity->eep_man; }
    uint32_t getID() { return identity->eep_id; }
    uint32_t getRevision() { return identity->eep_rev; }

    //addresses
    int getSlaveIndex() { return slaveIndex; }
    uint16_t getAssignedAddress() { return identity->configadr; } //configured station address

    //state machine
    bool isStateNone()              { return (identity->state & 0xF) == EC_STATE_NONE; }
    bool isStateInit()              { return (identity->state & 0xF) == EC_STATE_INIT; }
    bool isStatePreOperational()    { return (identity->state & 0xF) == EC_STATE_PRE_OP; }
    bool isStateBootstrap()         { return (identity->state & 0xF) == EC_STATE_BOOT; }
    bool isStateSafeOperational()   { return (identity->state & 0xF) == EC_STATE_SAFE_OP; }
    bool isStateOperational()       { return (identity->state & 0xF) == EC_STATE_OPERATIONAL; }

    const char* getEtherCatStateChar();
    bool hasStateError();
    uint16_t previousState = -1;

    //public display of raw pdo data
    EtherCatPdoAssignement txPdoAssignement;
    EtherCatPdoAssignement rxPdoAssignement;

    //Mailbox types
    bool isCoeSupported() { return identity->mbx_proto & ECT_MBXPROT_COE; }
    bool isFoeSupported() { return identity->mbx_proto & ECT_MBXPROT_FOE; }
    bool isEoESupported() { return identity->mbx_proto & ECT_MBXPROT_EOE; }
    bool isSoESupported() { return identity->mbx_proto & ECT_MBXPROT_SOE; }

    //Coe support details
    bool supportsCoE_SDO()          { return identity->CoEdetails & ECT_COEDET_SDO; }
    bool supportsCoE_SDOinfo()      { return identity->CoEdetails & ECT_COEDET_SDOINFO; }
    bool supportsCoE_PDOassign()    { return identity->CoEdetails & ECT_COEDET_PDOASSIGN; }
    bool supportsCoE_PDOconfig()    { return identity->CoEdetails & ECT_COEDET_PDOCONFIG; }
    bool supportsCoE_upload()       { return identity->CoEdetails & ECT_COEDET_UPLOAD; }
    bool supportsCoE_SDOCA()        { return identity->CoEdetails & ECT_COEDET_SDOCA; }

    //===== EVENTS =====

    struct Event {
        Event(const char* eventMessage, bool isError) : b_isError(isError) { strcpy(message, eventMessage); }
        Event(uint16_t errorCode) : b_isError(true) { sprintf(message, "Error Code %X", errorCode); }
        std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char message[128];
        bool b_isError;
    };
    std::mutex eventListMutex;
    std::vector<Event*> eventList;
    void pushEvent(const char* errorMessage, bool isError);
    void pushEvent(uint16_t errorCode);
    void clearEventList();

    EtherCatRegisterData uploadRegisterData = EtherCatRegisterData("uploadData", 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::BINARY);
    EtherCatRegisterData downloadRegisterData = EtherCatRegisterData("downloadData", 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::BINARY);
    EtherCatCoeData uploadCoeData = EtherCatCoeData("uploadData", 0x0, 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::DECIMAL);
    EtherCatCoeData downloadCoeData = EtherCatCoeData("uploadData", 0x0, 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::DECIMAL);
    EtherCatEepromData uploadEepromData = EtherCatEepromData("uploadData", 0x0, DataFormat::Type::HEXADECIMAL);
    EtherCatEepromData downloadEepromData = EtherCatEepromData("downloadData", 0x0, DataFormat::Type::HEXADECIMAL);

    //===== Upload Status Variables =====

    struct DataTransferState {
        enum class State {
            NO_TRANSFER,
            TRANSFERRING,
            SUCCEEDED,
            SAVING,
            SAVED,
            FAILED
        };
        State state;
        char displayName[64];
    };
    static std::vector<DataTransferState> dataTransferStates;
    DataTransferState* getDataTransferState(DataTransferState::State s);

    //======== GUI ========

    virtual void nodeSpecificGui();
    void genericInfoGui();
    void pdoDataGui();
    void generalGui();
    void identificationGui();
    void sendReceiveCanOpenGui();
    void sendReceiveEtherCatRegisterGui();
    void sendReceiveEepromGui();
    void eventListGui();

    //=====Reading and Writing SDO Data

    int maxSdoReadAttempts = 4;
    int maxSdoWriteAttemps = 4;

    bool readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& data);
    bool readSDO_S8(uint16_t index, uint8_t subindex, int8_t& data);
    bool readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& data);
    bool readSDO_S16(uint16_t index, uint8_t subindex, int16_t& data);
    bool readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& data);
    bool readSDO_S32(uint16_t index, uint8_t subindex, int32_t& data);
    bool readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& data);
    bool readSDO_S64(uint16_t index, uint8_t subindex, int64_t& data);
    bool readSDO_String(uint16_t index, uint8_t subindex, char* data);

    bool writeSDO_U8(uint16_t index, uint8_t subindex, const uint8_t& data);
    bool writeSDO_S8(uint16_t index, uint8_t subindex, const int8_t& data);
    bool writeSDO_U16(uint16_t index, uint8_t subindex, const uint16_t& data);
    bool writeSDO_S16(uint16_t index, uint8_t subindex, const int16_t& data);
    bool writeSDO_U32(uint16_t index, uint8_t subindex, const uint32_t& data);
    bool writeSDO_S32(uint16_t index, uint8_t subindex, const int32_t& data);
    bool writeSDO_U64(uint16_t index, uint8_t subindex, const uint64_t& data);
    bool writeSDO_S64(uint16_t index, uint8_t subindex, const int64_t& data);
    bool writeSDO_String(uint16_t index, uint8_t subindex, const char* data);

};