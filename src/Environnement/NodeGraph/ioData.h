#pragma once

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

	ioData(DataType t, DataDirection d, const char* n, bool acceptsMultipleInputs = false) : type(t), direction(d) {
		strcpy(name, n);
		switch (type) {
			case BOOLEAN_VALUE: booleanValue = false;
			case INTEGER_VALUE: integerValue = 0;
			case REAL_VALUE: realValue = 0.0;
		}
		b_acceptsMultipleInputs = acceptsMultipleInputs;
	}

	//data infos
	const char* getName() { return name; }
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }

	DataType getType() { return type; }
	const char* getTypeName() { return dataTypeNames[type]; }
	bool isSameTypeAs(ioData& other) { return other.type == type; }
	void setType(DataType t) {
		switch (t){
			case BOOLEAN_VALUE: set(getBoolean()); break;
			case INTEGER_VALUE: set(getInteger()); break;
			case REAL_VALUE: set(getReal()); break;
		}
		type = t;
	}

	//link infos
	std::vector<std::shared_ptr<ioLink>>& getLinks() { return ioLinks; }
	bool isConnected() { return !ioLinks.empty(); }
	bool acceptsMultipleInputs() { return b_acceptsMultipleInputs; }
	bool hasMultipleLinks() { return ioLinks.size() > 1; }
	
	//nodegraph infos
	int getUniqueID() { return uniqueID; }
	std::shared_ptr<ioNode> getNode() { return parentNode; }
	bool& isVisible() { return b_visible; }

	//datatype infos
	bool isBool()				{ return type == BOOLEAN_VALUE; }
	bool isInteger()			{ return type == INTEGER_VALUE; }
	bool isDouble()				{ return type == REAL_VALUE; }

	//setting data (with data conversions)
	void set(bool boolean);
	void set(long long int integer);
	void set(double real);

	//reading data (with data conversions)
	bool getBoolean();
	long long int getInteger();
	double getReal();

	//TODO: is this useful ?
	bool hasNewValue() { return b_hasNewValue; }

	const char* getValueString();

	void copyToLinked();
	void copyFromLinked();

	std::vector<std::shared_ptr<ioNode>> getNodesLinkedAtOutputs();
	std::vector<std::shared_ptr<ioNode>> getNodesLinkedAtInputs();

	float getGuiWidth(bool alwaysShowValue);
	void pinGui(bool alwaysShowValue);
	void dataGui();


private:

	friend class NodeGraph;
	friend class ioNode;
	friend class ioLink;

	std::shared_ptr<ioNode> parentNode;
	std::vector<std::shared_ptr<ioLink>> ioLinks;
	int uniqueID = -1;
	bool b_visible = true;

	DataType type;
	DataDirection direction;
	char name[64];
	bool b_acceptsMultipleInputs = false;
	
	bool b_hasNewValue = false;

	union {
		bool booleanValue;
		long long int integerValue;
		double realValue;
	};

	static const char* dataTypeNames[TYPE_COUNT];
};