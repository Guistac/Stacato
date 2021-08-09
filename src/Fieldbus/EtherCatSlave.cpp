#include "EtherCatSlave.h"

#include <iostream>

void EtherCatSlave::startupConfiguration() {}

void EtherCatSlave::process(bool b_processDataValid) {}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, uint8_t& data) {
    int size = 1;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, int8_t& data) {
    int size = 1;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, uint16_t& data) {
    int size = 2;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, int16_t& data) {
    int size = 2;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, uint32_t& data) {
    int size = 4;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, int32_t& data) {
    int size = 4;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, uint64_t& data) {
    int size = 8;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::readSDO(uint16_t index, uint8_t subindex, int64_t& data) {
    int size = 8;
    for (int i = 0; i < maxSdoReadAttempts; i++) if (ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

//=====Writing SDO Data

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, uint8_t& data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 1, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, int8_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 1, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, uint16_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 2, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, int16_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 2, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, uint32_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 4, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, int32_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 4, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, uint64_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 8, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}

bool EtherCatSlave::writeSDO(uint16_t index, uint8_t subindex, int64_t data) {
    for (int i = 0; i < maxSdoWriteAttemps; i++) if (ec_SDOwrite(slaveIndex, index, subindex, false, 8, &data, EC_TIMEOUTSAFE) == 1) return true;
    return false;
}
