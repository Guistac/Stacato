#include <pch.h>
#include "EasyCAT.h"

#include "Fieldbus/EtherCatFieldbus.h"

EasyCAT::EasyCAT() {
	outputData = {
		&byteOut,
		&shortOut,
		&longOut,
		&longLongOut
	};

	inputData = {
		&byteIn,
		&shortIn,
		&longIn,
		&longLongIn
	};
}

bool EasyCAT::startupConfiguration() { 
	uint32_t sync0Interval_nanoseconds = EtherCatFieldbus::processInterval_milliseconds * 1000000.0L;
	uint32_t sync0offset_nanoseconds = 0;
	ec_dcsync0(getSlaveIndex(), true, sync0Interval_nanoseconds, sync0offset_nanoseconds);
	return true;
}

void EasyCAT::process(bool b_processDataValid) {
	uint8_t* inputData = identity->inputs;
	
	longLongIn =	(uint64_t)inputData[0] |
					(uint64_t)inputData[1] << 8 |
					(uint64_t)inputData[2] << 16 |
					(uint64_t)inputData[3] << 24 |
					(uint64_t)inputData[4] << 32 |
					(uint64_t)inputData[5] << 40 |
					(uint64_t)inputData[6] << 48 |
					(uint64_t)inputData[7] << 56;

	longIn =	inputData[8] |
				inputData[9] << 8 |
				inputData[10] << 16 |
				inputData[11] << 24;

	shortIn =	inputData[12] |
				inputData[13] << 8;

	byteIn = inputData[14];

	static int counter = 0;
	//counter++;
	byteOut = counter * 8;
	shortOut = counter* 32;
	longOut = counter * 128;
	longLongOut = counter * 512;

	uint8_t* outputData = identity->outputs;

	uint8_t bOut = byteOut.getUnsignedByte();
	uint16_t sOut = shortOut.getUnsignedShort();
	uint32_t lOut = longOut.getUnsignedLong();
	uint64_t llOut = longLongOut.getUnsignedLongLong();

	outputData[0] = (lOut >> 0) & 0xFF;
	outputData[1] = (lOut >> 8) & 0xFF;
	outputData[2] = (lOut >> 16) & 0xFF;
	outputData[3] = (lOut >> 24) & 0xFF;
	outputData[4] = (lOut >> 32) & 0xFF;
	outputData[5] = (lOut >> 40) & 0xFF;
	outputData[6] = (lOut >> 48) & 0xFF;
	outputData[7] = (lOut >> 56) & 0xFF;

	outputData[8] = (lOut >> 0) & 0xFF;
	outputData[9] = (lOut >> 8) & 0xFF;
	outputData[10] = (lOut >> 16) & 0xFF;
	outputData[11] = (lOut >> 24) & 0xFF;

	outputData[12] = (sOut >> 0) & 0xFF;
	outputData[13] = (sOut >> 8) & 0xFF;

	outputData[14] = bOut;
}