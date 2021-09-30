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
    return identity != nullptr && identity->state != EC_STATE_NONE;
}

bool EtherCatSlave::isOnline() {
    return isDetected() && !isStateNone() && EtherCatFieldbus::b_processRunning;
}

bool EtherCatSlave::isReady() {
    if (!isOnline() || !isStateOperational()) return false;
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
            explicitDeviceID = id;
            break;
    }
    if (!loadDeviceData(xml)) return Logger::warn("Could not read device data");
    return true;
}


const char* EtherCatSlave::getEtherCatStateChar() {
    if (!isDetected()) return "Offline";
    uint16_t stateWithoutErrorBit = identity->state & 0xF;
    switch (stateWithoutErrorBit) {
        case EC_STATE_NONE: return "No State";
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

//=========================== DEVICE EVENTS ==========================

void EtherCatSlave::pushEvent(const char* eventMessage, bool isError) {
    eventListMutex.lock();
    eventList.push_back(new Event(eventMessage, isError));
    eventListMutex.unlock();
}

void EtherCatSlave::pushEvent(uint16_t errorCode) {
    eventListMutex.lock();
    eventList.push_back(new Event(errorCode));
    eventListMutex.unlock();
}

void EtherCatSlave::clearEventList() {
    eventListMutex.lock();
    for (Event* event : eventList) delete event;
    eventList.clear();
    eventListMutex.unlock();
}

//===== Reading SDO Data

bool EtherCatSlave::readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& data) {
    EtherCatCoeData CoE_U8(index, subindex, EtherCatData::Type::UINT8_T);
    if (CoE_U8.read(getSlaveIndex())) {
        data = CoE_U8.getU8();
        return true;
    } else return false;
}
bool EtherCatSlave::readSDO_S8(uint16_t index, uint8_t subindex, int8_t& data) {
    EtherCatCoeData CoE_S8(index, subindex, EtherCatData::Type::INT8_T);
    if (CoE_S8.read(getSlaveIndex())) {
        data = CoE_S8.getS8();
        return true;
    } else return false;
}

bool EtherCatSlave::readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& data) {
    EtherCatCoeData CoE_U16(index, subindex, EtherCatData::Type::UINT16_T);
    if (CoE_U16.read(getSlaveIndex())) {
        data = CoE_U16.getU16();
        return true;
    } else return false;
}

bool EtherCatSlave::readSDO_S16(uint16_t index, uint8_t subindex, int16_t& data) {
    EtherCatCoeData CoE_S16(index, subindex, EtherCatData::Type::INT16_T);
    if (CoE_S16.read(getSlaveIndex())) {
        data = CoE_S16.getS16();
        return true;
    } else return false;
}

bool EtherCatSlave::readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& data) {
    EtherCatCoeData CoE_U32(index, subindex, EtherCatData::Type::UINT32_T);
    if (CoE_U32.read(getSlaveIndex())) {
        data = CoE_U32.getU32();
        return true;
    }
    else return false;
}

bool EtherCatSlave::readSDO_S32(uint16_t index, uint8_t subindex, int32_t& data) {
    EtherCatCoeData CoE_S32(index, subindex, EtherCatData::Type::INT32_T);
    if (CoE_S32.read(getSlaveIndex())) {
        data = CoE_S32.getS32();
        return true;
    } else return false;
}

bool EtherCatSlave::readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& data) {
    EtherCatCoeData CoE_U64(index, subindex, EtherCatData::Type::UINT64_T);
    if (CoE_U64.read(getSlaveIndex())) {
        data = CoE_U64.getU64();
        return true;
    } else return false;
}

bool EtherCatSlave::readSDO_S64(uint16_t index, uint8_t subindex, int64_t& data) {
    EtherCatCoeData CoE_S64(index, subindex, EtherCatData::Type::INT64_T);
    if (CoE_S64.read(getSlaveIndex())) {
        data = CoE_S64.getS64();
        return true;
    } else return false;
}

bool EtherCatSlave::readSDO_String(uint16_t index, uint8_t subindex, char* data) {
    EtherCatCoeData CoE_String(index, subindex, EtherCatData::Type::STRING);
    if (CoE_String.read(getSlaveIndex())) {
        strcpy(data, CoE_String.getString());
        return true;
    }
    else return false;
}

//===== Writing SDO Data

bool EtherCatSlave::writeSDO_U8(uint16_t index, uint8_t subindex, const uint8_t& data) {
    EtherCatCoeData CoE_U8(index, subindex);
    CoE_U8.setU8(data);
    return CoE_U8.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_S8(uint16_t index, uint8_t subindex, const int8_t& data) {
    EtherCatCoeData CoE_S8(index, subindex);
    CoE_S8.setS8(data);
    return CoE_S8.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_U16(uint16_t index, uint8_t subindex, const uint16_t& data) {
    EtherCatCoeData CoE_U16(index, subindex);
    CoE_U16.setU16(data);
    return CoE_U16.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_S16(uint16_t index, uint8_t subindex, const int16_t& data) {
    EtherCatCoeData CoE_S16(index, subindex);
    CoE_S16.setS16(data);
    return CoE_S16.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_U32(uint16_t index, uint8_t subindex, const uint32_t& data) {
    EtherCatCoeData CoE_U32(index, subindex);
    CoE_U32.setU32(data);
    return CoE_U32.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_S32(uint16_t index, uint8_t subindex, const int32_t& data) {
    EtherCatCoeData CoE_S32(index, subindex);
    CoE_S32.setS32(data);
    return CoE_S32.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_U64(uint16_t index, uint8_t subindex, const uint64_t& data) {
    EtherCatCoeData CoE_U64(index, subindex);
    CoE_U64.setU64(data);
    return CoE_U64.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_S64(uint16_t index, uint8_t subindex, const int64_t& data) {
    EtherCatCoeData CoE_S64(index, subindex);
    CoE_S64.setS64(data);
    return CoE_S64.write(getSlaveIndex());
}

bool EtherCatSlave::writeSDO_String(uint16_t index, uint8_t subindex, const char* data) {
    EtherCatCoeData CoE_String(index, subindex);
    CoE_String.setString(data);
    return CoE_String.write(getSlaveIndex());
}

