#include <pch.h>

#include "EtherCatDevice.h"
#include "EtherCatFieldbus.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <tinyxml2.h>

std::vector<EtherCatDeviceIdentification> identificationTypes = {
    {EtherCatDeviceIdentification::Type::STATION_ALIAS, "Station Alias", "StationAlias"},
    {EtherCatDeviceIdentification::Type::EXPLICIT_DEVICE_ID, "Explicit Device ID", "ExplicitDeviceID"}
};

std::vector<EtherCatDeviceIdentification>& getIdentificationTypes() {
    return identificationTypes;
}
EtherCatDeviceIdentification* getIdentificationType(const char* saveName) {
    for (EtherCatDeviceIdentification& identification : identificationTypes) {
        if (strcmp(saveName, identification.saveName) == 0) return &identification;
    }
    return nullptr;
}
EtherCatDeviceIdentification* getIdentificationType(EtherCatDeviceIdentification::Type t) {
    for (EtherCatDeviceIdentification& identification : identificationTypes) {
        if (t == identification.type) return &identification;
    }
    return nullptr;
}

/*
bool EtherCatDevice::getPDOMapping(EtherCatPDO& pdo, uint16_t pdoIndex, const char* pdoDescription) {

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

bool EtherCatDevice::isDetected() {
    return identity != nullptr && identity->state != EC_STATE_NONE;
}

bool EtherCatDevice::isOnline() {
    return isDetected() && !isStateNone() && EtherCatFieldbus::isCyclicExchangeActive();;
}

bool EtherCatDevice::isReady() {
    if (!isOnline() || !isStateOperational()) return false;
    else return isDeviceReady();
}

bool EtherCatDevice::save(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    XMLElement* identificationXML = xml->InsertNewChildElement("Identification");
    identificationXML->SetAttribute("Type", getIdentificationType(identificationType)->saveName);
    switch (identificationType) {
        case EtherCatDeviceIdentification::Type::STATION_ALIAS:
            identificationXML->SetAttribute("StationAlias", stationAlias);
            break;
        case EtherCatDeviceIdentification::Type::EXPLICIT_DEVICE_ID:
            identificationXML->SetAttribute("ExplicitDeviceID", explicitDeviceID);
            break;
    }
    saveDeviceData(xml);
    return true;
}

bool EtherCatDevice::load(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    
    XMLElement* identificationXML = xml->FirstChildElement("Identification");
    if (!identificationXML) return Logger::warn("Could not load identification attribute");
    const char* identificationTypeString;
    if (identificationXML->QueryStringAttribute("Type", &identificationTypeString) != XML_SUCCESS) return Logger::warn("Could not load identification Type");
    if (getIdentificationType(identificationTypeString) == nullptr) return Logger::warn("Could not read identification Type");
    identificationType = getIdentificationType(identificationTypeString)->type;
    switch (identificationType) {
        case EtherCatDeviceIdentification::Type::STATION_ALIAS:
            int alias;
            if (identificationXML->QueryIntAttribute("StationAlias", &alias) != XML_SUCCESS) return Logger::warn("Could not load Station Alias");
            stationAlias = alias;
            break;
        case EtherCatDeviceIdentification::Type::EXPLICIT_DEVICE_ID:
            int id;
            if (identificationXML->QueryIntAttribute("ExplicitDeviceID", &id) != XML_SUCCESS) return Logger::warn("Could not load Explicit Device ID");
            explicitDeviceID = id;
            break;
    }
    if (!loadDeviceData(xml)) return Logger::warn("Could not read device data");
    return true;
}


const char* EtherCatDevice::getEtherCatStateChar() {
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

bool EtherCatDevice::hasStateError() {
    return identity->state & EC_STATE_ERROR;
}

//=========================== DEVICE EVENTS ==========================

void EtherCatDevice::pushEvent(const char* eventMessage, bool isError) {
    eventListMutex.lock();
    eventList.push_back(new Event(eventMessage, isError));
    eventListMutex.unlock();
}

void EtherCatDevice::pushEvent(uint16_t errorCode) {
    eventListMutex.lock();
    eventList.push_back(new Event(errorCode));
    eventListMutex.unlock();
}

void EtherCatDevice::clearEventList() {
    eventListMutex.lock();
    for (Event* event : eventList) delete event;
    eventList.clear();
    eventListMutex.unlock();
}

//===== Reading SDO Data

bool EtherCatDevice::readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& data) {
    return CanOpen::readSDO_U8(index, subindex, data, getSlaveIndex());
}
bool EtherCatDevice::readSDO_S8(uint16_t index, uint8_t subindex, int8_t& data) {
    return CanOpen::readSDO_S8(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& data) {
    return CanOpen::readSDO_U16(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_S16(uint16_t index, uint8_t subindex, int16_t& data) {
    return CanOpen::readSDO_S16(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& data) {
    return CanOpen::readSDO_U32(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_S32(uint16_t index, uint8_t subindex, int32_t& data) {
    return CanOpen::readSDO_S32(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& data) {
    return CanOpen::readSDO_U64(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_S64(uint16_t index, uint8_t subindex, int64_t& data) {
    return CanOpen::readSDO_S64(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::readSDO_String(uint16_t index, uint8_t subindex, char* data, int bufferSize) {
    return CanOpen::readSDO_String(index, subindex, data, bufferSize, getSlaveIndex());
}

//===== Writing SDO Data

bool EtherCatDevice::writeSDO_U8(uint16_t index, uint8_t subindex, const uint8_t& data) {
    return CanOpen::writeSDO_U8(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_S8(uint16_t index, uint8_t subindex, const int8_t& data) {
    return CanOpen::writeSDO_S8(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_U16(uint16_t index, uint8_t subindex, const uint16_t& data) {
    return CanOpen::writeSDO_U16(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_S16(uint16_t index, uint8_t subindex, const int16_t& data) {
    return CanOpen::writeSDO_S16(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_U32(uint16_t index, uint8_t subindex, const uint32_t& data) {
    return CanOpen::writeSDO_U32(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_S32(uint16_t index, uint8_t subindex, const int32_t& data) {
    return CanOpen::writeSDO_U32(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_U64(uint16_t index, uint8_t subindex, const uint64_t& data) {
    return CanOpen::writeSDO_U64(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_S64(uint16_t index, uint8_t subindex, const int64_t& data) {
    return CanOpen::writeSDO_S64(index, subindex, data, getSlaveIndex());
}

bool EtherCatDevice::writeSDO_String(uint16_t index, uint8_t subindex, const char* data) {
    return CanOpen::writeSDO_String(index, subindex, data, getSlaveIndex());
}



//====================== DATA TRANSFER STATE =================================

std::vector<EtherCatDevice::DataTransferState> EtherCatDevice::dataTransferStates = {
    {DataTransferState::State::NO_TRANSFER, ""},
    {DataTransferState::State::TRANSFERRING, "Transferring..."},
    {DataTransferState::State::SUCCEEDED, "Transfer Succeeded"},
    {DataTransferState::State::SAVING, "Saving to Device Memory..."},
    {DataTransferState::State::SAVED, "Saved to Device Memory"},
    {DataTransferState::State::FAILED, "Transfer Failed"}
};

EtherCatDevice::DataTransferState* EtherCatDevice::getDataTransferState(DataTransferState::State s) {
    for (DataTransferState& state : dataTransferStates) {
        if (s == state.state) return &state;
    }
    return nullptr;
}
