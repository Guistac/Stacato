#pragma once

#include <ethercat.h> //this defines the basic data types

enum DataType {
	BOOL_VALUE,
	UINT8_T,
	INT8_T,
	UINT16_T,
	INT16_T,
	UINT32_T,
	INT32_T,
	UINT64_T,
	INT64_T,
	FLOAT32,
	FLOAT64,
	TYPE_COUNT
};


enum DataDirection {
	NODE_INPUT,
	NODE_OUTPUT,
	NOT_SPECIFIED
};

class ioNode;
class ioLink;

class ioData {
public:

	ioData(DataType t, DataDirection d, const char* n) : type(t), direction(d) { strcpy(name, n); }

	//data infos
	const char* getName() { return name; }
	DataType getType() { return type; }
	const char* getTypeName() { return dataTypeNames[type]; }
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }

	//nodegraph infos
	int getUniqueID() { return uniqueID; }
	bool isConnected() { return !ioLinks.empty(); }
	ioNode* getNode() { return parentNode; }
	bool& isVisible() { return b_visible; }
	bool hasChanged() { return hasChanged; }

	int getByteCount() {
		switch (type) {
			case BOOL_VALUE:	return 1;
			case UINT8_T:		return 1;
			case INT8_T:		return 1;
			case UINT16_T:		return 2;
			case INT16_T:		return 2;
			case UINT32_T:		return 4;
			case INT32_T:		return 4;
			case UINT64_T:		return 8;
			case INT64_T:		return 8;
			case FLOAT32:		return 4;
			case FLOAT64:		return 8;
		}
	}

	bool isBool()				{ return type == BOOL_VALUE; }
	bool isUnsignedByte()		{ return type == UINT8_T; }
	bool isSignedByte()			{ return type == INT8_T; }
	bool isUnsignedShort()		{ return type == UINT16_T; }
	bool isSignedShort()		{ return type == INT16_T; }
	bool isUnsignedLong()		{ return type == UINT32_T; }
	bool isSignedLong()			{ return type == INT32_T; }
	bool isUnsignedLongLong()	{ return type == UINT64_T; }
	bool isSignedLongLong()		{ return type == INT64_T; }
	bool isFloat()				{ return type == FLOAT32; }
	bool isDouble()				{ return type == FLOAT64; }

	bool isSameTypeAs(ioData& other) { return other.type == type; }



	//assigning data
	void set(bool data)		{ boolValue = data; }
	void set(uint8_t data)	{ unsignedByte = data; }
	void set(int8_t data)	{ signedByte = data; }
	void set(uint16_t data)	{ unsignedShort = data; }
	void set(int16_t data)	{ signedShort = data; }
	void set(uint32_t data)	{ unsignedLong = data; }
	void set(int32_t data)	{ signedLong = data; }
	void set(uint64_t data)	{ unsignedLongLong = data; }
	void set(int64_t data)	{ signedLongLong = data; }
	void set(float data)	{ floatValue = data; }
	void set(double data)	{ doubleValue = data; }

	//reading data
	bool& getBool()					{ return boolValue; }
	uint8_t& getUnsignedByte()		{ return unsignedByte; }
	int8_t& getSignedByte()			{ return signedByte; }
	uint16_t& getUnsignedShort()	{ return unsignedShort; }
	int16_t& getSignedShort()		{ return signedShort; }
	uint32_t& getUnsignedLong()		{ return unsignedLong; }
	int32_t& getSignedLong()		{ return signedLong; }
	uint64_t& getUnsignedLongLong() { return unsignedLongLong; }
	int64_t& getSignedLongLong()	{ return signedLongLong; }
	float& getFloat()				{ return floatValue; }
	double& getDouble()				{ return doubleValue; }

	bool getData(bool& data)		{ if (type == DataType::BOOL_VALUE) { data = boolValue;			return true; } else return false; }
	bool getData(uint8_t& data)		{ if (type == DataType::UINT8_T)	{ data = unsignedByte;		return true; } else return false; }
	bool getData(int8_t& data)		{ if (type == DataType::INT8_T)		{ data = signedByte;		return true; } else return false; }
	bool getData(uint16_t& data)	{ if (type == DataType::UINT16_T)	{ data = unsignedShort;		return true; } else return false; }
	bool getData(int16_t& data)		{ if (type == DataType::INT16_T)	{ data = signedShort;		return true; } else return false; }
	bool getData(uint32_t& data)	{ if (type == DataType::UINT32_T)	{ data = unsignedLong;		return true; } else return false; }
	bool getData(int32_t& data)		{ if (type == DataType::INT32_T)	{ data = signedLong;		return true; } else return false; }
	bool getData(uint64_t& data)	{ if (type == DataType::UINT64_T)	{ data = unsignedLongLong;	return true; } else return false; }
	bool getData(int64_t& data)		{ if (type == DataType::INT64_T)	{ data = signedLongLong;	return true; } else return false; }
	bool getData(float& data)		{ if (type == DataType::FLOAT32)	{ data = floatValue;		return true; } else return false; }
	bool getData(double& data)		{ if (type == DataType::FLOAT64)	{ data = doubleValue;		return true; } else return false; }

private:

	friend class NodeGraph;
	friend class ioNode;
	friend class ioLink;

	ioNode* parentNode = nullptr;
	std::vector<std::shared_ptr<ioLink>> ioLinks;
	int uniqueID = -1;
	bool b_visible = true;

	DataType type;
	DataDirection direction;
	char name[64];
	bool b_hasChanged = false;

	union {
		bool boolValue;
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

	static const char* dataTypeNames[TYPE_COUNT];
};