#pragma once

class ActuatorDevice;
class PositionFeedbackDevice;
class GpioDevice;

class ioNode;
class ioLink;

namespace tinyxml2 { class XMLElement; }

struct ioDataType {
	enum Type {
		BOOLEAN_VALUE,
		INTEGER_VALUE,
		REAL_VALUE,
		ACTUATOR_DEVICELINK,
		POSITIONFEEDBACK_DEVICELINK,
		GPIO_DEVICELINK
	};
	Type type;
	char displayName[64];
	char saveName[64];
};
extern std::vector<ioDataType> ioDataTypes;
ioDataType* getDataType(ioDataType::Type type);
ioDataType* getDataType(const char* saveName);

enum DataDirection {
	NODE_INPUT,
	NODE_OUTPUT,
	NO_DIRECTION
};

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

	ioData(ioDataType::Type t, DataDirection d, const char* displayN, const char* saveN, ioDataFlags flags) : type(t), direction(d) {
		setup(t, d, displayN, saveN, flags);
	}

	ioData(ioDataType::Type t, DataDirection d, const char* displayN, ioDataFlags flags) : type(t), direction(d) {
		setup(t, d, displayN, displayN, flags); //displayName is used as savename
	}

	ioData(ioDataType::Type t, DataDirection d, const char* displayN, const char* saveN) : type(t), direction(d) {
		setup(t, d, displayN, saveN, ioDataFlags_None);
	}

	ioData(ioDataType::Type t, DataDirection d, const char* displayN) : type(t), direction(d) {
		setup(t, d, displayN, displayN, ioDataFlags_None); //displayName is used as savename
	}

	ioData() {} //default constructor is needed for dummy creation and later xml loading

	//data infos
	const char* getDisplayName() { return displayName; }
	const char* getSaveName() { return saveName; }
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }

	ioDataType::Type getType() { return type; }
	const char* getTypeName() { 
		ioDataType* dataType = getDataType(type);
		if (dataType == nullptr) return "unknown";
		else return dataType->displayName;
	}
	bool isSameTypeAs(ioData& other) { return other.type == type; }
	void setType(ioDataType::Type t) {
		switch (t){
			case ioDataType::Type::BOOLEAN_VALUE: set(getBoolean()); break;
			case ioDataType::Type::INTEGER_VALUE: set(getInteger()); break;
			case ioDataType::Type::REAL_VALUE: set(getReal()); break;
			case ioDataType::Type::ACTUATOR_DEVICELINK: break;
			case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK: break;
			case ioDataType::Type::GPIO_DEVICELINK: break;
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
	bool isBool()							{ return type == ioDataType::Type::BOOLEAN_VALUE; }
	bool isInteger()						{ return type == ioDataType::Type::INTEGER_VALUE; }
	bool isDouble()							{ return type == ioDataType::Type::REAL_VALUE; }
	bool isActuatorDeviceLink()				{ return type == ioDataType::Type::ACTUATOR_DEVICELINK; }
	bool isPositionFeedbackDeviceLink()		{ return type == ioDataType::Type::POSITIONFEEDBACK_DEVICELINK; }
	bool isGpioDeviceLink()					{ return type == ioDataType::Type::GPIO_DEVICELINK; }

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

	ioDataType::Type type;
	DataDirection direction;
	char saveName[64];		//used for matching
	char displayName[64];	//used for displaying
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

	void setup(ioDataType::Type t, DataDirection d, const char* displayN, const char* saveN, ioDataFlags flags) {
		strcpy(displayName, displayN);
		strcpy(saveName, saveN);
		b_acceptsMultipleInputs = flags & ioDataFlags_AcceptMultipleInputs;
		b_disablePin = flags & ioDataFlags_DisablePin;
		b_noDataField = flags & ioDataFlags_NoDataField;
		b_forceDataField = flags & ioDataFlags_ForceDataField;
		b_disableDataField = flags & ioDataFlags_DisableDataField;
		b_visible = !(flags & ioDataFlags_HidePin);
		switch (type) {
			case ioDataType::Type::BOOLEAN_VALUE: booleanValue = false; break;
			case ioDataType::Type::INTEGER_VALUE: integerValue = 0; break;
			case ioDataType::Type::REAL_VALUE: realValue = 0.0; break;
			case ioDataType::Type::ACTUATOR_DEVICELINK:
			case ioDataType::Type::POSITIONFEEDBACK_DEVICELINK:
			case ioDataType::Type::GPIO_DEVICELINK:
					b_noDataField = true;
					b_forceDataField = false;
					break;
		}
	}
};