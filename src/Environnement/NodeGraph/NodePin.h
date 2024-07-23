#pragma once

namespace Motion{ class Axis; }

class DeadMansSwitch;

class GpioInterface;
class MotionFeedbackInterface;
class ActuatorInterface;
class AxisInterface;

class Node;
class NodeLink;

namespace tinyxml2 { class XMLElement; }


class NodePin {
public:
	
	enum class DataType{
		BOOLEAN,
		INTEGER,
		REAL,
		DEAD_MANS_SWITCH,
		GPIO_INTERFACE,
		MOTIONFEEDBACK_INTERFACE,
		ACTUATOR_INTERFACE,
		AXIS_INTERFACE,
		AXIS
	};
	
	enum class Direction {
		NODE_INPUT,
		NODE_OUTPUT,
		NODE_INPUT_BIDIRECTIONAL,
		NODE_OUTPUT_BIDIRECTIONAL,
		NO_DIRECTION
	};
	
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
	
	//Constructors with predeclared data type
	
	NodePin(DataType type, Direction dir, const char* displayN, const char* saveN, Flags flags = Flags::None) {
		initialize(dir, displayN, saveN, flags);
		setType(type);
	}
	
	NodePin(DataType type, Direction dir, const char* displayN, Flags flags = Flags::None){
		initialize(dir, displayN, displayN, flags);
		setType(type);
	}
	
	//Templated constructors with direct data assignement
	
	template<typename T>
	NodePin(std::shared_ptr<T> dataPointer, Direction dir, const char* displayN, Flags flags = Flags::None){
		initialize(dir, displayN, displayN, flags);
		setType(detectType(dataPointer));
		pointer = dataPointer;
	}
	
	template<typename T>
	NodePin(std::shared_ptr<T> dataPointer, Direction dir, const char* displayN, const char* saveN, Flags flags = Flags::None){
		initialize(dir, displayN, saveN, flags);
		detectType(dataPointer);
		setType(detectType(dataPointer));
		pointer = dataPointer;
	}

	//default constructor is needed for dummy creation and later xml loading
	NodePin() {}
	
	//==============================================================
	//============== Data Assigning / Reading & Writing ============
	//==============================================================
	
	//Assigns data and sets datatype enumerator value
	//needs a template specialization for each supported type
	template<typename T>
	void assignData(std::shared_ptr<T> dataPointer);
	
	//Get a value directly or through implicit conversion
	//needs a template specialization for each supported type
	template<typename T>
	inline T read();
	
	template<typename T>
	inline void write(T data);
	
	//Get a shared pointer of the specified type
	template<typename T>
	const std::shared_ptr<T> getSharedPointer(){
		return std::static_pointer_cast<T>(pointer.lock());
	}
	
	inline void copyConnectedPinValue();
	
	void updateConnectedPins();
	
	//========================================================
	
	//strings
	const char* getDisplayString() { return displayString; }
	const char* getSaveString() { return saveString; }
	const char* getValueString();
	
	//info
	bool isInput() { return direction == Direction::NODE_INPUT || direction == Direction::NODE_INPUT_BIDIRECTIONAL; }
	bool isOutput() { return direction == Direction::NODE_OUTPUT || direction == Direction::NODE_OUTPUT_BIDIRECTIONAL; }
	bool isBidirectional(){ return direction == Direction::NODE_INPUT_BIDIRECTIONAL || direction == Direction::NODE_OUTPUT_BIDIRECTIONAL; }
	bool isConnected() { return !nodeLinks.empty(); }
	bool hasMultipleConnections() { return nodeLinks.size() > 1; }
	bool acceptsMultipleInputs() { return b_acceptsMultipleInputs; }
	bool& isVisible() { return b_visible; }
	bool isDataTypeCompatible(std::shared_ptr<NodePin> other);
	
	//connections
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return nodeLinks; }
	std::vector<std::shared_ptr<NodePin>> getConnectedPins();
	std::shared_ptr<NodePin> getConnectedPin();
	
	//commands
	void disconnectAllLinks();
	void setVisible(bool v) { b_visible = v; }
	
	//nodegraph
	int getUniqueID() { return uniqueID; }
	std::shared_ptr<Node> getNode() { return parentNode; }

	//gui
	float getGuiWidth();
	bool shouldDisplayDataGui();
	
	void pinGui();
	void dataGui();

	//saving & loading
	bool save(tinyxml2::XMLElement* xml);
	bool load(tinyxml2::XMLElement* xml);
	bool matches(const char* saveName, DataType type);
	

	
	void initialize(Direction dir, const char* displayStr, const char* saveStr, Flags flags) {
		direction = dir;
		strcpy(displayString, displayStr);
		strcpy(saveString, saveStr);
		b_acceptsMultipleInputs = flags & Flags::AcceptMultipleInputs;
		b_disablePin = flags & Flags::DisablePin;
		b_noDataField = flags & Flags::NoDataField;
		b_forceDataField = flags & Flags::ForceDataField;
		b_disableDataField = flags & Flags::DisableDataField;
		b_visible = !(flags & Flags::HidePin);
	}
	
	template<typename T>
	DataType detectType(std::shared_ptr<T> dataPointer);
	
	void setType(DataType type){
		dataType = type;
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

	std::shared_ptr<Node> parentNode; //set when assigning when calling addNodePin();
	int uniqueID = -1;
	
	char displayString[64];	//used for displaying
	char saveString[64];	//used for matching
	
	bool b_acceptsMultipleInputs = false;
	bool b_disablePin = false;
	bool b_noDataField = false;
	bool b_forceDataField = false;
	bool b_disableDataField = false;
	bool b_visible = true;
	
	//actual data pointer, type and direction
	std::weak_ptr<void> pointer;
	DataType dataType;
	Direction direction;
	std::vector<std::shared_ptr<NodeLink>> nodeLinks;
	
	//========= ERROR Handling =========

	template<typename T>
	bool logTypeMismatchError(std::shared_ptr<T> assignedData){
		return Logger::critical("Assigning Wrong NodePin DataType to Pin '{}'. Pin DataType is {} while assigned DataType is {}",
								getDisplayString(),
								Enumerator::getDisplayString(dataType),
								Enumerator::getDisplayString(detectType(assignedData)));
	}
	
	bool logReadNullPointerError(){
		return Logger::critical("Cannot Read from NodePin '{}' because its data pointer is Null", getDisplayString());
	}
	
	bool logWriteNullPointerError(){
		return Logger::critical("Cannot Write to NodePin '{}' because its data pointer is Null", getDisplayString());
	}
	
	bool logNoConnectionError(){
		return Logger::critical("Could not complete operation because no pins are connected to NodePin '{}'", getDisplayString());
	}
};





//==============================================================
//========================= Enumerators ========================
//==============================================================

inline NodePin::Flags operator|(NodePin::Flags a, NodePin::Flags b) {
	return static_cast<NodePin::Flags>(static_cast<int>(a) | static_cast<int>(b));
}

#define NodePinDataTypes \
	{NodePin::DataType::BOOLEAN, 					.displayString = "Boolean", 					.saveString = "Boolean"},\
	{NodePin::DataType::INTEGER, 					.displayString = "Integer", 					.saveString = "Integer"},\
	{NodePin::DataType::REAL, 						.displayString = "Real", 						.saveString = "Real"},\
	{NodePin::DataType::DEAD_MANS_SWITCH, 			.displayString = "Dead Man's Switch", 			.saveString = "DeadMansSwitchLink"},\
	{NodePin::DataType::GPIO_INTERFACE,				.displayString = "Gpio Module",					.saveString = "GpioInterface"},\
	{NodePin::DataType::MOTIONFEEDBACK_INTERFACE,	.displayString = "Motion Feedback Interface",	.saveString = "MotionFeedbackInterface"},\
	{NodePin::DataType::ACTUATOR_INTERFACE,			.displayString = "Actuator Interface",			.saveString = "ActuatorInterface"},\
	{NodePin::DataType::AXIS_INTERFACE,				.displayString = "Axis Interface",				.saveString = "AxisInterface"},\
	{NodePin::DataType::AXIS,						.displayString = "Axis",						.saveString = "Axis"}\

DEFINE_ENUMERATOR(NodePin::DataType, NodePinDataTypes)


//==============================================================
//=========== Direct Reading with Implicit Conversion ==========
//==============================================================

//these are not to be specialized for data types without implicit conversion ability

template<>
inline bool NodePin::read(){
	if(pointer.expired()) return logReadNullPointerError();
	switch(dataType){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return false;
	}
}

template<>
inline int NodePin::read(){
	if(pointer.expired()) return logReadNullPointerError();
	switch(dataType){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0;
	}
}

template<>
inline double NodePin::read(){
	if(pointer.expired()) return logReadNullPointerError();
	switch(dataType){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0.0;
	}
}

inline void NodePin::copyConnectedPinValue(){
	if(!isConnected()) {
		logNoConnectionError();
		return;
	}
	switch(dataType){
		case DataType::BOOLEAN:
			*getSharedPointer<bool>() = getConnectedPin()->read<bool>();
			break;
		case DataType::INTEGER:
			*getSharedPointer<int>() = getConnectedPin()->read<int>();
			break;
		case DataType::REAL:
			*getSharedPointer<double>() = getConnectedPin()->read<double>();
			break;
		default:
			break;
	}
}


//==============================================================
//=========== Direct Writing with Implicit Conversion ==========
//==============================================================

//these are not to be specialized for data types without implicit conversion ability

template<>
inline void NodePin::write(bool data){
	if(pointer.expired()) {
		logWriteNullPointerError();
		return;
	}
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
inline void NodePin::write(int data){
	if(pointer.expired()) {
		logWriteNullPointerError();
		return;
	}
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
inline void NodePin::write(double data){
	if(pointer.expired()) {
		logWriteNullPointerError();
		return;
	}
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



//==============================================================
//================== Templated Type Detection ==================
//==============================================================


template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<bool> ptr) { return DataType::BOOLEAN; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<int> ptr) { return DataType::INTEGER; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<double> ptr) { return DataType::REAL; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<DeadMansSwitch> ptr) { return DataType::DEAD_MANS_SWITCH; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<GpioInterface> ptr) { return DataType::GPIO_INTERFACE; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<MotionFeedbackInterface> ptr) { return DataType::MOTIONFEEDBACK_INTERFACE; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<ActuatorInterface> ptr) { return DataType::ACTUATOR_INTERFACE; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<AxisInterface> ptr) { return DataType::AXIS_INTERFACE; }

template<>
inline NodePin::DataType NodePin::detectType(std::shared_ptr<Motion::Axis> ptr) { return DataType::AXIS; }

//==============================================================
//====================== Data Assignement ======================
//==============================================================


template<>
inline void NodePin::assignData(std::shared_ptr<bool> ptr) {
	if(dataType != DataType::BOOLEAN) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<int> ptr) {
	if(dataType != DataType::INTEGER) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<double> ptr) {
	if(dataType != DataType::REAL) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<GpioInterface> ptr) {
	if(dataType != DataType::GPIO_INTERFACE) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<MotionFeedbackInterface> ptr) {
	if(dataType != DataType::MOTIONFEEDBACK_INTERFACE) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<ActuatorInterface> ptr) {
	if(dataType != DataType::ACTUATOR_INTERFACE) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<AxisInterface> ptr) {
	if(dataType != DataType::AXIS_INTERFACE) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}

template<>
inline void NodePin::assignData(std::shared_ptr<DeadMansSwitch> ptr) {
	if(dataType != DataType::DEAD_MANS_SWITCH) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}


template<>
inline void NodePin::assignData(std::shared_ptr<Motion::Axis> ptr) {
	if(dataType != DataType::AXIS) {
		logTypeMismatchError(ptr);
		return;
	}
	pointer = ptr;
}
