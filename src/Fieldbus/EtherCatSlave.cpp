#include <pch.h>

#include "EtherCatSlave.h"
#include "EtherCatFieldbus.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <tinyxml2.h>

std::vector<EtherCatSlaveIdentification> identificationTypes = {
    {EtherCatSlaveIdentification::Type::STATION_ALIAS, "Station Alias", "StationAlias"},
    {EtherCatSlaveIdentification::Type::EXPLICIT_DEVICE_ID, "Explicit Device ID", "ExplicitDeviceID"}
};

std::vector<EtherCatSlaveIdentification>& getIdentificationTypes() {
    return identificationTypes;
}
EtherCatSlaveIdentification* getIdentificationType(const char* saveName) {
    for (EtherCatSlaveIdentification& identification : identificationTypes) {
        if (strcmp(saveName, identification.saveName) == 0) return &identification;
    }
    return nullptr;
}
EtherCatSlaveIdentification* getIdentificationType(EtherCatSlaveIdentification::Type t) {
    for (EtherCatSlaveIdentification& identification : identificationTypes) {
        if (t == identification.type) return &identification;
    }
    return nullptr;
}

/*
bool EtherCatSlave::getPDOMapping(EtherCatPDO& pdo, uint16_t pdoIndex, const char* pdoDescription) {

    pdo.modules.clear();
    uint8_t moduleCount;
    Logger::trace("reading entry count of index 0x{:X}", pdoIndex);
    if (!readSDO(pdoIndex, 0x0, moduleCount)) return false;
    Logger::debug("***** {} Mapping Module count: {}", pdoDescription, (int)moduleCount);

    for (int i = 1; i <= moduleCount; i++) {
        EtherCatPDOModule module;
        Logger::trace("reading index of pdo mapping module {}", i);
        if (!readSDO(pdoIndex, i, module.index)) return false;
        uint8_t entryCount;
        Logger::trace("reading entry count of pdo mapping module {}", i);
        if (!readSDO(module.index, 0x0, entryCount)) return false;

        Logger::debug("  *** {} Mapping Module [{}] : 0x{:X} ({} entries)", pdoDescription, i, module.index, entryCount);

        for (int j = 1; j <= entryCount; j++) {
            EtherCatPDOEntry entry;
            uint32_t entryData;
            Logger::trace("reading index of pdo mapping module entry {}", j);
            if (!readSDO(module.index, j, entryData)) return false;
            entry.index = entryData >> 16;
            entry.subindex = entryData >> 8;
            entry.byteCount = entryData;
            module.entries.push_back(entry);
            Logger::debug("    * entry [{}] : index: 0x{:X}  subindex: 0x{:X}  size: {}", j, entry.index, entry.subindex, entry.byteCount);
        }

        pdo.modules.push_back(module);
    }

    return true;
}
*/

bool EtherCatSlave::isDetected() {
    return !isStateOffline();
}

bool EtherCatSlave::isOnline() {
    return !isStateOffline() && EtherCatFieldbus::b_processRunning;
}

bool EtherCatSlave::hasError() {
    if (!isSlaveKnown()) return true;
    if (identity == nullptr) return false;
    if (hasDeviceError()) return true;
    if (identity != nullptr && hasStateError()) return true;
    return false;
}

const char* EtherCatSlave::getErrorString() {
    if (hasError()) {
        if (!isSlaveKnown()) return "Unknown Device";
        if (identity != nullptr && hasStateError()) {
            static char stateError[128];
            sprintf(stateError, "EtherCAT State Machine Error: %s", getStateChar());
            return stateError;
        }
        if (hasDeviceError()) return getDeviceErrorString();
    }
    else {
        return "No Error";
    }
}

void EtherCatSlave::clearError() {
    clearDeviceError();
}

bool EtherCatSlave::isReady() {
    if (identity == nullptr) return false;
    else if (!isStateOperational()) return false;
    else return isDeviceReady();
}

bool EtherCatSlave::save(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    XMLElement* identificationXML = xml->InsertNewChildElement("Identification");
    identificationXML->SetAttribute("Type", getIdentificationType(identificationType)->saveName);
    switch (identificationType) {
        case EtherCatSlaveIdentification::Type::STATION_ALIAS:
            identificationXML->SetAttribute("StationAlias", stationAlias);
            break;
        case EtherCatSlaveIdentification::Type::EXPLICIT_DEVICE_ID:
            identificationXML->SetAttribute("ExplicitDeviceID", explicitDeviceID);
            break;
    }
    saveDeviceData(xml);
    return true;
}

bool EtherCatSlave::load(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    
    XMLElement* identificationXML = xml->FirstChildElement("Identification");
    if (!identificationXML) return Logger::warn("Could not load identification attribute");
    const char* identificationTypeString;
    if (identificationXML->QueryStringAttribute("Type", &identificationTypeString) != XML_SUCCESS) return Logger::warn("Could not load identification Type");
    if (getIdentificationType(identificationTypeString) == nullptr) return Logger::warn("Could not read identification Type");
    identificationType = getIdentificationType(identificationTypeString)->type;
    switch (identificationType) {
        case EtherCatSlaveIdentification::Type::STATION_ALIAS:
            int alias;
            if (identificationXML->QueryIntAttribute("StationAlias", &alias) != XML_SUCCESS) return Logger::warn("Could not load Station Alias");
            stationAlias = alias;
            break;
        case EtherCatSlaveIdentification::Type::EXPLICIT_DEVICE_ID:
            int id;
            if (identificationXML->QueryIntAttribute("ExplicitDeviceID", &id) != XML_SUCCESS) return Logger::warn("Could not load Explicit Device ID");
            break;
    }
    if (!loadDeviceData(xml)) return Logger::warn("Could not read device data");
    return true;
}


const char* EtherCatSlave::getStateChar() {
    if (identity == nullptr) return "No Status";
    uint16_t stateWithoutErrorBit = identity->state & 0xF;
    switch (stateWithoutErrorBit) {
        case EC_STATE_NONE: return "No Status";
        case EC_STATE_INIT: return "Init";
        case EC_STATE_PRE_OP: return "Pre-Operational";
        case EC_STATE_BOOT: return "Boot";
        case EC_STATE_SAFE_OP: return "Safe-Operational";
        case EC_STATE_OPERATIONAL: return "Operational";
        default: return "Unknown State";
    }
}

bool EtherCatSlave::hasStateError() {
    return identity->state & EC_STATE_ERROR;
}


bool EtherCatSlave::matches(std::shared_ptr<EtherCatSlave> otherSlave) {
    //two matching slaves should have the same class device name (offline copy of the original device name)
    //the same station alias / manual address
    if (strcmp(getNodeName(), otherSlave->getNodeName()) != 0) return false;

    if (identificationType == otherSlave->identificationType) {
        switch (identificationType) {
        case EtherCatSlaveIdentification::Type::STATION_ALIAS:
            return stationAlias == otherSlave->stationAlias;
        case EtherCatSlaveIdentification::Type::EXPLICIT_DEVICE_ID:
            return explicitDeviceID == otherSlave->explicitDeviceID;
        }
    }
    return true;
}

void EtherCatSlave::compareNewState() {
    uint16_t previousStateWithoutError = previousState & 0xF;
    uint16_t currentStateWithoutError = identity->state & 0xF;
    bool previousError = previousState & EC_STATE_ERROR;
    bool currentError = identity->state & EC_STATE_ERROR;
    if (currentStateWithoutError != previousStateWithoutError || previousError != currentError) {
        Logger::warn("{} state changed to {} {}", getName(), getStateChar(), currentError ? "(Error)" : "");
        switch (currentStateWithoutError) {
        case EC_STATE_NONE: break;
        case EC_STATE_INIT: break;
        case EC_STATE_PRE_OP: break;
        case EC_STATE_BOOT: break;
        case EC_STATE_SAFE_OP: break;
        case EC_STATE_OPERATIONAL: break;
        default: break;
        }
    }
}


//===== Reading SDO Data

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

//===== Writing SDO Data

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
