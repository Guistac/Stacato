#pragma once

class ActuatorDevice;
class PositionFeedbackDevice;
class GpioDevice;

enum DataType {
	BOOLEAN_VALUE,
	INTEGER_VALUE,
	REAL_VALUE,
	ACTUATOR_DEVICELINK,
	POSITIONFEEDBACK_DEVICELINK,
	GPIO_DEVICELINK,
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
	ioDataFlags_DisableDataField		= 1 << 4,
	ioDataFlags_HidePin					= 1 << 5
};

inline ioDataFlags operator|(ioDataFlags a, ioDataFlags b){
	return static_cast<ioDataFlags>(static_cast<int>(a) | static_cast<int>(b));
}

class ioData {
public:

	ioData(DataType t, DataDirection d, const char* n, ioDataFlags flags) : type(t), direction(d) {
		b_acceptsMultipleInputs = flags & ioDataFlags_AcceptMultipleInputs;
		b_disablePin = flags & ioDataFlags_DisablePin;
		b_noDataField = flags & ioDataFlags_NoDataField;
		b_forceDataField = flags & ioDataFlags_ForceDataField;
		b_disableDataField = flags & ioDataFlags_DisableDataField;
		b_visible = !(flags & ioDataFlags_HidePin);
		setup(t, d, n);
	}

	ioData(DataType t, DataDirection d, const char* n) : type(t), direction(d) {
		setup(t, d, n);
	}

	ioData() {}

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
			case ACTUATOR_DEVICELINK: "Actuator Device";
			case POSITIONFEEDBACK_DEVICELINK: "Position Feedback Device";
			case GPIO_DEVICELINK: "Reference Device";
			default: return "unknown";
		}
	}
	bool isSameTypeAs(ioData& other) { return other.type == type; }
	void setType(DataType t) {
		switch (t){
			case BOOLEAN_VALUE: set(getBoolean()); break;
			case INTEGER_VALUE: set(getInteger()); break;
			case REAL_VALUE: set(getReal()); break;
			case ACTUATOR_DEVICELINK: break;
			case POSITIONFEEDBACK_DEVICELINK: break;
			case GPIO_DEVICELINK: break;
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
	bool isActuatorDeviceLink()		{ return type == ACTUATOR_DEVICELINK; }
	bool isPositionFeedbackDeviceLink()		{ return type == POSITIONFEEDBACK_DEVICELINK; }
	bool isGpioDeviceLink()	{ return type == GPIO_DEVICELINK; }

	//setting data (with data conversions)
	void set(bool boolean);
	void set(long long int integer);
	void set(double real);
	void set(std::shared_ptr<ActuatorDevice>);
	void set(std::shared_ptr<PositionFeedbackDevice>);
	void set(std::shared_ptr<GpioDevice>);

	//reading data (with data conversions)
	bool getBoolean();
	long long int getInteger();
	double getReal();
	std::shared_ptr<ActuatorDevice> getActuatorDevice();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();
	std::shared_ptr<GpioDevice> getGpioDevice();

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

	
	//ACTUAL DATA
	bool booleanValue = false;
	long long int integerValue = 0;
	double realValue = 0.0;
	std::shared_ptr<ActuatorDevice> actuatorDevice = nullptr;
	std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevice = nullptr;
	std::shared_ptr<GpioDevice> gpioDevice = nullptr;

	void setup(DataType t, DataDirection d, const char* n) {
		strcpy(name, n);
		switch (type) {
			case BOOLEAN_VALUE: booleanValue = false; break;
			case INTEGER_VALUE: integerValue = 0; break;
			case REAL_VALUE: realValue = 0.0; break;
			case ACTUATOR_DEVICELINK:
			case POSITIONFEEDBACK_DEVICELINK:
			case GPIO_DEVICELINK:
				b_noDataField = true;
				b_forceDataField = false;
				break;
		}
	}
};