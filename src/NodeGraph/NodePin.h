#pragma once

class ActuatorDevice;
class PositionFeedbackDevice;
class GpioDevice;
class ServoActuatorDevice;
class VelocityControlledAxis;
class PositionControlledAxis;
class DeadMansSwitch;

class Node;
class NodeLink;

namespace tinyxml2 { class XMLElement; }




class NodePin {
public:
	
	enum class DataType{
		BOOLEAN,
		INTEGER,
		REAL,
		ACTUATOR,
		POSITIONFEEDBACK,
		GPIO,
		SERVO_ACTUATOR,
		VELOCITY_CONTROLLED_AXIS,
		POSITION_CONTROLLED_AXIS,
		DEAD_MANS_SWITCH
	};
	
	enum class DataDirection {
		NODE_INPUT,
		NODE_OUTPUT,
		NO_DIRECTION
	};
	
	//TODO: check if this can be a scoped enum with string enumerator
	enum Flags {
		None = 0,
		AcceptMultipleInputs = 1 << 0,
		DisablePin = 1 << 1,
		NoDataField = 1 << 2,
		ForceDataField = 1 << 3,
		DisableDataField = 1 << 4,
		HidePin = 1 << 5
	};
	
	//==============================================================
	//========================= Constructors =======================
	//==============================================================
	
	//displayName, saveNamen, flags
	NodePin(DataDirection d, const char* displayN, const char* saveN, Flags flags) : direction(d) {
		initialize(d, displayN, saveN, flags);
	}

	//displayName, flags
	NodePin(DataDirection d, const char* displayN, Flags flags) : direction(d) {
		initialize(d, displayN, displayN, flags); //displayName is used as savename
	}

	//displayName, saveName
	NodePin(DataDirection d, const char* displayN, const char* saveN) : direction(d) {
		initialize(d, displayN, saveN, Flags::None);
	}

	//displayName
	NodePin(DataDirection d, const char* displayN) : direction(d) {
		initialize(d, displayN, displayN, Flags::None); //displayName is used as savename
	}

	//default constructor is needed for dummy creation and later xml loading
	NodePin() {}
	
	//==============================================================
	//==================== Data Setting & Getting ==================
	//==============================================================
	
	//Assigns data and sets datatype enumerator value
	//needs a template specialization for each supported type
	template<typename T>
	void assignData(std::shared_ptr<T> dataPointer){
		setPointer(dataPointer);
		switch (type) {
			case DataType::BOOLEAN:
			case DataType::INTEGER:
			case DataType::REAL:
				break;
			default:
				b_noDataField = true;
				b_forceDataField = false;
				break;
		}
	}
	
	//Get a shared pointer of the specified type
	template<typename T>
	const std::shared_ptr<T> getSharedPointer(){
		return std::static_pointer_cast<T>(pointer.lock());
	}
	
	//Get a value directly or through implicit conversion
	//needs a template specialization for each supported type
	template<typename T>
	T get();
	
	//return type compatibility
	//indicate if type is the same or implicit conversion is possible
	bool canConnectTo(std::shared_ptr<NodePin> other){
		switch(type){
			case DataType::BOOLEAN:
			case DataType::INTEGER:
			case DataType::REAL:
				switch(other->type){
					case DataType::BOOLEAN:
					case DataType::INTEGER:
					case DataType::REAL:
						return true;
					default:
						return false;
				}
			default:
				return other->type == type;
		}
	}
	
	//data infos
	const char* getDisplayString() { return displayString; }
	const char* getSaveString() { return saveString; }
	
	bool isInput() { return direction == DataDirection::NODE_INPUT; }
	bool isOutput() { return direction == DataDirection::NODE_OUTPUT; }
	
	//link infos
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return NodeLinks; }
	std::vector<std::shared_ptr<NodePin>> getConnectedPins();
	bool isConnected() { return !NodeLinks.empty(); }
	bool acceptsMultipleInputs() { return b_acceptsMultipleInputs; }
	bool hasMultipleLinks() { return NodeLinks.size() > 1; }
	bool isDataTypeCompatible(std::shared_ptr<NodePin> otherData);

	//commands
	void disconnectAllLinks();

	//nodegraph infos
	int getUniqueID() { return uniqueID; }
	std::shared_ptr<Node> getNode() { return parentNode; }

	void setVisible(bool v) { b_visible = v; }
	bool& isVisible() { return b_visible; }

	const char* getValueString();

	std::vector<std::shared_ptr<Node>> getNodesLinkedAtOutputs();
	std::vector<std::shared_ptr<Node>> getNodesLinkedAtInputs();

	float getGuiWidth();
	void pinGui();
	bool shouldDisplayDataGui();
	void dataGui();

	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	bool matches(const char* saveName, DataType type);
	
private:
	
	friend class NodeGraph;
	friend class Node;
	friend class NodeLink;

	std::shared_ptr<Node> parentNode;
	std::vector<std::shared_ptr<NodeLink>> NodeLinks;
	int uniqueID = -1;
	
	char displayString[64];	//used for displaying
	char saveString[64];	//used for matching
	
	DataDirection direction;
	
	bool b_acceptsMultipleInputs = false;
	bool b_disablePin = false;
	bool b_noDataField = false;
	bool b_forceDataField = false;
	bool b_disableDataField = false;
	bool b_visible = true;
	
	void initialize(DataDirection d, const char* displayStr, const char* saveStr, Flags flags) {
		strcpy(displayString, displayStr);
		strcpy(saveString, saveStr);
		b_acceptsMultipleInputs = flags & Flags::AcceptMultipleInputs;
		b_disablePin = flags & Flags::DisablePin;
		b_noDataField = flags & Flags::NoDataField;
		b_forceDataField = flags & Flags::ForceDataField;
		b_disableDataField = flags & Flags::DisableDataField;
		b_visible = !(flags & Flags::HidePin);
	}
	
	std::weak_ptr<void> pointer;
	DataType type;
	
	//Assigns data and sets datatype enumerator value
	//needs a template specialization for each supported type
	template<typename T>
	void setPointer(std::shared_ptr<T> sptr);
	
};







inline NodePin::Flags operator|(NodePin::Flags a, NodePin::Flags b) {
	return static_cast<NodePin::Flags>(static_cast<int>(a) | static_cast<int>(b));
}

#define NodePinDataTypes \
	{NodePin::DataType::BOOLEAN, 					.displayString = "Boolean", 					.saveString = "Boolean"},\
	{NodePin::DataType::INTEGER, 					.displayString = "Integer", 					.saveString = "Integer"},\
	{NodePin::DataType::REAL, 						.displayString = "Real", 						.saveString = "Real"},\
	{NodePin::DataType::ACTUATOR, 					.displayString = "Actuator", 					.saveString = "ActuatorDeviceLink"},\
	{NodePin::DataType::POSITIONFEEDBACK, 			.displayString = "Position Feedback", 			.saveString = "PositionFeedbackDeviceLink"},\
	{NodePin::DataType::GPIO, 						.displayString = "GPIO", 						.saveString = "GPIODeviceLink"},\
	{NodePin::DataType::SERVO_ACTUATOR, 			.displayString = "Servo Actuator", 				.saveString = "ServoActuatorDeviceLink"},\
	{NodePin::DataType::VELOCITY_CONTROLLED_AXIS, 	.displayString = "Velocity Controlled Axis", 	.saveString = "VelocityControlledAxisLink"},\
	{NodePin::DataType::POSITION_CONTROLLED_AXIS, 	.displayString = "Position Controlled Axis", 	.saveString = "PositionControlledAxisLink"},\
	{NodePin::DataType::DEAD_MANS_SWITCH, 			.displayString = "Dead Man's Switch", 			.saveString = "DeadMansSwitchLink"}\

DEFINE_ENUMERATOR(NodePin::DataType, NodePinDataTypes)







//==============================================================
//============= Direct Return / Implicit Conversion ============
//==============================================================

//these don't have to be defined for data types without implicit conversion ability

template<>
bool NodePin::get(){
	switch(type){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return false;
	}
}

template<>
int NodePin::get(){
	switch(type){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0;
	}
}

template<>
double NodePin::get(){
	switch(type){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0.0;
	}
}


//==============================================================
//======================== Data Setters ========================
//==============================================================

template<>
void NodePin::setPointer(std::shared_ptr<bool> booleanPointer) {
	pointer = booleanPointer;
	type = DataType::BOOLEAN;
}

template<>
void NodePin::setPointer(std::shared_ptr<int> integerPointer) {
	pointer = integerPointer;
	type = DataType::INTEGER;
}

template<>
void NodePin::setPointer(std::shared_ptr<double> realPointer) {
	pointer = realPointer;
	type = DataType::REAL;
}
