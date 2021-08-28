#include "EtherCatSlave.h"

#include <iostream>

#include <imgui.h>
#include <imgui_internal.h>

bool EtherCatSlave::getPDOMapping() {

    //read the content of SM2 (0x1C12) & SM3 (0x1C13)
    //to retrieve modules and pdo entries

    static auto readPdoMapping = [this](EtherCatPDO& pdo, uint16_t pdoIndex, const char* pdoDescription) -> bool {
  
        pdo.modules.clear();
        uint8_t moduleCount;
        if (!readSDO(pdoIndex, 0x0, moduleCount)) return false;
        std::cout << "***** " << pdoDescription << " module count: " << std::dec << (int)moduleCount << std::endl;

        for (int i = 1; i <= moduleCount; i++) {
            EtherCatPDOModule module;
            if (!readSDO(pdoIndex, i, module.index)) return false;
            uint8_t entryCount;
            if (!readSDO(module.index, 0x0, entryCount)) return false;

            std::cout << "  *** " << pdoDescription << " module [" << i << "] : 0x" << std::hex << module.index << " (" << std::dec << (int)entryCount << " entries)" << std::endl;

            for (int j = 1; j <= entryCount; j++) {
                EtherCatPDOEntry entry;
                uint32_t entryData;
                if (!readSDO(module.index, j, entryData)) return false;
                entry.index = entryData >> 16;
                entry.subindex = entryData >> 8;
                entry.byteCount = entryData;
                module.entries.push_back(entry);
                std::cout << "    * entry [" << std::dec << (int)j << "] : index: 0x" << std::hex << entry.index << " subindex: 0x" << std::hex << (int)entry.subindex << " size: " << std::dec << (int)entry.byteCount << std::endl;
            }

            pdo.modules.push_back(module);
        }

        return true;
    };

    uint16_t RxPDOIndex = 0x1C12;
    uint16_t TxPDOIndex = 0x1C13;

    if (!readPdoMapping(rxPdo, RxPDOIndex, "RxPDO")) return false;
    if (!readPdoMapping(txPdo, TxPDOIndex, "TxPDO")) return false;

    return true;
}

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
