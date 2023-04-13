#include <pch.h>

#include "EtherCatDevice.h"
#include "EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatError.h"

#include <imgui.h>
#include <tinyxml2.h>

bool EtherCatDevice::isDetected() {
    return identity != nullptr && identity->state != EC_STATE_NONE;
}

bool EtherCatDevice::isConnected() {
    return isDetected() && !isStateNone() && EtherCatFieldbus::isRunning();
}

void EtherCatDevice::readInputs(){}
void EtherCatDevice::writeOutputs(){}
void EtherCatDevice::onConnection(){}
void EtherCatDevice::onDisconnection(){}

bool EtherCatDevice::onSerialization() {
	bool success = true;
	success &= Node::onSerialization();
	Serializable identificationData;
	identificationData.setSaveString("Identification");
	success &= identificationData.serializeIntoParent(this);
	switch(identificationType){
		case IdentificationType::STATION_ALIAS:
			success &= identificationData.serializeAttribute("Type", "StationAlias");
			success &= identificationData.serializeAttribute("Address", stationAlias);
			break;
		case IdentificationType::EXPLICIT_DEVICE_ID:
			success &= identificationData.serializeAttribute("Type", "ExplicitDeviceID");
			success &= identificationData.serializeAttribute("Address", explicitDeviceID);
			break;
	}
	return success;
}

bool EtherCatDevice::onDeserialization() {
	bool success = true;
	success &= Node::onDeserialization();
	Serializable identificationData;
	identificationData.setSaveString("Identification");
	success &= identificationData.deserializeFromParent(this);
	std::string identificationTypeString;
	success &= identificationData.deserializeAttribute("Type", identificationTypeString);
	if(identificationTypeString == "StationAlias"){
		int stationAliasAddress;
		success &= identificationData.deserializeAttribute("Address", stationAliasAddress);
		stationAlias = stationAliasAddress;
	}
	else if(identificationTypeString == "ExplicitDeviceID"){
		int stationAliasIDAddress;
		success &= identificationData.deserializeAttribute("Address", stationAliasIDAddress);
		explicitDeviceID = stationAliasIDAddress;
	}
	else success = false;
	return success;
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
	if(isError) Logger::warn("{} : {}", getName(), eventMessage);
	else Logger::trace("{} : {}", getName(), eventMessage);
}

void EtherCatDevice::pushEvent(uint16_t errorCode) {
    eventListMutex.lock();
    eventList.push_back(new Event(errorCode));
    eventListMutex.unlock();
	Logger::warn("{} : Error Code 0x{:X}", getName(), errorCode);
}

void EtherCatDevice::clearEventList() {
    eventListMutex.lock();
    for (Event* event : eventList) delete event;
    eventList.clear();
    eventListMutex.unlock();
}

//===== Read SDO Data

static void logSDOreadResult(int workingCounter, uint16_t objectIndex, uint8_t objectSubindex, std::string objectName){
	if(workingCounter > 0) return;
	if(objectName.empty()) Logger::error("Failed to read SDO 0x{:x}.{:x}", objectIndex, objectSubindex);
	else Logger::error("Failed to read SDO '{}' 0x{:x}.{:x}", objectName, objectIndex, objectSubindex);
	while(EtherCatError::hasError()) EtherCatError::logError();
}

bool EtherCatDevice::readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& data, std::string objectName) {
	int size = 1;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}
bool EtherCatDevice::readSDO_S8(uint16_t index, uint8_t subindex, int8_t& data, std::string objectName) {
	int size = 1;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& data, std::string objectName) {
	int size = 2;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_S16(uint16_t index, uint8_t subindex, int16_t& data, std::string objectName) {
	int size = 2;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& data, std::string objectName) {
	int size = 4;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_S32(uint16_t index, uint8_t subindex, int32_t& data, std::string objectName) {
	int size = 4;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& data, std::string objectName) {
	int size = 8;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_S64(uint16_t index, uint8_t subindex, int64_t& data, std::string objectName) {
	int size = 8;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, &data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::readSDO_String(uint16_t index, uint8_t subindex, char* data, int bufferSize, std::string objectName) {
	int size = bufferSize;
	int wkc = ec_SDOread(slaveIndex, index, subindex, false, &size, (void*)data, EC_TIMEOUTRXM);
	logSDOreadResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

//===== Write SDO Data

static void logSDOwriteResult(int workingCounter, uint16_t objectIndex, uint8_t objectSubindex, std::string objectName){
	if(workingCounter > 0) return;
	if(objectName.empty()) Logger::warn("Failed to write SDO 0x{:x}.{:x}", objectIndex, objectSubindex);
	else Logger::warn("Failed to write SDO '{}' 0x{:x}.{:x}", objectName, objectIndex, objectSubindex);
	while(EtherCatError::hasError()) EtherCatError::logError();
}

bool EtherCatDevice::writeSDO_U8(uint16_t index, uint8_t subindex, const uint8_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 1, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_S8(uint16_t index, uint8_t subindex, const int8_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 1, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_U16(uint16_t index, uint8_t subindex, const uint16_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 2, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_S16(uint16_t index, uint8_t subindex, const int16_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 2, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_U32(uint16_t index, uint8_t subindex, const uint32_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 4, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_S32(uint16_t index, uint8_t subindex, const int32_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 4, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_U64(uint16_t index, uint8_t subindex, const uint64_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 8, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_S64(uint16_t index, uint8_t subindex, const int64_t& data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, 8, (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
}

bool EtherCatDevice::writeSDO_String(uint16_t index, uint8_t subindex, const char* data, std::string objectName) {
	int wkc = ec_SDOwrite(slaveIndex, index, subindex, false, strlen(data), (void*)&data, EC_TIMEOUTRXM);
	logSDOwriteResult(wkc, index, subindex, objectName);
	return wkc == 1;
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

void EtherCatDevice::downloadErrorCounters(){
	ec_FPRD(getAssignedAddress(), 0x0300, 1, &errorCounters.portErrors[0].frameRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0302, 1, &errorCounters.portErrors[1].frameRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0304, 1, &errorCounters.portErrors[2].frameRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0306, 1, &errorCounters.portErrors[3].frameRxErrors, EC_TIMEOUTSAFE);
	
	ec_FPRD(getAssignedAddress(), 0x0301, 1, &errorCounters.portErrors[0].physicalRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0303, 1, &errorCounters.portErrors[1].physicalRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0305, 1, &errorCounters.portErrors[2].physicalRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0307, 1, &errorCounters.portErrors[3].physicalRxErrors, EC_TIMEOUTSAFE);
	
	ec_FPRD(getAssignedAddress(), 0x0308, 1, &errorCounters.portErrors[0].forwardedRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0309, 1, &errorCounters.portErrors[1].forwardedRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x030A, 1, &errorCounters.portErrors[2].forwardedRxErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x030B, 1, &errorCounters.portErrors[3].forwardedRxErrors, EC_TIMEOUTSAFE);
	
	ec_FPRD(getAssignedAddress(), 0x0310, 1, &errorCounters.portErrors[0].lostLinks, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0311, 2, &errorCounters.portErrors[1].lostLinks, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0312, 3, &errorCounters.portErrors[2].lostLinks, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x0313, 4, &errorCounters.portErrors[3].lostLinks, EC_TIMEOUTSAFE);
	
	ec_FPRD(getAssignedAddress(), 0x030C, 1, &errorCounters.processingUnitErrors, EC_TIMEOUTSAFE);
	ec_FPRD(getAssignedAddress(), 0x030D, 1, &errorCounters.processDataInterfaceErrors, EC_TIMEOUTSAFE);
}


void EtherCatDevice::resetErrorCounters(){
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
	
	for(int i = 0; i < 4; i++){
		errorCounters.portErrors[i].frameRxErrors = 0;
		errorCounters.portErrors[i].physicalRxErrors = 0;
		errorCounters.portErrors[i].forwardedRxErrors = 0;
		errorCounters.portErrors[i].lostLinks = 0;
	}
	errorCounters.processingUnitErrors = 0;
	errorCounters.processDataInterfaceErrors = 0;
}
