#include <pch.h>
#include "EtherCatData.h"

#include "EtherCatError.h"

std::vector<EtherCatData> etherCatDataTypes = {
	{EtherCatData::Type::UINT8_T,	"uint8_t",	"UINT8_T"},
	{EtherCatData::Type::INT8_T,	"int8_t",	"INT8_T"},
	{EtherCatData::Type::UINT16_T,	"uint16_t", "UINT16_T"},
	{EtherCatData::Type::INT16_T,	"int16_t",	"INT16_T"},
	{EtherCatData::Type::UINT32_T,	"uint32_t", "UINT32_T"},
	{EtherCatData::Type::INT32_T,	"int32_t",	"INT32_T"},
	{EtherCatData::Type::UINT64_T,	"uint64_t", "UINT64_T"},
	{EtherCatData::Type::INT64_T,	"int64_t",	"INT64_T"},
	{EtherCatData::Type::STRING,	"string",	"STRING"},
	{EtherCatData::Type::NONE,		"none",		"NONE"}
};
std::vector<EtherCatData>& getEtherCatDataTypes() { return etherCatDataTypes; }
EtherCatData* getEtherCatDataType(const char* saveName) {
	for (EtherCatData& etherCatData : etherCatDataTypes) {
		if (strcmp(saveName, etherCatData.saveName) == 0) return &etherCatData;
	}
	return nullptr;
}
EtherCatData* getEtherCatDataType(EtherCatData::Type type) {
	for (EtherCatData& etherCatData : etherCatDataTypes) {
		if (type == etherCatData.type) return &etherCatData;
	}
	return nullptr;
}

std::vector<DataFormat> dataFormats = {
	{DataFormat::Type::DECIMAL,		"Dec", "Dec"},
	{DataFormat::Type::HEXADECIMAL, "Hex", "Hex"},
	{DataFormat::Type::BINARY,		"Bin", "Bin"},
	{DataFormat::Type::STRING,		"Str", "Str"}
};
std::vector<DataFormat>& getDataFormats() { return dataFormats; }
DataFormat* getDataFormat(const char* saveName) {
	for (DataFormat& dataFormat : dataFormats) {
		if (strcmp(saveName, dataFormat.saveName) == 0) return &dataFormat;
	}
	return nullptr;
}
DataFormat* getDataFormat(DataFormat::Type type) {
	for (DataFormat& dataFormat : dataFormats) {
		if (type == dataFormat.type) return &dataFormat;
	}
	return nullptr;
}

bool EtherCatCoeData::write(uint16_t slaveIndex) {
	bool success = false;
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &u8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT8_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &s8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT16_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &u16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT16_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &s16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT32_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &u32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT32_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &s32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT64_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &u64, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT64_T:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &s64, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::STRING:
			success = 1 == ec_SDOwrite(slaveIndex, index, subindex, false, strlen(stringBuffer), stringBuffer, EC_TIMEOUTSAFE);
	}
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}

bool EtherCatCoeData::read(uint16_t slaveIndex) {
	int size;
	bool success = false;
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			size = 1;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT8_T:
			size = 1;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT16_T:
			size = 2;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT16_T:
			size = 2;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT32_T:
			size = 4;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT32_T:
			size = 4;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT64_T:
			size = 8;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u64, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT64_T:
			size = 8;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s64, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::STRING:
			size = stringBufferSize;
			success = 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, stringBuffer, EC_TIMEOUTSAFE); break;
	}
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}


bool EtherCatRegisterData::write(uint16_t slaveAddress) {
	bool success = false;
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 1, &u8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT8_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 1, &s8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT16_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 2, &u16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT16_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 2, &s16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT32_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 4, &u32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT32_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 4, &s32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT64_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 8, &u64, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT64_T:		
			success = 1 == ec_FPWR(slaveAddress, registerAddress, 8, &s64, EC_TIMEOUTSAFE); break;
	}
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}

bool EtherCatRegisterData::read(uint16_t slaveAddress) {
	bool success = false;
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 1, &u8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT8_T:	   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 1, &s8, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT16_T:	   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 2, &u16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT16_T:	   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 2, &s16, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT32_T:	   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 4, &u32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT32_T:	   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 4, &s32, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::UINT64_T:	   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 8, &u64, EC_TIMEOUTSAFE); break;
		case EtherCatData::Type::INT64_T:	   				   
			success = 1 == ec_FPRD(slaveAddress, registerAddress, 8, &s64, EC_TIMEOUTSAFE); break;
	}
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}

bool EtherCatEepromData::write(uint16_t slaveIndex) {
	bool success = 1 == ec_writeeeprom(slaveIndex, address, u32, EC_TIMEOUTEEP);
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}

bool EtherCatEepromData::read(uint16_t slaveIndex) {
	u32 = ec_readeeprom(slaveIndex, address, EC_TIMEOUTEEP);
	if (u32 == 0) return false;
	return true;
}