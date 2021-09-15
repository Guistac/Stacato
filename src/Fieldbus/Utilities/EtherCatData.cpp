#include <pch.h>
#include "EtherCatData.h"

std::vector<EtherCatDataType> EtherCatData::dataTypes = {
	{EtherCatDataType::Type::UINT8_T,	"uint8_t",	"UINT8_T"},
	{EtherCatDataType::Type::INT8_T,	"int8_t",	"INT8_T"},
	{EtherCatDataType::Type::UINT16_T,	"uint16_t", "UINT16_T"},
	{EtherCatDataType::Type::INT16_T,	"int16_t",	"INT16_T"},
	{EtherCatDataType::Type::UINT32_T,	"uint32_t", "UINT32_T"},
	{EtherCatDataType::Type::INT32_T,	"int32_t",	"INT32_T"},
	{EtherCatDataType::Type::UINT64_T,	"uint64_t", "UINT64_T"},
	{EtherCatDataType::Type::INT64_T,	"int64_t",	"INT64_T"}
};


bool EtherCatData::write(uint16_t slaveIndex) {
	switch (dataType) {
		case EtherCatDataType::Type::UINT8_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &u8, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT8_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &s8, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::UINT16_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &u16, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT16_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &s16, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::UINT32_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &u32, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT32_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &s32, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::UINT64_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &u64, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT64_T:
			return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &s64, EC_TIMEOUTSAFE);
	}
}

bool EtherCatData::read(uint16_t slaveIndex) {
	int size;
	switch (dataType) {
		case EtherCatDataType::Type::UINT8_T:
			size = 1;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u8, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT8_T:
			size = 1;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s8, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::UINT16_T:
			size = 2;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u16, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT16_T:
			size = 2;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s16, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::UINT32_T:
			size = 4;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u32, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT32_T:
			size = 4;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s32, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::UINT64_T:
			size = 8;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &u64, EC_TIMEOUTSAFE);
		case EtherCatDataType::Type::INT64_T:
			size = 8;
			return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &s64, EC_TIMEOUTSAFE);
	}
}