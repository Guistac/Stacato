#include <pch.h>

#include "EtherCatDevice.h"
#include "EtherCatFieldbus.h"

#include <imgui.h>
#include <tinyxml2.h>

void EtherCatDevice::initialize(){}

bool EtherCatDevice::isDetected() {
    return identity != nullptr && identity->state != EC_STATE_NONE;
}

bool EtherCatDevice::isConnected() {
    return isDetected() && !isStateNone() && EtherCatFieldbus::isCyclicExchangeActive();;
}

void EtherCatDevice::readInputs(){}
void EtherCatDevice::prepareOutputs(){}
void EtherCatDevice::onConnection(){}
void EtherCatDevice::onDisconnection(){}

bool EtherCatDevice::save(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;
    XMLElement* identificationXML = xml->InsertNewChildElement("Identification");
    identificationXML->SetAttribute("Type", Enumerator::getSaveString(identificationType));
    switch (identificationType) {
		case EtherCatDevice::IdentificationType::STATION_ALIAS:
            identificationXML->SetAttribute("StationAlias", stationAlias);
            break;
		case EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID:
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
    if (!Enumerator::isValidSaveName<EtherCatDevice::IdentificationType>(identificationTypeString)) return Logger::warn("Could not read identification Type");
	identificationType = Enumerator::getEnumeratorFromSaveString<EtherCatDevice::IdentificationType>(identificationTypeString);
    switch (identificationType) {
		case EtherCatDevice::IdentificationType::STATION_ALIAS:
            int alias;
            if (identificationXML->QueryIntAttribute("StationAlias", &alias) != XML_SUCCESS) return Logger::warn("Could not load Station Alias");
            stationAlias = alias;
            break;
		case EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID:
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
	if(isError) Logger::error("{} : {}", getName(), eventMessage);
	else Logger::info("{} : {}", getName(), eventMessage);
}

void EtherCatDevice::pushEvent(uint16_t errorCode) {
    eventListMutex.lock();
    eventList.push_back(new Event(errorCode));
    eventListMutex.unlock();
	Logger::error("{} : Error Code 0x{:X}", getName(), errorCode);
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

//================== AL Status Code Download =======================

void EtherCatDevice::downloadALStatusCode(){
	std::thread AlStatusDownloader([this](){
		AlStatusCodeDownloadState = DataTransferState::TRANSFERRING;
		int wc = ec_FPRD(getAssignedAddress(), 0x134, 2, &downloadedALStatuscode, EC_TIMEOUTSAFE);
		if(wc == 1) AlStatusCodeDownloadState = DataTransferState::SUCCEEDED;
		else AlStatusCodeDownloadState = DataTransferState::FAILED;
	});
	AlStatusDownloader.detach();
}


void EtherCatDevice::downloadTransmissionErrors(){
	std::thread transmissionErrorDownloader([this](){
		transmissionErrorDownloadState = DataTransferState::TRANSFERRING;
		
		ec_FPRD(getAssignedAddress(), 0x0300, 1, &transmissionErrors.A_Frame, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0302, 1, &transmissionErrors.B_Frame, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0304, 1, &transmissionErrors.C_Frame, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0306, 1, &transmissionErrors.D_Frame, EC_TIMEOUTSAFE);
		
		ec_FPRD(getAssignedAddress(), 0x0301, 1, &transmissionErrors.A_Physical, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0303, 1, &transmissionErrors.B_Physical, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0305, 1, &transmissionErrors.C_Physical, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0307, 1, &transmissionErrors.D_Physical, EC_TIMEOUTSAFE);
		
		ec_FPRD(getAssignedAddress(), 0x0308, 1, &transmissionErrors.A_Forwarded, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0309, 1, &transmissionErrors.B_Forwarded, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x030A, 1, &transmissionErrors.C_Forwarded, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x030B, 1, &transmissionErrors.D_Forwarded, EC_TIMEOUTSAFE);
		
		ec_FPRD(getAssignedAddress(), 0x030C, 1, &transmissionErrors.processingUnit, EC_TIMEOUTSAFE);
		
		ec_FPRD(getAssignedAddress(), 0x030D, 1, &transmissionErrors.processDataInterface, EC_TIMEOUTSAFE);
		
		ec_FPRD(getAssignedAddress(), 0x0310, 1, &transmissionErrors.A_LostLinks, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0311, 2, &transmissionErrors.B_LostLinks, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0312, 3, &transmissionErrors.C_LostLinks, EC_TIMEOUTSAFE);
		ec_FPRD(getAssignedAddress(), 0x0313, 4, &transmissionErrors.D_LostLinks, EC_TIMEOUTSAFE);
		
		transmissionErrorDownloadState = DataTransferState::SUCCEEDED;
	});
	transmissionErrorDownloader.detach();
}


void EtherCatDevice::resetTransmissionErrors(){
	std::thread transmissionErrorResetter([this](){
		resetTransmissionErrorsState = DataTransferState::TRANSFERRING;
		
		uint16_t resetRxErrors = 0;
		ec_FPWR(getAssignedAddress(), 0x0300, 2, &resetRxErrors, EC_TIMEOUTSAFE);
		
		uint8_t resetFwErrors = 0;
		ec_FPWR(getAssignedAddress(), 0x0308, 1, &resetFwErrors, EC_TIMEOUTSAFE);
		
		uint8_t resetProcessingUnitErrors = 0;
		ec_FPWR(getAssignedAddress(), 0x030C, 1, &resetProcessingUnitErrors, EC_TIMEOUTSAFE);
		
		uint8_t resetPdiErrors = 0;
		ec_FPWR(getAssignedAddress(), 0x030D, 1, &resetPdiErrors, EC_TIMEOUTSAFE);
		
		uint8_t resetLostLinks = 0;
		ec_FPWR(getAssignedAddress(), 0x0310, 1, &resetLostLinks, EC_TIMEOUTSAFE);
		
		transmissionErrors.A_Frame = 0;
		transmissionErrors.A_Physical = 0;
		transmissionErrors.A_Forwarded = 0;
		transmissionErrors.A_LostLinks = 0;
		transmissionErrors.B_Frame = 0;
		transmissionErrors.B_Physical = 0;
		transmissionErrors.B_Forwarded = 0;
		transmissionErrors.B_LostLinks = 0;
		transmissionErrors.C_Frame = 0;
		transmissionErrors.C_Physical = 0;
		transmissionErrors.C_Forwarded = 0;
		transmissionErrors.C_LostLinks = 0;
		transmissionErrors.D_Frame = 0;
		transmissionErrors.D_Physical = 0;
		transmissionErrors.D_Forwarded = 0;
		transmissionErrors.D_LostLinks = 0;
		transmissionErrors.processingUnit = 0;
		transmissionErrors.processDataInterface = 0;
		
		resetTransmissionErrorsState = DataTransferState::SUCCEEDED;
	});
	transmissionErrorResetter.detach();
}

