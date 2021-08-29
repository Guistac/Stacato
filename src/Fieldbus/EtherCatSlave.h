#pragma once

#include <ethercat.h>

#include <vector>
#include <cstring>

#include "EtherCatDataTypes.h"
#include "EtherCatPDO.h"

//classDeviceName is a static string used to identify the device class when creating a new instance for a specific device
//the static method is for use by the identifying method which will check all available device classes for a match
//the non static method is an override to see what the subclass name is from a base class reference or pointer
//the rest of the functions are the basic mandatory interface members for an ethercat device (config and process)
//Device that are matched against a device class will return true for isDeviceKnown()
//Unknown devices will not and will be of the base type EtherCatSlave

#define INTERFACE_DEFINITION(className, deviceName) const char * classDeviceName = deviceName;	                        \
											        static const char * getStaticClassDeviceName(){	                    \
												        static className singleton;				                        \
												        return singleton.classDeviceName;		                        \
											        }											                        \
                                                    virtual const char* getClassDeviceName() { return classDeviceName; }\
                                                    virtual bool isDeviceKnown(){ return false; }                       \
                                                    virtual bool startupConfiguration(){ return true; }                 \
                                                    virtual void process(bool b_processDataValid){}                     \
                                                    virtual void deviceSpecificGui(){}                                  \

//All Slave Device Classes Need to Implement this Macro 
#define SLAVE_DEFINITION(className, deviceName) className();                                                        \
                                                const char * classDeviceName = deviceName;	                        \
											    static const char * getStaticClassDeviceName(){	                    \
												    static className singleton;				                        \
												    return singleton.classDeviceName;		                        \
											    }											                        \
                                                virtual const char* getClassDeviceName() { return classDeviceName; }\
                                                virtual bool isDeviceKnown(){ return true; }                        \
                                                virtual bool startupConfiguration();                                \
                                                virtual void process(bool b_processDataValid);                      \
                                                virtual void deviceSpecificGui();                                   \
                                        

#define RETURN_SLAVE_IF_TYPE_MATCHING(name, className) if(strcmp(name, className::getStaticClassDeviceName()) == 0) return std::make_shared<className>()

class EtherCatSlave {
public:      

    //===== Base EtherCAT device
    //serves as device interface and as default device type for unknow devices
    INTERFACE_DEFINITION(EtherCatSlave, "Unknown Device")

    int slaveIndex = -1;
    char customName[128];
    ec_slavet* identity;

    //serves as a public and labeled way of accessing input and output data
    std::vector<EtherCatData*> outputData;
    std::vector<EtherCatData*> inputData;

    void setName(const char* name)  { strcpy(customName, name); }
    const char* getName()           { return customName; }
    const char* getDeviceName()     { return identity->name; }

    int getSlaveIndex()             { return slaveIndex; }
    int getManualAddress()          { return identity->aliasadr; }  //configured station alias address
    int getAssignedAddress()        { return identity->configadr; } //configured station address

    bool isStateInit()              { return identity->state == EC_STATE_INIT; }
    bool isStatePreOperational()    { return identity->state == EC_STATE_PRE_OP; }
    bool isStateBootstrap()         { return identity->state == EC_STATE_BOOT; }
    bool isStateSafeOperational()   { return identity->state == EC_STATE_SAFE_OP; }
    bool isStateOperational()       { return identity->state == EC_STATE_OPERATIONAL; }

    bool isCoeSupported() { return identity->CoEdetails > 0; }
    bool isFoeSupported() { return identity->FoEdetails > 0; }
    bool isEoESupported() { return identity->EoEdetails > 0; }
    bool isSoESupported() { return identity->SoEdetails > 0; }

    bool b_mapped = false;

    bool getPDOMapping(EtherCatPDO& pdo, uint16_t index, const char* pdoName);
    EtherCatPDO txPdo;
    EtherCatPDO rxPdo;

    void gui();
    void genericInfoGui();
    void ioDataGui();

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
};



//TODO:
//add vector of inputs and outputs
//add default gui containing basic information about the slave