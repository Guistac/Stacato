#pragma once

#include <ethercat.h>

class EtherCatData {
public:

	enum Type {
		BOOL,
		UINT8_T,
		INT8_T,
		UINT16_T,
		INT16_T,
		UINT32_T,
		INT32_T,
		UINT64_T,
		INT64_T,
		FLOAT,
		DOUBLE,
		TYPE_COUNT
	};

	EtherCatData() {}
	EtherCatData(Type t, const char* n) : type(t) { strcpy(name, n); }

	const char* getName() { return name; }
	Type getType() { return type; }
	const char* getTypeName() { return dataTypeNames[type]; }

	int getByteCount() {
		switch (type) {
			case BOOL:		return 1; break;
			case UINT8_T:	return 1; break;
			case INT8_T:	return 1; break;
			case UINT16_T:	return 2; break;
			case INT16_T:	return 2; break;
			case UINT32_T:	return 4; break;
			case INT32_T:	return 4; break;
			case UINT64_T:	return 8; break;
			case INT64_T:	return 8; break;
			case FLOAT:		return 4; break;
			case DOUBLE:	return 8; break;
		}
	}

	bool isBool()				{ return type == BOOL; }
	bool isUnsignedByte()		{ return type == UINT8_T; }
	bool isSignedByte()			{ return type == INT8_T; }
	bool isUnsignedShort()		{ return type == UINT16_T; }
	bool isSignedShort()		{ return type == INT16_T; }
	bool isUnsignedLong()		{ return type == UINT32_T; }
	bool isSignedLong()			{ return type == INT32_T; }
	bool isUnsignedLongLong()	{ return type == UINT64_T; }
	bool isSignedLongLong()		{ return type == INT64_T; }
	bool isFloat()				{ return type == FLOAT; }
	bool isDouble()				{ return type == DOUBLE; }

	bool isSameType(EtherCatData& other) { return other.type == type; }

	//assigning
	void operator= (bool data)		{ binaryValue = data; }
	void operator= (uint8_t data)	{ unsignedByte = data; }
	void operator= (int8_t data)	{ signedByte = data; }
	void operator= (uint16_t data)	{ unsignedShort = data; }
	void operator= (int16_t data)	{ signedShort = data; }
	void operator= (uint32_t data)	{ unsignedLong = data; }
	void operator= (int32_t data)	{ signedLong = data; }
	void operator= (uint64_t data)	{ unsignedLongLong = data; }
	void operator= (int64_t data)	{ signedLongLong = data; }
	void operator= (float data)		{ floatValue = data; }
	void operator= (double data)	{ doubleValue = data; }

	//reading
	bool getBool()					{ return binaryValue; }
	uint8_t getUnsignedByte()		{ return unsignedByte; }
	int8_t getSignedByte()			{ return signedByte; }
	uint16_t getUnsignedShort()		{ return unsignedShort; }
	int16_t getSignedShort()		{ return signedShort; }
	uint32_t getUnsignedLong()		{ return unsignedLong; }
	int32_t getSignedLong()			{ return signedLong; }
	uint64_t getUnsignedLongLong()	{ return unsignedLongLong; }
	int64_t getSignedLongLong()		{ return signedLongLong; }
	float getFloat()				{ return floatValue; }
	double getDouble()				{ return doubleValue; }

	bool getData(bool& data)		{ if (type == Type::BOOL)		{ data = binaryValue;		return true; } else return false; }
	bool getData(uint8_t& data)		{ if (type == Type::UINT8_T)	{ data = unsignedByte;		return true; } else return false; }
	bool getData(int8_t& data)		{ if (type == Type::INT8_T)		{ data = signedByte;		return true; } else return false; }
	bool getData(uint16_t& data)	{ if (type == Type::UINT16_T)	{ data = unsignedShort;		return true; } else return false; }
	bool getData(int16_t& data)		{ if (type == Type::INT16_T)	{ data = signedShort;		return true; } else return false; }
	bool getData(uint32_t& data)	{ if (type == Type::UINT32_T)	{ data = unsignedLong;		return true; } else return false; }
	bool getData(int32_t& data)		{ if (type == Type::INT32_T)	{ data = signedLong;		return true; } else return false; }
	bool getData(uint64_t& data)	{ if (type == Type::UINT64_T)	{ data = unsignedLongLong;	return true; } else return false; }
	bool getData(int64_t& data)		{ if (type == Type::INT64_T)	{ data = signedLongLong;	return true; } else return false; }
	bool getData(float& data)		{ if (type == Type::FLOAT)		{ data = floatValue;		return true; } else return false; }
	bool getData(double& data)		{ if (type == Type::BOOL)		{ data = doubleValue;		return true; } else return false; }

private:

	char name[32];
	Type type;
	static const char* dataTypeNames[TYPE_COUNT];

	union {
		bool binaryValue;
		uint8_t unsignedByte;
		int8_t signedByte;
		uint16_t unsignedShort;
		int16_t signedShort;
		uint32_t unsignedLong;
		int32_t signedLong;
		uint64_t unsignedLongLong;
		int64_t signedLongLong = 0;
		float floatValue;
		double doubleValue;
	};

};