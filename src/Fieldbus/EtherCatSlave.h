#pragma once

#include <ethercat.h>

#include "NodeGraph/DeviceNode.h"

#include "Utilities/EtherCatPDO.h"

#include "Utilities/EtherCatData.h"

#include "Motion/Subdevice.h"

//classDeviceName is a static string used to identify the device class when creating a new instance for a specific device
//the static method is for use by the identifying method which will check all available device classes for a match
//the non static method is an override to see what the subclass name is from a base class reference or pointer
//the rest of the functions are the basic mandatory interface members for an ethercat device (config and process)
//Device that are matched against a device class will return true for isDeviceKnown()
//Unknown devices will not and will be of the base type EtherCatSlave

#define INTERFACE_DEFINITION(className, deviceName, manufacturerName, category)     public:                                                                                                 \
                                                                                    /*ioNode Functions*/                                                                                    \
                                                                                    virtual NodeType getType() { return NodeType::IODEVICE; }                                               \
                                                                                    virtual DeviceType getDeviceType() { return DeviceType::ETHERCATSLAVE; }                                \
														                            virtual const char * getNodeName() { return deviceName; }			                                    \
                                                                                    virtual const char * getNodeCategory() { return category; }			                                    \
                                                                                    virtual const char* getManufacturerName(){ return manufacturerName; }                                   \
                                                                                    virtual std::shared_ptr<ioNode> getNewNodeInstance() { return nullptr; }                                \
														                            className() { setName(deviceName); }												                    \
                                                                                    virtual void assignIoData(){}                                                                           \
                                                                                    virtual void process(){}                                                                                \
                                                                                    virtual bool save(tinyxml2::XMLElement* xml);                                                           \
                                                                                    virtual bool load(tinyxml2::XMLElement* xml);                                                           \
                                                                                    /*DeviceNode Functions*/                                                                                \
                                                                                    virtual bool isDetected();              /*checks generic ethercat status first*/                        \
                                                                                    virtual bool isOnline();                /*checks generic ethercat status first*/                        \
                                                                                    virtual bool hasError();            	/*checks generic ethercat status first*/                        \
                                                                                    virtual const char* getErrorString();   /*checks generic ethercat status first*/                        \
                                                                                    virtual void clearError();              /*checks generic ethercat status first*/                        \
                                                                                    virtual bool isReady();                 /*checks generic ethercat status first*/                        \
                                                                                    virtual void enable(){}																			        \
                                                                                    virtual void disable(){}																			    \
                                                                                    virtual bool isEnabled(){ return false; }																\
                                                                                    /*EtherCAT Slave Functions*/                                                                            \
                                                                                    virtual bool hasDeviceError(){ return false; }                                                          \
                                                                                    virtual const char* getDeviceErrorString(){ return ""; }                                                \
                                                                                    virtual void clearDeviceError(){}                                                                       \
                                                                                    virtual bool isDeviceReady(){ return false; }                                                           \
                                                                                    virtual bool isSlaveKnown(){ return false; }                                                            \
                                                                                    virtual bool startupConfiguration(){ return true; }                                                     \
                                                                                    virtual void readInputs(){}                                                                             \
                                                                                    virtual void prepareOutputs(){}                                                                         \
                                                                                    virtual void deviceSpecificGui(){}                                                                      \
                                                                                    virtual bool saveDeviceData(tinyxml2::XMLElement* xml){ return true; }                                  \
                                                                                    virtual bool loadDeviceData(tinyxml2::XMLElement* xml){ return true; }                                  \
                                                                                    virtual std::shared_ptr<EtherCatSlave> getNewDeviceInstance() { return std::make_shared<className>(); } \

//All Slave Device Classes Need to Implement this Macro 
#define SLAVE_DEFINITION(className, deviceName, manufacturerName, category) public:                                                                                                 \
                                                                            /*ioNode Functions*/                                                                                    \
                                                                            virtual NodeType getType() { return NodeType::IODEVICE; }                                               \
                                                                            virtual DeviceType getDeviceType() { return DeviceType::ETHERCATSLAVE; }                                \
                                                                            virtual const char* getNodeName() { return deviceName; }			                                    \
                                                                            virtual const char * getNodeCategory() { return category; }			                                    \
                                                                            virtual const char* getManufacturerName() { return manufacturerName; }                                  \
                                                                            virtual std::shared_ptr<ioNode> getNewNodeInstance() { return nullptr; }                                \
                                                                            className(){ setName(deviceName); }                                                                     \
                                                                            virtual void assignIoData();                                                                            \
                                                                            virtual void process();                                                                                 \
                                                                            /*DeviceNode Functions*/                                                                                \
                                                                            virtual void enable();                  														        \
                                                                            virtual void disable();                 															    \
                                                                            virtual bool isEnabled();               																\
                                                                            /*EtherCAT Slave Functions*/                                                                            \
                                                                            virtual bool hasDeviceError();                                                                          \
                                                                            virtual const char* getDeviceErrorString();                                                             \
                                                                            virtual void clearDeviceError();                                                                        \
                                                                            virtual bool isDeviceReady();                                                                           \
                                                                            virtual bool isSlaveKnown(){ return true; }                                                             \
                                                                            virtual bool startupConfiguration();                                                                    \
                                                                            virtual void readInputs();                                                                              \
                                                                            virtual void prepareOutputs();                                                                          \
                                                                            virtual void deviceSpecificGui();                                                                       \
                                                                            virtual bool saveDeviceData(tinyxml2::XMLElement* xml);                                                 \
                                                                            virtual bool loadDeviceData(tinyxml2::XMLElement* xml);                                                 \
                                                                            virtual std::shared_ptr<EtherCatSlave> getNewDeviceInstance() { return std::make_shared<className>(); } \

#define RETURN_SLAVE_IF_TYPE_MATCHING(name, className) if(strcmp(name, className::getNodeNameStatic()) == 0) return std::make_shared<className>()

struct EtherCatSlaveIdentification {
    enum class Type {
        STATION_ALIAS,
        EXPLICIT_DEVICE_ID
    };
    Type type;
    const char displayName[64];
    const char saveName[64];
};

extern std::vector<EtherCatSlaveIdentification> indentificationTypes;
std::vector<EtherCatSlaveIdentification>& getIdentificationTypes();
EtherCatSlaveIdentification* getIdentificationType(const char *);
EtherCatSlaveIdentification* getIdentificationType(EtherCatSlaveIdentification::Type t);

class EtherCatSlave : public DeviceNode {
public:

    //===== Base EtherCAT device
    //serves as device interface and as default device type for unknow devices
    INTERFACE_DEFINITION(EtherCatSlave, "Unknown Device", "Unknown manufacturer", "No Category");

    int slaveIndex = -1;
    ec_slavet* identity = nullptr;

    EtherCatSlaveIdentification::Type identificationType = EtherCatSlaveIdentification::Type::STATION_ALIAS;
    uint16_t stationAlias = 0;
    uint16_t explicitDeviceID = 0;

    //public display of raw pdo data
    EtherCatPdoAssignement txPdoAssignement;
    EtherCatPdoAssignement rxPdoAssignement;

    //basic info
    uint32_t getManufacturer() { return identity->eep_man; }
    uint32_t getID() { return identity->eep_id; }
    uint32_t getRevision() { return identity->eep_rev; }

    bool matches(std::shared_ptr<EtherCatSlave> otherSlave);

    //addresses
    int getSlaveIndex() { return slaveIndex; }
    uint16_t getStationAlias() { return stationAlias; }  //configured station alias address
    uint16_t getAssignedAddress() { return identity->configadr; } //configured station address

    //state machine
    bool isStateOffline()           { return (identity == nullptr) || (identity->state & 0xF) == EC_STATE_NONE; }
    bool isStateInit()              { return (identity->state & 0xF) == EC_STATE_INIT; }
    bool isStatePreOperational()    { return (identity->state & 0xF) == EC_STATE_PRE_OP; }
    bool isStateBootstrap()         { return (identity->state & 0xF) == EC_STATE_BOOT; }
    bool isStateSafeOperational()   { return (identity->state & 0xF) == EC_STATE_SAFE_OP; }
    bool isStateOperational()       { return (identity->state & 0xF) == EC_STATE_OPERATIONAL; }

    const char* getStateChar();
    bool hasStateError();

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


    bool b_mapped = false;

    uint16_t previousState = -1;
    void saveCurrentState() { previousState = identity->state; }
    void compareNewState();

    virtual void nodeSpecificGui();
    void genericInfoGui();
    void pdoDataGui();
    void generalGui();
    void sendReceiveCanOpenGui();
    void sendReceiveEtherCatRegisterGui();
    void sendReceiveEepromGui();

    EtherCatRegisterData uploadRegisterData = EtherCatRegisterData("uploadData", 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::BINARY);
    EtherCatRegisterData downloadRegisterData = EtherCatRegisterData("downloadData", 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::BINARY);
    EtherCatCoeData uploadCoeData = EtherCatCoeData("uploadData", 0x0, 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::DECIMAL);
    EtherCatCoeData downloadCoeData = EtherCatCoeData("uploadData", 0x0, 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::DECIMAL);
    EtherCatEepromData uploadEepromData = EtherCatEepromData("uploadData", 0x0, DataFormat::Type::HEXADECIMAL);
    EtherCatEepromData downloadEepromData = EtherCatEepromData("downloadData", 0x0, DataFormat::Type::HEXADECIMAL);


    //=====Reading and Writing SDO Data

    int maxSdoReadAttempts = 4;
    int maxSdoWriteAttemps = 4;

    bool readSDO(uint16_t index, uint8_t subindex, uint8_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, int8_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, uint16_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, int16_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, uint32_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, int32_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, uint64_t& data);
    bool readSDO(uint16_t index, uint8_t subindex, int64_t& data);

    bool writeSDO(uint16_t index, uint8_t subindex, uint8_t& data);
    bool writeSDO(uint16_t index, uint8_t subindex, int8_t data);
    bool writeSDO(uint16_t index, uint8_t subindex, uint16_t data);
    bool writeSDO(uint16_t index, uint8_t subindex, int16_t data);
    bool writeSDO(uint16_t index, uint8_t subindex, uint32_t data);
    bool writeSDO(uint16_t index, uint8_t subindex, int32_t data);
    bool writeSDO(uint16_t index, uint8_t subindex, uint64_t data);
    bool writeSDO(uint16_t index, uint8_t subindex, int64_t data);

private:

};



//TODO:
//add vector of inputs and outputs
//add default gui containing basic information about the slave