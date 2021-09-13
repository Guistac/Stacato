#pragma once

enum DataType {
	BOOLEAN_VALUE,
	INTEGER_VALUE,
	REAL_VALUE,
	DEVICE_LINK,
	TYPE_COUNT
};

enum DataDirection {
	NODE_INPUT,
	NODE_OUTPUT,
	NO_DIRECTION
};

class ioNode;
class ioLink;

namespace tinyxml2 { class XMLElement; }

enum ioDataFlags {
	ioDataFlags_None					= 0,
	ioDataFlags_AcceptMultipleInputs	= 1 << 0,
	ioDataFlags_DisablePin				= 1 << 1,
	ioDataFlags_NoDataField				= 1 << 2,
	ioDataFlags_ForceDataField			= 1 << 3,
	ioDataFlags_DisableDataField		= 1 << 4
};

inline ioDataFlags operator|(ioDataFlags a, ioDataFlags b){
	return static_cast<ioDataFlags>(static_cast<int>(a) | static_cast<int>(b));
}

class ioData {
public:

	ioData(DataType t, DataDirection d, const char* n, ioDataFlags flags) : type(t), direction(d) {
		strcpy(name, n);
		switch (type) {
			case BOOLEAN_VALUE: booleanValue = false; break;
			case INTEGER_VALUE: integerValue = 0; break;
			case REAL_VALUE: realValue = 0.0; break;
			case DEVICE_LINK: break;
		}
		b_acceptsMultipleInputs = flags & ioDataFlags_AcceptMultipleInputs;
		b_disablePin = flags & ioDataFlags_DisablePin;
		b_noDataField = flags & ioDataFlags_NoDataField;
		b_forceDataField = flags & ioDataFlags_ForceDataField;
		b_disableDataField = flags & ioDataFlags_DisableDataField;
	}

	ioData(DataType t, DataDirection d, const char* n) : type(t), direction(d) {
		strcpy(name, n);
		switch (type) {
		case BOOLEAN_VALUE: booleanValue = false; break;
		case INTEGER_VALUE: integerValue = 0; break;
		case REAL_VALUE: realValue = 0.0; break;
		case DEVICE_LINK: break;
		}
	}

	//data infos
	const char* getName() { return name; }
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }

	DataType getType() { return type; }
	const char* getTypeName() { 
		switch (getType()) {
			case BOOLEAN_VALUE: return "Boolean";
			case INTEGER_VALUE:	return "Integer";
			case REAL_VALUE: return	"Real";
			case DEVICE_LINK: return "Device Link";
			default: return "unknown";
		}
	}
	bool isSameTypeAs(ioData& other) { return other.type == type; }
	void setType(DataType t) {
		switch (t){
			case BOOLEAN_VALUE: set(getBoolean()); break;
			case INTEGER_VALUE: set(getInteger()); break;
			case REAL_VALUE: set(getReal()); break;
			case DEVICE_LINK: break;
		}
		type = t;
	}

	//link infos
	std::vector<std::shared_ptr<ioLink>>& getLinks() { return ioLinks; }
	bool isConnected() { return !ioLinks.empty(); }
	bool acceptsMultipleInputs() { return b_acceptsMultipleInputs; }
	bool hasMultipleLinks() { return ioLinks.size() > 1; }
	bool isDataTypeCompatible(std::shared_ptr<ioData> otherData);
	
	//nodegraph infos
	int getUniqueID() { return uniqueID; }
	std::shared_ptr<ioNode> getNode() { return parentNode; }
	bool& isVisible() { return b_visible; }

	//datatype infos
	bool isBool()				{ return type == BOOLEAN_VALUE; }
	bool isInteger()			{ return type == INTEGER_VALUE; }
	bool isDouble()				{ return type == REAL_VALUE; }
	bool isDeviceLink()			{ return type == DEVICE_LINK; }

	//setting data (with data conversions)
	void set(bool boolean);
	void set(long long int integer);
	void set(double real);

	//reading data (with data conversions)
	bool getBoolean();
	long long int getInteger();
	double getReal();

	const char* getValueString();

	std::vector<std::shared_ptr<ioNode>> getNodesLinkedAtOutputs();
	std::vector<std::shared_ptr<ioNode>> getNodesLinkedAtInputs();

	float getGuiWidth();
	void pinGui();
	bool shouldDisplayDataGui();
	void dataGui();

	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	bool matches(const char* name, const char* dataTypeString);

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
	bool b_disablePin = false;
	bool b_noDataField = false;
	bool b_forceDataField = false;
	bool b_disableDataField = false;

	union {
		bool booleanValue;
		long long int integerValue;
		double realValue;
	};
};