#pragma once

class ActuatorDevice;
class PositionFeedbackDevice;
class GpioDevice;
class ServoActuatorDevice;

class Node;
class NodeLink;

namespace tinyxml2 { class XMLElement; }

struct NodeData {
	enum Type {
		BOOLEAN_VALUE,
		INTEGER_VALUE,
		REAL_VALUE,
		ACTUATOR_DEVICELINK,
		POSITIONFEEDBACK_DEVICELINK,
		GPIO_DEVICELINK,
		SERVO_ACTUATOR_DEVICE_LINK
	};
	Type type;
	char displayName[64];
	char saveName[64];
};
extern std::vector<NodeData> NodeDataTypes;
std::vector<NodeData>& getNodeDataTypes();
NodeData* getNodeDataType(NodeData::Type type);
NodeData* getNodeDataType(const char* saveName);

enum DataDirection {
	NODE_INPUT,
	NODE_OUTPUT,
	NO_DIRECTION
};

enum NodePinFlags {
	NodePinFlags_None					= 0,
	NodePinFlags_AcceptMultipleInputs	= 1 << 0,
	NodePinFlags_DisablePin				= 1 << 1,
	NodePinFlags_NoDataField			= 1 << 2,
	NodePinFlags_ForceDataField			= 1 << 3,
	NodePinFlags_DisableDataField		= 1 << 4,
	NodePinFlags_HidePin				= 1 << 5
};

inline NodePinFlags operator|(NodePinFlags a, NodePinFlags b){
	return static_cast<NodePinFlags>(static_cast<int>(a) | static_cast<int>(b));
}

class NodePin {
public:

	NodePin(NodeData::Type t, DataDirection d, const char* displayN, const char* saveN, NodePinFlags flags) : type(t), direction(d) {
		setup(t, d, displayN, saveN, flags);
	}

	NodePin(NodeData::Type t, DataDirection d, const char* displayN, NodePinFlags flags) : type(t), direction(d) {
		setup(t, d, displayN, displayN, flags); //displayName is used as savename
	}

	NodePin(NodeData::Type t, DataDirection d, const char* displayN, const char* saveN) : type(t), direction(d) {
		setup(t, d, displayN, saveN, NodePinFlags_None);
	}

	NodePin(NodeData::Type t, DataDirection d, const char* displayN) : type(t), direction(d) {
		setup(t, d, displayN, displayN, NodePinFlags_None); //displayName is used as savename
	}

	NodePin() {} //default constructor is needed for dummy creation and later xml loading

	//data infos
	const char* getDisplayName() { return displayName; }
	const char* getSaveName() { return saveName; }
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }

	NodeData::Type getType() { return type; }
	bool isSameTypeAs(NodePin& other) { return other.type == type; }
	void setType(NodeData::Type t) {
		switch (t){
			case NodeData::Type::BOOLEAN_VALUE: set(getBoolean()); break;
			case NodeData::Type::INTEGER_VALUE: set(getInteger()); break;
			case NodeData::Type::REAL_VALUE: set(getReal()); break;
			case NodeData::Type::ACTUATOR_DEVICELINK: break;
			case NodeData::Type::POSITIONFEEDBACK_DEVICELINK: break;
			case NodeData::Type::GPIO_DEVICELINK: break;
			case NodeData::Type::SERVO_ACTUATOR_DEVICE_LINK: break;
		}
		type = t;
	}

	//link infos
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return NodeLinks; }
	std::vector<std::shared_ptr<NodePin>> getConnectedPins();
	bool isConnected() { return !NodeLinks.empty(); }
	bool acceptsMultipleInputs() { return b_acceptsMultipleInputs; }
	bool hasMultipleLinks() { return NodeLinks.size() > 1; }
	bool isDataTypeCompatible(std::shared_ptr<NodePin> otherData);
	
	//nodegraph infos
	int getUniqueID() { return uniqueID; }
	std::shared_ptr<Node> getNode() { return parentNode; }
	bool& isVisible() { return b_visible; }

	//datatype infos
	bool isBool()							{ return type == NodeData::Type::BOOLEAN_VALUE; }
	bool isInteger()						{ return type == NodeData::Type::INTEGER_VALUE; }
	bool isDouble()							{ return type == NodeData::Type::REAL_VALUE; }
	bool isActuatorDeviceLink()				{ return type == NodeData::Type::ACTUATOR_DEVICELINK; }
	bool isPositionFeedbackDeviceLink()		{ return type == NodeData::Type::POSITIONFEEDBACK_DEVICELINK; }
	bool isGpioDeviceLink()					{ return type == NodeData::Type::GPIO_DEVICELINK; }
	bool isServoActuatorDeviceLink()		{ return type == NodeData::Type::SERVO_ACTUATOR_DEVICE_LINK; }

	//setting data (with data conversions)
	void set(bool boolean);
	void set(long long int integer);
	void set(double real);
	void set(std::shared_ptr<ActuatorDevice>);
	void set(std::shared_ptr<PositionFeedbackDevice>);
	void set(std::shared_ptr<GpioDevice>);
	void set(std::shared_ptr<ServoActuatorDevice>);

	//reading data (with data conversions)
	bool getBoolean();
	long long int getInteger();
	double getReal();
	std::shared_ptr<ActuatorDevice> getActuatorDevice();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();
	std::shared_ptr<GpioDevice> getGpioDevice();
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice();

	const char* getValueString();

	std::vector<std::shared_ptr<Node>> getNodesLinkedAtOutputs();
	std::vector<std::shared_ptr<Node>> getNodesLinkedAtInputs();

	float getGuiWidth();
	void pinGui();
	bool shouldDisplayDataGui();
	void dataGui();

	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	bool matches(const char* saveName, NodeData::Type type);

private:

	friend class NodeGraph;
	friend class Node;
	friend class NodeLink;

	std::shared_ptr<Node> parentNode;
	std::vector<std::shared_ptr<NodeLink>> NodeLinks;
	int uniqueID = -1;
	bool b_visible = true;
	bool b_isEditingDisplayName = false;

	NodeData::Type type;
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
	std::shared_ptr<ServoActuatorDevice> servoActuatorDevice = nullptr;

	void setup(NodeData::Type t, DataDirection d, const char* displayN, const char* saveN, NodePinFlags flags) {
		strcpy(displayName, displayN);
		strcpy(saveName, saveN);
		b_acceptsMultipleInputs = flags & NodePinFlags_AcceptMultipleInputs;
		b_disablePin = flags & NodePinFlags_DisablePin;
		b_noDataField = flags & NodePinFlags_NoDataField;
		b_forceDataField = flags & NodePinFlags_ForceDataField;
		b_disableDataField = flags & NodePinFlags_DisableDataField;
		b_visible = !(flags & NodePinFlags_HidePin);
		switch (type) {
			case NodeData::Type::BOOLEAN_VALUE: booleanValue = false; break;
			case NodeData::Type::INTEGER_VALUE: integerValue = 0; break;
			case NodeData::Type::REAL_VALUE: realValue = 0.0; break;
			case NodeData::Type::ACTUATOR_DEVICELINK:
			case NodeData::Type::POSITIONFEEDBACK_DEVICELINK:
			case NodeData::Type::GPIO_DEVICELINK:
			case NodeData::Type::SERVO_ACTUATOR_DEVICE_LINK:
					b_noDataField = true;
					b_forceDataField = false;
					break;
		}
	}
};