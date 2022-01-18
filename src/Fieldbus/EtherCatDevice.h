#pragma once

#include <ethercat.h>

#include "NodeGraph/Device.h"

#include "Utilities/EtherCatPDO.h"

#include "Utilities/EtherCatData.h"

#include "Motion/SubDevice.h"

//classDeviceName is a static string used to identify the device class when creating a new instance for a specific device
//the static method is for use by the identifying method which will check all available device classes for a match
//the non static method is an override to see what the subclass name is from a base class reference or pointer
//the rest of the functions are the basic mandatory interface members for an ethercat device (config and process)
//Device that are matched against a device class will return true for isDeviceKnown()
//Unknown devices will not and will be of the base type EtherCatDevice

#define DEFINE_ETHERCAT_DEVICE_INTERFACE(className, EtherCatName, displayName, saveName, manufacturerName, category) public:\
	DEFINE_DEVICE_NODE(className, displayName, saveName, Device::Type::ETHERCAT_DEVICE, category)\
	/*EtherCat Device Specific*/\
	virtual const char* getManufacturerName(){ return manufacturerName; }\
	virtual const char* getEtherCatName(){ return EtherCatName; }\
	virtual bool isSlaveKnown(){ return false; }\
	virtual bool startupConfiguration(){ return true; }\
	virtual bool saveDeviceData(tinyxml2::XMLElement* xml){ return true; }\
	virtual bool loadDeviceData(tinyxml2::XMLElement* xml){ return true; }\
	virtual void deviceSpecificGui(){}\

#define DEFINE_ETHERCAT_DEVICE(className, EtherCatName, displayName, saveName, manufacturerName, category) public:\
	DEFINE_NODE(className, displayName, saveName, Node::Type::IODEVICE, category)\
	virtual Device::Type getDeviceType() { return Device::Type::ETHERCAT_DEVICE; }\
	virtual void onConnection();\
	virtual void onDisconnection();\
	virtual void readInputs();\
	virtual void prepareOutputs();\
	/*EtherCat Device Specific*/\
	virtual const char* getManufacturerName() { return manufacturerName; }\
	virtual const char* getEtherCatName(){ return EtherCatName; }\
	virtual bool isSlaveKnown(){ return true; }\
	virtual bool startupConfiguration();\
	virtual bool saveDeviceData(tinyxml2::XMLElement* xml);\
	virtual bool loadDeviceData(tinyxml2::XMLElement* xml);\
	virtual void deviceSpecificGui();\

namespace tinyxml2{ class XMLElement; }

class EtherCatDevice : public Device {
public:
	
	enum class IdentificationType{
		STATION_ALIAS,
		EXPLICIT_DEVICE_ID
	};

    //===== Base EtherCAT device
    //serves as device interface and as default device type for unknow devices
    DEFINE_ETHERCAT_DEVICE_INTERFACE(EtherCatDevice, "Unknown Device", "Unknown Device", "UnknownDevice", "Unknown manufacturer", "No Category");

    ec_slavet* identity = nullptr;
    IdentificationType identificationType = IdentificationType::STATION_ALIAS;
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
        Event(const char* eventMessage, bool isError) : b_isError(isError) {strcpy(message, eventMessage); }
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

    //===== Upload Status Variables =====

	enum class DataTransferState{
		NO_TRANSFER,
		TRANSFERRING,
		SUCCEEDED,
		SAVING,
		SAVED,
		FAILED
	};

    //======== GUI ========

	virtual void nodeSpecificGui();
    void genericInfoGui();
    void pdoDataGui();
    void generalGui();
    void identificationGui();
    void sendReceiveCanOpenGui();
    void sendReceiveEtherCatRegisterGui();
    void sendReceiveSiiGui();
    void sendReceiveEeprom();
    void eventListGui();

    //=====Reading and Writing SDO Data

    bool readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& data);
    bool readSDO_S8(uint16_t index, uint8_t subindex, int8_t& data);
    bool readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& data);
    bool readSDO_S16(uint16_t index, uint8_t subindex, int16_t& data);
    bool readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& data);
    bool readSDO_S32(uint16_t index, uint8_t subindex, int32_t& data);
    bool readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& data);
    bool readSDO_S64(uint16_t index, uint8_t subindex, int64_t& data);
    bool readSDO_String(uint16_t index, uint8_t subindex, char* data, int bufferSize);

    bool writeSDO_U8(uint16_t index, uint8_t subindex, const uint8_t& data);
    bool writeSDO_S8(uint16_t index, uint8_t subindex, const int8_t& data);
    bool writeSDO_U16(uint16_t index, uint8_t subindex, const uint16_t& data);
    bool writeSDO_S16(uint16_t index, uint8_t subindex, const int16_t& data);
    bool writeSDO_U32(uint16_t index, uint8_t subindex, const uint32_t& data);
    bool writeSDO_S32(uint16_t index, uint8_t subindex, const int32_t& data);
    bool writeSDO_U64(uint16_t index, uint8_t subindex, const uint64_t& data);
    bool writeSDO_S64(uint16_t index, uint8_t subindex, const int64_t& data);
    bool writeSDO_String(uint16_t index, uint8_t subindex, const char* data);

    //===== Reading And Writing Data

    EtherCatRegisterData uploadRegisterData = EtherCatRegisterData("uploadData", 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::BINARY);
    EtherCatRegisterData downloadRegisterData = EtherCatRegisterData("downloadData", 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::BINARY);
    EtherCatCoeData uploadCoeData = EtherCatCoeData("uploadData", 0x0, 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::DECIMAL);
    EtherCatCoeData downloadCoeData = EtherCatCoeData("uploadData", 0x0, 0x0, EtherCatData::Type::UINT16_T, DataFormat::Type::DECIMAL);
    EtherCatEepromData uploadEepromData = EtherCatEepromData("uploadData", 0x0, DataFormat::Type::HEXADECIMAL);
    EtherCatEepromData downloadEepromData = EtherCatEepromData("downloadData", 0x0, DataFormat::Type::HEXADECIMAL);

	uint16_t downloadedALStatuscode = 0x0;
	void downloadALStatusCode();
	DataTransferState AlStatusCodeDownloadState = DataTransferState::NO_TRANSFER;
	
    bool downloadEEPROM(char* fileName);
    DataTransferState eepromDownloadState = DataTransferState::NO_TRANSFER;
    char eepromSaveFilePath[512];

    bool flashEEPROM(char* fileName);
    DataTransferState eepromFlashState = DataTransferState::NO_TRANSFER;
    char eepromLoadFilePath[512];

    bool setStationAlias(uint16_t alias);
    uint16_t stationAliasToolValue = 0;
    DataTransferState stationAliasAssignState = DataTransferState::NO_TRANSFER;
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
};

#define EtherCatIdentificationTypeStrings \
	{EtherCatDevice::IdentificationType::STATION_ALIAS, "Station Alias", "StationAlias"},\
	{EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID, "Explicit Device ID", "ExplicitDeviceID"}\

DEFINE_ENUMERATOR(EtherCatDevice::IdentificationType, EtherCatIdentificationTypeStrings)

//====================== DATA TRANSFER STATE =================================

#define EtherCatDataTransferStateStrings \
	{EtherCatDevice::DataTransferState::NO_TRANSFER, ""},\
	{EtherCatDevice::DataTransferState::TRANSFERRING, "Transferring..."},\
	{EtherCatDevice::DataTransferState::SUCCEEDED, "Transfer Succeeded"},\
	{EtherCatDevice::DataTransferState::SAVING, "Saving to Device Memory..."},\
	{EtherCatDevice::DataTransferState::SAVED, "Saved to Device Memory"},\
	{EtherCatDevice::DataTransferState::FAILED, "Transfer Failed"}\

DEFINE_ENUMERATOR(EtherCatDevice::DataTransferState, EtherCatDataTransferStateStrings)
