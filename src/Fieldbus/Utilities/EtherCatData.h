#pragma once

#include <ethercat.h>

struct EtherCatData {
	enum Type {
		UINT8_T,
		INT8_T,
		UINT16_T,
		INT16_T,
		UINT32_T,
		INT32_T,
		UINT64_T,
		INT64_T,
		STRING,
		NONE
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};
extern std::vector<EtherCatData> etherCatDataTypes;
std::vector<EtherCatData>& getEtherCatDataTypes();
EtherCatData* getEtherCatDataType(const char* saveName);
EtherCatData* getEtherCatDataType(EtherCatData::Type type);

struct DataFormat {
	enum Type {
		DECIMAL,
		HEXADECIMAL,
		BINARY,
		STRING
	};
	Type type;
	const char displayName[64];
	const char saveName[64];
};

extern std::vector<DataFormat> dataFormats;
std::vector<DataFormat>& getDataFormats();
DataFormat* getDataFormat(const char* saveName);
DataFormat* getDataFormat(DataFormat::Type type);

class EtherCatBaseData {
public:

	EtherCatBaseData() {
		sprintf(name, "");
		dataType = EtherCatData::Type::NONE;
		dataFormat = DataFormat::DECIMAL;
	}

	EtherCatBaseData(EtherCatData::Type t) {
		sprintf(name, "");
		dataType = t;
		if (dataType == EtherCatData::Type::STRING) dataFormat = DataFormat::STRING;
		else dataFormat = DataFormat::DECIMAL;
	}

	EtherCatBaseData(const char* n, EtherCatData::Type t, DataFormat::Type r) {
		sprintf(name, n);
		dataType = t;
		if (dataType == EtherCatData::Type::STRING) dataFormat = DataFormat::STRING;
		else if (r = DataFormat::STRING) dataFormat = DataFormat::DECIMAL;
		else dataFormat = r;
	}

	EtherCatData::Type dataType;
	DataFormat::Type dataFormat;
	char name[128];

	uint8_t		u8 = 0;
	int8_t		s8 = 0;
	uint16_t	u16 = 0;
	int16_t		s16 = 0;
	uint32_t	u32 = 0;
	int32_t		s32 = 0;
	uint64_t	u64 = 0;
	int64_t		s64 = 0;
	char stringBuffer[128] = "0";
	const int stringBufferSize = 128;

	const char* getValueString() {
		static char valueString[128];
		switch (dataType) {
			case EtherCatData::Type::UINT8_T:	sprintf(valueString, "%i", u8); break;
			case EtherCatData::Type::INT8_T:	sprintf(valueString, "%i", s8); break;
			case EtherCatData::Type::UINT16_T:	sprintf(valueString, "%i", u16); break;
			case EtherCatData::Type::INT16_T:	sprintf(valueString, "%i", s16); break;
			case EtherCatData::Type::UINT32_T:	sprintf(valueString, "%i", u32); break;
			case EtherCatData::Type::INT32_T:	sprintf(valueString, "%i", s32); break;
			case EtherCatData::Type::UINT64_T:	sprintf(valueString, "%lli", u64); break;
			case EtherCatData::Type::INT64_T:	sprintf(valueString, "%lli", s64); break;
			case EtherCatData::Type::STRING:	return stringBuffer;
		}
		return valueString;
	}

	void valueTextGui();
	bool dataEditFieldGui();
	bool dataTypeSelectorGui();
	void dataFormatSelectorGui();

	void setU8(uint8_t val)		{ u8 = val;		dataType = EtherCatData::Type::UINT8_T; }
	void setS8(int8_t val)		{ s8 = val;		dataType = EtherCatData::Type::INT8_T; }
	void setU16(uint16_t val)	{ u16 = val;	dataType = EtherCatData::Type::UINT16_T; }
	void setS16(int16_t val)	{ s16 = val;	dataType = EtherCatData::Type::INT16_T; }
	void setU32(uint32_t val)	{ u32 = val;	dataType = EtherCatData::Type::UINT32_T; }
	void setS32(int32_t val)	{ s32 = val;	dataType = EtherCatData::Type::INT32_T; }
	void setU64(uint64_t val)	{ u64 = val;	dataType = EtherCatData::Type::UINT64_T; }
	void setS64(int64_t val)	{ s64 = val;	dataType = EtherCatData::Type::INT64_T;	}
	void setString(const char* str) { strcpy(stringBuffer, str); dataType = EtherCatData::Type::STRING; }

	uint8_t getU8() { return u8; }
	int8_t getS8() { return s8; }
	uint16_t getU16() { return u16; }
	int16_t getS16() { return s16; }
	uint32_t getU32() { return u32; }
	int32_t getS32() { return s32; }
	uint64_t getU64() { return u64; }
	int64_t getS64() { return s64; }
	const char* getString() { return stringBuffer; }

	virtual bool write(uint16_t slaveIndex) = 0;
	virtual bool read(uint16_t slaveIndex) = 0;

	bool b_isTransfering = false;
	bool b_transferSuccessfull = false;
	bool b_hasTransferred = false;
};




class EtherCatCoeData : public EtherCatBaseData {
public:

	EtherCatCoeData(uint16_t idx, uint8_t sidx) : EtherCatBaseData(){
		index = idx;
		subindex = sidx;
	}

	EtherCatCoeData(uint16_t idx, uint8_t sidx, EtherCatData::Type t) : EtherCatBaseData(t) {
		index = idx;
		subindex = sidx;
	}

	EtherCatCoeData(const char* name, uint16_t idx, uint8_t sidx, EtherCatData::Type t, DataFormat::Type r) : EtherCatBaseData(name, t, r) {
		index = idx;
		subindex = sidx;
	}

	uint16_t index;
	uint8_t subindex;

	bool indexEditFieldGui();
	bool subindexEditFieldGui();

	virtual bool write(uint16_t slaveIndex);
	virtual bool read(uint16_t slaveIndex);
};



class EtherCatRegisterData : public EtherCatBaseData {
public:

	EtherCatRegisterData(const char* name, uint16_t reg, EtherCatData::Type t, DataFormat::Type r) : EtherCatBaseData(name, t, r) {
		registerAddress = reg;
	}

	uint16_t registerAddress;

	bool registerEditFieldGui();

	virtual bool write(uint16_t slaveAddress);
	virtual bool read(uint16_t slaveAddress);

};

class EtherCatEepromData : public EtherCatBaseData {
public:
	EtherCatEepromData(const char* name, uint16_t a, DataFormat::Type r) : EtherCatBaseData(name, EtherCatData::Type::UINT16_T, r) {
		address = a;
	}

	uint16_t address;

	bool addressFieldGui();

	virtual bool write(uint16_t slaveIndex);
	virtual bool read(uint16_t slaveIndex);
};


namespace CanOpen {

	bool writeSDO_U8(uint16_t index, uint8_t subindex, uint8_t val, uint16_t slaveIndex);
	bool writeSDO_S8(uint16_t index, uint8_t subindex, int8_t val, uint16_t slaveIndex);
	bool writeSDO_U16(uint16_t index, uint8_t subindex, uint16_t val, uint16_t slaveIndex);
	bool writeSDO_S16(uint16_t index, uint8_t subindex, int16_t val, uint16_t slaveIndex);
	bool writeSDO_U32(uint16_t index, uint8_t subindex, uint32_t val, uint16_t slaveIndex);
	bool writeSDO_S32(uint16_t index, uint8_t subindex, int32_t val, uint16_t slaveIndex);
	bool writeSDO_U64(uint16_t index, uint8_t subindex, uint64_t val, uint16_t slaveIndex);
	bool writeSDO_S64(uint16_t index, uint8_t subindex, int64_t val, uint16_t slaveIndex);
	bool writeSDO_String(uint16_t index, uint8_t subindex, const char* val, uint16_t slaveIndex);

	bool readSDO_U8(uint16_t index, uint8_t subindex, uint8_t& val, uint16_t slaveIndex);
	bool readSDO_S8(uint16_t index, uint8_t subindex, int8_t& val, uint16_t slaveIndex);
	bool readSDO_U16(uint16_t index, uint8_t subindex, uint16_t& val, uint16_t slaveIndex);
	bool readSDO_S16(uint16_t index, uint8_t subindex, int16_t& val, uint16_t slaveIndex);
	bool readSDO_U32(uint16_t index, uint8_t subindex, uint32_t& val, uint16_t slaveIndex);
	bool readSDO_S32(uint16_t index, uint8_t subindex, int32_t& val, uint16_t slaveIndex);
	bool readSDO_U64(uint16_t index, uint8_t subindex, uint64_t& val, uint16_t slaveIndex);
	bool readSDO_S64(uint16_t index, uint8_t subindex, int64_t& val, uint16_t slaveIndex);
	bool readSDO_String(uint16_t index, uint8_t subindex, const char* val, int maxStringSize, uint16_t slaveIndex);

}