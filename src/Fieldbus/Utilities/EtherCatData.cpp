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
			success = CanOpen::writeSDO_U8(index, subindex, u8, slaveIndex); break;
		case EtherCatData::Type::INT8_T:
			success = CanOpen::writeSDO_S8(index, subindex, s8, slaveIndex); break;
		case EtherCatData::Type::UINT16_T:
			success = CanOpen::writeSDO_U16(index, subindex, u16, slaveIndex); break;
		case EtherCatData::Type::INT16_T:
			success = CanOpen::writeSDO_S16(index, subindex, s16, slaveIndex); break;
		case EtherCatData::Type::UINT32_T:
			success = CanOpen::writeSDO_U32(index, subindex, u32, slaveIndex); break;
		case EtherCatData::Type::INT32_T:
			success = CanOpen::writeSDO_S32(index, subindex, s32, slaveIndex); break;
		case EtherCatData::Type::UINT64_T:
			success = CanOpen::writeSDO_U64(index, subindex, u64, slaveIndex); break;
		case EtherCatData::Type::INT64_T:
			success = CanOpen::writeSDO_S64(index, subindex, s64, slaveIndex); break;
		case EtherCatData::Type::STRING:
			success = CanOpen::writeSDO_String(index, subindex, stringBuffer, slaveIndex); break;
	}
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}




bool EtherCatCoeData::read(uint16_t slaveIndex) {
	int size;
	bool success = false;
	switch (dataType) {
		case EtherCatData::Type::UINT8_T:
			success = CanOpen::readSDO_U8(index, subindex, u8, slaveIndex); break;
		case EtherCatData::Type::INT8_T:
			success = CanOpen::readSDO_S8(index, subindex, s8, slaveIndex); break;
		case EtherCatData::Type::UINT16_T:
			success = CanOpen::readSDO_U16(index, subindex, u16, slaveIndex); break;
		case EtherCatData::Type::INT16_T:
			success = CanOpen::readSDO_S16(index, subindex, s16, slaveIndex); break;
		case EtherCatData::Type::UINT32_T:
			success = CanOpen::readSDO_U32(index, subindex, u32, slaveIndex); break;
		case EtherCatData::Type::INT32_T:
			success = CanOpen::readSDO_S32(index, subindex, s32, slaveIndex); break;
		case EtherCatData::Type::UINT64_T:
			success = CanOpen::readSDO_U64(index, subindex, u64, slaveIndex); break;
		case EtherCatData::Type::INT64_T:
			success = CanOpen::readSDO_S64(index, subindex, s64, slaveIndex); break;
		case EtherCatData::Type::STRING:
			success = CanOpen::readSDO_String(index, subindex, stringBuffer, stringBufferSize, slaveIndex); break;
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
	bool success = 1 == ec_writeeeprom(slaveIndex, address, u16, EC_TIMEOUTEEP);
	if (EtherCatError::hasError()) EtherCatError::logError();
	return success;
}

bool EtherCatEepromData::read(uint16_t slaveIndex) {
	u16 = ec_readeeprom(slaveIndex, address, EC_TIMEOUTEEP);
	if (u16 == 0) return false;
	return true;
}


namespace CanOpen {

	//=============================== WRITE COE SDO ========================

	bool writeSDO_U8(uint16_t index, uint8_t subindex, uint8_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_S8(uint16_t index, uint8_t subindex, int8_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 1, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_U16(uint16_t index, uint8_t subindex, uint16_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_S16(uint16_t index, uint8_t subindex, int16_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 2, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_U32(uint16_t index, uint8_t subindex, uint32_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_S32(uint16_t index, uint8_t subindex, int32_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 4, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_U64(uint16_t index, uint8_t subindex, uint64_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_S64(uint16_t index, uint8_t subindex, int64_t val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, 8, &val, EC_TIMEOUTRXM);
	}
	bool writeSDO_String(uint16_t index, uint8_t subindex, const char* val, uint16_t slaveIndex) {
		return 1 == ec_SDOwrite(slaveIndex, index, subindex, false, strlen(val), &val, EC_TIMEOUTRXM);
	}

	//=============================== READ COE SDO ========================

	bool readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& val, uint16_t slaveIndex) {
		int size = 1;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_S8(uint16_t index, uint8_t subindex, int8_t& val, uint16_t slaveIndex) {
		int size = 1;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& val, uint16_t slaveIndex) {
		int size = 2;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_S16(uint16_t index, uint8_t subindex, int16_t& val, uint16_t slaveIndex) {
		int size = 2;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& val, uint16_t slaveIndex) {
		int size = 4;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_S32(uint16_t index, uint8_t subindex, int32_t& val, uint16_t slaveIndex) {
		int size = 4;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& val, uint16_t slaveIndex) {
		int size = 8;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_S64(uint16_t index, uint8_t subindex, int64_t& val, uint16_t slaveIndex) {
		int size = 8;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, &val, EC_TIMEOUTRXM);
	}
	bool readSDO_String(uint16_t index, uint8_t subindex, const char* val, int bufferSize, uint16_t slaveIndex) {
		int size = bufferSize;
		return 1 == ec_SDOread(slaveIndex, index, subindex, false, &size, (void*)val, EC_TIMEOUTRXM);
	}

}
