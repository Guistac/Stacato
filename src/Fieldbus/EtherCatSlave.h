#pragma once

#include <ethercat.h>

class EtherCatSlave{
public:

    virtual void configureStartupParameters() = 0;
    virtual void process(bool b_processDataValid) = 0;

    int slaveIndex;
    char customName[128];
    ec_slavet* identity;

    void setName(const char* name)  { strcpy(customName, name); }
    const char* getName()           { return customName; }
    const char* getDeviceName()     { return identity->name; };

    int getSlaveIndex()            { return slaveIndex; }
    int getManualAddress()          { return identity->aliasadr; }
    int getAssignedAddress()        { return identity->configadr; }

    bool isStateInit()              { return identity->state == EC_STATE_INIT; }
    bool isStatePreOperational()    { return identity->state == EC_STATE_PRE_OP; }
    bool isStateBootstrap()         { return identity->state == EC_STATE_BOOT; }
    bool isStateSafeOperational()   { return identity->state == EC_STATE_SAFE_OP; }
    bool isStateOperational()       { return identity->state == EC_STATE_OPERATIONAL; }


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