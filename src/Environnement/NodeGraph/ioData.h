#pragma once

#include <ethercat.h> //this defines the basic data types

enum DataType {
	BOOLEAN_VALUE,
	INTEGER_VALUE,
	REAL_VALUE,
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

	ioData(DataType t, DataDirection d, const char* n) : type(t), direction(d) {
		strcpy(name, n);
		switch (type) {
			case BOOLEAN_VALUE: booleanValue = false;
			case INTEGER_VALUE: integerValue = 0;
			case REAL_VALUE: realValue = 0.0;
		}
	}

	//data infos
	const char* getName() { return name; }
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }

	DataType getType() { return type; }
	const char* getTypeName() { return dataTypeNames[type]; }
	bool isSameTypeAs(ioData& other) { return other.type == type; }

	//nodegraph infos
	int getUniqueID() { return uniqueID; }
	bool isConnected() { return !ioLinks.empty(); }
	ioNode* getNode() { return parentNode; }
	bool& isVisible() { return b_visible; }

	bool isBool()				{ return type == BOOLEAN_VALUE; }
	bool isInteger()			{ return type == INTEGER_VALUE; }
	bool isDouble()				{ return type == REAL_VALUE; }

	//setting data (with data conversions)
	void set(bool boolean) {
		switch (type) {
		case BOOLEAN_VALUE: booleanValue = boolean; break;
		case INTEGER_VALUE: integerValue = boolean; break;
		case REAL_VALUE: realValue = boolean; break;
		}
	}

	void set(long long int integer) {
		switch (type) {
		case INTEGER_VALUE: integerValue = integer; break;
		case BOOLEAN_VALUE: booleanValue = integer; break;
		case REAL_VALUE: realValue = integer; break;
		}
	}

	void set(double real) {
		switch (type) {
		case REAL_VALUE: realValue = real; break;
		case BOOLEAN_VALUE: booleanValue = real; break;
		case INTEGER_VALUE: integerValue = real; break;
		}
	}

	//reading data (with data conversions)
	bool getBoolean() {
		switch (type) {
		case BOOLEAN_VALUE: return booleanValue;
		case INTEGER_VALUE: return integerValue > 0;
		case REAL_VALUE: return realValue > 0;
		}
	}
	long long int getInteger() {
		switch (type) {
		case INTEGER_VALUE: return integerValue;
		case BOOLEAN_VALUE: return (long long int)booleanValue;
		case REAL_VALUE: return (long long int)realValue;
		}
	}
	double getReal() {
		switch (type) {
		case REAL_VALUE: return realValue;
		case BOOLEAN_VALUE: return (double)booleanValue;
		case INTEGER_VALUE: return (double)integerValue;
		}
	}


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

	union {
		bool booleanValue;
		long long int integerValue;
		double realValue;
	};

	static const char* dataTypeNames[TYPE_COUNT];
};