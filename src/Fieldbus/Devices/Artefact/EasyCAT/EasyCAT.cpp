#include <pch.h>
#include "EasyCAT.h"

#include "Fieldbus/EtherCatFieldbus.h"

bool EasyCAT::isDeviceReady() { return false;  }
void EasyCAT::enable() {}
void EasyCAT::disable() {}
bool EasyCAT::isEnabled() { return false; }
void EasyCAT::onDisconnection() {}

void EasyCAT::assignIoData() {
	addIoData(byteOut);
	addIoData(shortOut);
	addIoData(longOut);
	addIoData(longLongOut);

	addIoData(byteIn);
	addIoData(shortIn);
	addIoData(longIn);
	addIoData(longLongIn);

	txPdoAssignement.addNewModule(0x0);
	txPdoAssignement.addEntry(0x0, 0x0, 1, "byteIn", &ui8_byteIn);
	txPdoAssignement.addEntry(0x0, 0x0, 2, "shortIn", &ui16_shortIn);
	txPdoAssignement.addEntry(0x0, 0x0, 4, "longIn", &ui32_longIn);
	txPdoAssignement.addEntry(0x0, 0x0, 8, "longLongIn", &ui64_longLongIn);
										   
	rxPdoAssignement.addNewModule(0x0);	  
	rxPdoAssignement.addEntry(0x0, 0x0, 1, "byteOut", &ui8_byteOut);
	rxPdoAssignement.addEntry(0x0, 0x0, 2, "shortOut", &ui16_shortOut);
	rxPdoAssignement.addEntry(0x0, 0x0, 4, "longOut", &ui32_longOut);
	rxPdoAssignement.addEntry(0x0, 0x0, 8, "longLongOut", &ui64_longLongOut);
}

bool EasyCAT::startupConfiguration() { 
	uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0L;
	uint32_t sync0offset_nanoseconds = 0;
	ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);
	return true;
}

void EasyCAT::readInputs() {
	uint8_t* inputData = identity->inputs;

	ui64_longLongIn = (uint64_t)inputData[0] |
		(uint64_t)inputData[1] << 8 |
		(uint64_t)inputData[2] << 16 |
		(uint64_t)inputData[3] << 24 |
		(uint64_t)inputData[4] << 32 |
		(uint64_t)inputData[5] << 40 |
		(uint64_t)inputData[6] << 48 |
		(uint64_t)inputData[7] << 56;
	longLongIn->set((long long int)ui64_longLongIn);

	ui32_longIn = inputData[8] |
		inputData[9] << 8 |
		inputData[10] << 16 |
		inputData[11] << 24;
	longIn->set((long long int)ui32_longIn);

	ui16_shortIn = inputData[12] |
		inputData[13] << 8;
	shortIn->set((long long int)ui16_shortIn);

	ui8_byteIn = inputData[14];
	byteIn->set((long long int)ui8_byteIn);
}

void EasyCAT::prepareOutputs() {
	uint8_t* outputData = identity->outputs;

	ui64_longLongOut = longLongOut->getInteger();
	outputData[0] =		(ui64_longLongOut >> 0) & 0xFF;
	outputData[1] =		(ui64_longLongOut >> 8) & 0xFF;
	outputData[2] =		(ui64_longLongOut >> 16) & 0xFF;
	outputData[3] =		(ui64_longLongOut >> 24) & 0xFF;
	outputData[4] =		(ui64_longLongOut >> 32) & 0xFF;
	outputData[5] =		(ui64_longLongOut >> 40) & 0xFF;
	outputData[6] =		(ui64_longLongOut >> 48) & 0xFF;
	outputData[7] =		(ui64_longLongOut >> 56) & 0xFF;

	ui32_longOut = longOut->getInteger();
	outputData[8] =		(ui32_longOut >> 0) & 0xFF;
	outputData[9] =		(ui32_longOut >> 8) & 0xFF;
	outputData[10] =	(ui32_longOut >> 16) & 0xFF;
	outputData[11] =	(ui32_longOut >> 24) & 0xFF;

	ui16_shortOut = shortOut->getInteger();
	outputData[12] =	(ui16_shortOut >> 0) & 0xFF;
	outputData[13] =	(ui16_shortOut >> 8) & 0xFF;

	ui8_byteOut = byteOut->getInteger();
	outputData[14] =	ui8_byteOut;
}

bool EasyCAT::saveDeviceData(tinyxml2::XMLElement* xml) { return true; }
bool EasyCAT::loadDeviceData(tinyxml2::XMLElement* xml) { return true; }