#include <pch.h>
#include "EtherCatData.h"

std::vector<EtherCatData> dataTypes = {
	{EtherCatData::Type::UINT8_T,	"uint8_t",	"UINT8_T"},
	{EtherCatData::Type::INT8_T,	"int8_t",	"INT8_T"},
	{EtherCatData::Type::UINT16_T,	"uint16_t", "UINT16_T"},
	{EtherCatData::Type::INT16_T,	"int16_t",	"INT16_T"},
	{EtherCatData::Type::UINT32_T,	"uint32_t", "UINT32_T"},
	{EtherCatData::Type::INT32_T,	"int32_t",	"INT32_T"},
	{EtherCatData::Type::UINT64_T,	"uint64_t", "UINT64_T"},
	{EtherCatData::Type::INT64_T,	"int64_t",	"INT64_T"},
	{EtherCatData::Type::NONE,		"none",		"NONE"}
};

std::vector<DataFormat> dataRepresentations = {
	{DataFormat::Type::DECIMAL,		"Dec", "Dec"},
	{DataFormat::Type::HEXADECIMAL, "Hex", "Hex"},
	{DataFormat::Type::BINARY,		"Bin", "Bin"}
};


bool EtherCatCoeData::write(uint16_t slaveIndex) {
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &u8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT8_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &s8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT16_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &u16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT16_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &s16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT32_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &u32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT32_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &s32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT64_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &u64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT64_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &s64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::NONE: return false;
	}
}

bool EtherCatCoeData::read(uint16_t slaveIndex) {
	int size;
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			size = 1;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT8_T:
			size = 1;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT16_T:
			size = 2;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT16_T:
			size = 2;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT32_T:
			size = 4;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT32_T:
			size = 4;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT64_T:
			size = 8;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT64_T:
			size = 8;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::NONE: return false;
	}
}


bool EtherCatRegisterData::write(uint16_t slaveAddress) {
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			return 1 == ec_FPWR(slaveAddress, registerAddress, 1, &u8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT8_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 1, &s8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT16_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 2, &u16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT16_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 2, &s16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT32_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 4, &u32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT32_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 4, &s32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT64_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 8, &u64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT64_T:		
			return 1 == ec_FPWR(slaveAddress, registerAddress, 8, &s64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::NONE: return false;
	}
}

bool EtherCatRegisterData::read(uint16_t slaveAddress) {
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			return 1 == ec_FPRD(slaveAddress, registerAddress, 1, &u8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT8_T:	   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 1, &s8, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT16_T:	   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 2, &u16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT16_T:	   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 2, &s16, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT32_T:	   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 4, &u32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT32_T:	   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 4, &s32, EC_TIMEOUTSAFE);
		case EtherCatData::Type::UINT64_T:	   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 8, &u64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::INT64_T:	   				   
			return 1 == ec_FPRD(slaveAddress, registerAddress, 8, &s64, EC_TIMEOUTSAFE);
		case EtherCatData::Type::NONE: return false;
	}
}

bool EtherCatEepromData::write(uint16_t slaveIndex) {
	return 1 == ec_writeeeprom(slaveIndex, address, u32, EC_TIMEOUTEEP);
}

bool EtherCatEepromData::read(uint16_t slaveIndex) {
	u32 = ec_readeeprom(slaveIndex, address, EC_TIMEOUTEEP);
	if (u32 == 0) return false;
	return true;
}