#pragma once

#include "NodeGraph/ioNode.h"
#include "Motion/Subdevice.h"
#include "Motion/MotionTypes.h"

enum ControlMode {
	VELOCITY_CONTROL,
	TARGET_CONTROL,
	FOLLOW_CURVE,
	HOMING
};

class Axis : public ioNode {
public:

	DEFINE_AXIS_NODE("Axis", Axis);

	//Device Links
	std::shared_ptr<ioData> actuatorDeviceLinks = std::make_shared<ioData>(DataType::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuators", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> feedbackDeviceLink = std::make_shared<ioData>(DataType::FEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Encoder");
	std::shared_ptr<ioData> referenceDeviceLinks = std::make_shared<ioData>(DataType::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Reference Devices", ioDataFlags_AcceptMultipleInputs);

	//Inputs
	std::shared_ptr<ioData> positionFeedback = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_INPUT, "Position Feedback");
	std::shared_ptr<ioData> positionReferences = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "References", ioDataFlags_AcceptMultipleInputs);

	//Outputs
	std::shared_ptr<ioData> actuatorCommand = std::make_shared<ioData>(DataType::REAL_VALUE, DataDirection::NODE_OUTPUT, "Command", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> resetPositionFeedback = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Reset Position Feedback", ioDataFlags_DisableDataField);


	//==================== AXIS DATA ====================

	//Axis Type
	UnitType axisUnitType = UnitType::LINEAR;
	PositionUnit::Unit axisPositionUnit = PositionUnit::Unit::METER;

	//Feedback Type
	PositionFeedback::Type positionFeedbackType = PositionFeedback::Type::ABSOLUTE_FEEDBACK;
	PositionUnit::Unit feedbackPositionUnit = PositionUnit::Unit::DEGREE;
	double feedbackUnitsPerAxisUnits = 0.0;

	//CommandType
	CommandType::Type commandType = CommandType::Type::POSITION_COMMAND;
	PositionUnit::Unit commandPositionUnit = PositionUnit::Unit::DEGREE;
	double commandUnitsPerAxisUnits = 0.0;

	//Reference and Homing Type
	PositionReference::Type positionReferenceType = PositionReference::Type::NO_LIMIT;
	HomingDirection::Type homingDirectionType = HomingDirection::Type::DONT_CARE;

	//Kinematic Limits
	double velocityLimit_degreesPerSecond = 0.0;
	double accelerationLimit_degreesPerSecondSquared = 0.0;

	//Default Manual Movement
	double defaultManualVelocity_degreesPerSecond = 10.0;
	double defaultManualAcceleration_degreesPerSecondSquared = 5.0;

	//Reference Deviation and Homing Velocity
	double homingVelocity_degreesPerSecond = 0.0;
	double allowedPositiveDeviationFromReference_degrees = 0.0;
	double allowedNegativeDeviationFromReference_degrees = 0.0;


	//============== CONTROL VARIABLES ===================


	ControlMode controlMode = VELOCITY_CONTROL;
	double velocityControlTarget_degreesPerSecond = 0.0;

	//motion profile generator variables
	double lastProfileUpdateTime_seconds = 0.0; //used to calculate deltaT
	double profilePosition_degrees = 0.0;
	double profileVelocity_degreesPerSecond = 0.0;
	double profileAcceleration_degreesPerSecondSquared = 0.0;

	//Axis State Control
	void enable();
	void onEnable();
	void disable();
	bool isEnabled();
	bool b_enabled = false;
	bool areAllDevicesReady();
	void enableAllActuators();
	void disableAllActuators();

	//Manual Control Commands
	void setVelocity(double velocity_axisUnits);
	void moveToPosition(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);

	//Homing Control
	void startHoming();
	void cancelHoming();
	bool isHoming();


	const char* getAxisUnitStringSingular() { return getPositionUnitType(axisPositionUnit)->displayName; }
	const char* getAxisUnitStringPlural() { return getPositionUnitType(axisPositionUnit)->displayNamePlural; }

	void controlsGui();
	void settingsGui();
	void devicesGui();

	virtual void assignIoData() {
		addIoData(actuatorDeviceLinks);
		addIoData(feedbackDeviceLink);
		addIoData(positionFeedback);
		addIoData(referenceDeviceLinks);
		addIoData(positionReferences);

		addIoData(actuatorCommand);
		addIoData(resetPositionFeedback);
	}

	virtual void process();

	virtual void nodeSpecificGui();

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);

};

























class StateMachineAxis : public ioNode {
public:

	DEFINE_AXIS_NODE("State Machine Axis", StateMachineAxis);

	std::shared_ptr<ioData> deviceLink = std::make_shared<ioData>(DataType::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuators", ioDataFlags_AcceptMultipleInputs);
	std::shared_ptr<ioData> state0ref = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 0 Feedback");
	std::shared_ptr<ioData> state1ref = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 1 Feedback");
	std::shared_ptr<ioData> state2ref = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 2 Feedback");

	std::shared_ptr<ioData> state0Command = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> state1Command = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", ioDataFlags_DisableDataField);
	std::shared_ptr<ioData> state2Command = std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", ioDataFlags_DisableDataField);

	virtual void assignIoData() {
		addIoData(deviceLink);
		addIoData(state0ref);
		addIoData(state1ref);
		addIoData(state2ref);

		addIoData(state0Command);
		addIoData(state1Command);
		addIoData(state2Command);
	}

	virtual void process() {}

	virtual void nodeSpecificGui() {}
	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }

};

