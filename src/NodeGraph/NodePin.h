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
	
	enum class Direction {
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
	NodePin(DataType type, Direction dir, const char* displayN, const char* saveN, Flags flags) {
		initialize(type, dir, displayN, saveN, flags);
	}

	//displayName, flags
	NodePin(DataType type, Direction dir, const char* displayN, Flags flags) {
		initialize(type, dir, displayN, displayN, flags); //displayName is used as savename
	}

	//displayName, saveName
	NodePin(DataType type, Direction dir, const char* displayN, const char* saveN) {
		initialize(type, dir, displayN, saveN, Flags::None);
	}

	//displayName
	NodePin(DataType type, Direction dir, const char* displayN) {
		initialize(type, dir, displayN, displayN, Flags::None); //displayName is used as savename
	}

	//default constructor is needed for dummy creation and later xml loading
	NodePin() {}
	
	//==============================================================
	//==================== Data Setting & Getting ==================
	//==============================================================
	
	//Assigns data and sets datatype enumerator value
	//needs a template specialization for each supported type
	template<typename T>
	void assignData(std::shared_ptr<T> dataPointer);
	
	//Get a shared pointer of the specified type
	template<typename T>
	const std::shared_ptr<T> getSharedPointer(){
		return std::static_pointer_cast<T>(pointer.lock());
	}
	
	//TODO: this should be called read and write
	
	//Get a value directly or through implicit conversion
	//needs a template specialization for each supported type
	template<typename T>
	inline T get();
	
	/*
	template<typename T>
	void set(T data);
	*/
	
	inline void updateValueFromConnectedPinValue();
	
	
	//TODO: function to assign value to pointer directly (bool int/double)
	
	
	
	//data infos
	const char* getDisplayString() { return displayString; }
	const char* getSaveString() { return saveString; }
	
	bool isInput() { return direction == Direction::NODE_INPUT; }
	bool isOutput() { return direction == Direction::NODE_OUTPUT; }
	
	//link infos
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return NodeLinks; }
	std::vector<std::shared_ptr<NodePin>> getConnectedPins();
	std::shared_ptr<NodePin> getConnectedPin();
	bool isConnected() { return !NodeLinks.empty(); }
	bool hasMultipleConnections() { return NodeLinks.size() > 1; }
	bool acceptsMultipleInputs() { return b_acceptsMultipleInputs; }
	bool isDataTypeCompatible(std::shared_ptr<NodePin> other){
		switch(dataType){
			case DataType::BOOLEAN:
			case DataType::INTEGER:
			case DataType::REAL:
				switch(other->dataType){
					case DataType::BOOLEAN:
					case DataType::INTEGER:
					case DataType::REAL:
						return true;
					default:
						return false;
				}
			default:
				return other->dataType == dataType;
		}
	}
	
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
	
	Direction direction;
	
	bool b_acceptsMultipleInputs = false;
	bool b_disablePin = false;
	bool b_noDataField = false;
	bool b_forceDataField = false;
	bool b_disableDataField = false;
	bool b_visible = true;
	
	void initialize(DataType type, Direction dir, const char* displayStr, const char* saveStr, Flags flags) {
		dataType = type;
		direction = dir;
		strcpy(displayString, displayStr);
		strcpy(saveString, saveStr);
		b_acceptsMultipleInputs = flags & Flags::AcceptMultipleInputs;
		b_disablePin = flags & Flags::DisablePin;
		b_noDataField = flags & Flags::NoDataField;
		b_forceDataField = flags & Flags::ForceDataField;
		b_disableDataField = flags & Flags::DisableDataField;
		b_visible = !(flags & Flags::HidePin);
		switch (dataType) {
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
	
	std::weak_ptr<void> pointer;
	DataType dataType;
	
	//Assigns data and sets datatype enumerator value
	//needs a template specialization for each supported type
	template<typename T>
	inline void setPointer(std::shared_ptr<T> sptr);
	
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
inline bool NodePin::get(){
	switch(dataType){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return false;
	}
}

template<>
inline int NodePin::get(){
	switch(dataType){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0;
	}
}

template<>
inline double NodePin::get(){
	switch(dataType){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0.0;
	}
}

inline void NodePin::updateValueFromConnectedPinValue(){
	switch(dataType){
		case DataType::BOOLEAN:
			*getSharedPointer<bool>() = getConnectedPin()->get<bool>();
			break;
		case DataType::INTEGER:
			*getSharedPointer<int>() = getConnectedPin()->get<int>();
			break;
		case DataType::REAL:
			*getSharedPointer<double>() = getConnectedPin()->get<double>();
			break;
		default:
			break;
	}
}

/*
template<>
void NodePin::set(bool data){
	switch(dataType){
		case DataType::BOOLEAN:
			*getSharedPointer<bool>() = data;
			break;
		case DataType::INTEGER:
			*getSharedPointer<int>() = data;
			break;
		case DataType::REAL:
			*getSharedPointer<double>() = data;
			break;
		default:
			break;
	}
}

template<>
void NodePin::set(int data){
	switch(dataType){
		case DataType::BOOLEAN:
			*getSharedPointer<bool>() = data;
			break;
		case DataType::INTEGER:
			*getSharedPointer<int>() = data;
			break;
		case DataType::REAL:
			*getSharedPointer<double>() = data;
			break;
		default:
			break;
	}
}

template<>
void NodePin::set(double data){
	switch(dataType){
		case DataType::BOOLEAN:
			*getSharedPointer<bool>() = data;
			break;
		case DataType::INTEGER:
			*getSharedPointer<int>() = data;
			break;
		case DataType::REAL:
			*getSharedPointer<double>() = data;
			break;
		default:
			break;
	}
}
 */


//==============================================================
//======================== Data Setters ========================
//==============================================================

template<>
inline void NodePin::assignData(std::shared_ptr<bool> booleanPointer) {
	if(dataType != DataType::BOOLEAN) return;
	pointer = booleanPointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<int> integerPointer) {
	if(dataType != DataType::INTEGER) return;
	pointer = integerPointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<double> realPointer) {
	if(dataType != DataType::REAL) return;
	pointer = realPointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<ActuatorDevice> actuatorDevicePointer) {
	if(dataType != DataType::ACTUATOR) return;
	pointer = actuatorDevicePointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<PositionFeedbackDevice> positionFeedbackDevicePointer) {
	if(dataType != DataType::POSITIONFEEDBACK) return;
	pointer = positionFeedbackDevicePointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<GpioDevice> gpioDevicePointer) {
	if(dataType != DataType::GPIO) return;
	pointer = gpioDevicePointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<ServoActuatorDevice> servoActuatorDevicePointer) {
	if(dataType != DataType::SERVO_ACTUATOR) return;
	pointer = servoActuatorDevicePointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<VelocityControlledAxis> velocityControlledAxisPointer) {
	if(dataType != DataType::VELOCITY_CONTROLLED_AXIS) return;
	pointer = velocityControlledAxisPointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<PositionControlledAxis> positionControlledAxisPointer) {
	if(dataType != DataType::POSITION_CONTROLLED_AXIS) return;
	pointer = positionControlledAxisPointer;
}

template<>
inline void NodePin::assignData(std::shared_ptr<DeadMansSwitch> deadMansSwitchPointer) {
	if(dataType != DataType::DEAD_MANS_SWITCH) return;
	pointer = deadMansSwitchPointer;
}
