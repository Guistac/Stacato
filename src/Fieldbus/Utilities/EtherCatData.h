#pragma once

#include <ethercat.h>

struct EtherCatDataType {
	enum Type {
		UINT8_T,
		INT8_T,
		UINT16_T,
		INT16_T,
		UINT32_T,
		INT32_T,
		UINT64_T,
		INT64_T
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

class EtherCatData{
public:

	EtherCatData(const char* name, uint16_t idx, uint8_t sidx, EtherCatDataType::Type type) {
		sprintf(variableName, name);
		index = idx;
		subindex = subindex;
		dataType = type;
	}

	EtherCatDataType::Type dataType;
	char variableName[128];
	uint16_t index;
	uint8_t subindex;

	union {
		uint8_t		u8;
		int8_t		s8;
		uint16_t	u16;
		int16_t		s16;
		uint32_t	u32;
		int32_t		s32;
		uint64_t	u64;
		int64_t		s64;
	};

	const char* getTypeString() {
		return dataTypes[dataType].displayName;
	}

	const char* getValueString() {
		static char valueString[64];
		switch (dataType) {
			case EtherCatDataType::Type::UINT8_T:	sprintf(valueString, "%i", u8); break;
			case EtherCatDataType::Type::INT8_T:	sprintf(valueString, "%i", s8); break;
			case EtherCatDataType::Type::UINT16_T:	sprintf(valueString, "%i", u16); break;
			case EtherCatDataType::Type::INT16_T:	sprintf(valueString, "%i", s16); break;
			case EtherCatDataType::Type::UINT32_T:	sprintf(valueString, "%i", u32); break;
			case EtherCatDataType::Type::INT32_T:	sprintf(valueString, "%i", s32); break;
			case EtherCatDataType::Type::UINT64_T:	sprintf(valueString, "%i", u64); break;
			case EtherCatDataType::Type::INT64_T:	sprintf(valueString, "%i", s64); break;
		}
		return valueString;
	}

	void valueTextGui();
	bool indexEditFieldGui();
	bool subindexEditFieldGui();
	bool dataEditFieldGui();
	bool dataTypeSelectorGui();

	void set(uint8_t val) { u8 = val; }
	void set(int8_t val) { u8 = val; }
	void set(uint16_t val) { u16 = val; }
	void set(int16_t val) { u16 = val; }
	void set(uint32_t val) { u32 = val; }
	void set(int32_t val) { u32 = val; }
	void set(uint64_t val) { u64 = val; }
	void set(int64_t val) { u64 = val; }

	uint8_t getU8() { return u8; }
	int8_t getS8() { return s8; }
	uint16_t getU16() { return u16; }
	int16_t getS16() { return s16; }
	uint32_t getU32() { return u32; }
	int32_t getS32() { return s32; }
	uint64_t getU64() { return u64; }
	int64_t getS64() { return s64; }

	bool write(uint16_t slaveIndex);
	bool read(uint16_t slaveIndex);

	static std::vector<EtherCatDataType> dataTypes;
	 
	bool b_isTransfering = false;
	bool b_transferSuccessfull = false;
	bool b_hasTransferred = false;
};

