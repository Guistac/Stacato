#pragma once

#include "NodeGraph/Node.h"
#include "Motion/Subdevice.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Utilities/CircularBuffer.h"

class Axis : public Node {
public:

	DEFINE_AXIS_NODE("Axis", Axis);

	//Device Links
	std::shared_ptr<NodePin> actuatorDeviceLinks = std::make_shared<NodePin>(NodeData::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuators", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> feedbackDeviceLink = std::make_shared<NodePin>(NodeData::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Encoder");
	std::shared_ptr<NodePin> referenceDeviceLinks = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Reference Devices", NodePinFlags_AcceptMultipleInputs);

	//Inputs
	std::shared_ptr<NodePin> positionFeedback = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> positionReferences = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "References", NodePinFlags_AcceptMultipleInputs);

	//Outputs
	std::shared_ptr<NodePin> actuatorCommand = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> resetPositionFeedback = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Reset Position Feedback", NodePinFlags_DisableDataField | NodePinFlags_HidePin);


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

	//Reference Deviation and Homing Velocity
	double homingVelocity_degreesPerSecond = 0.0;
	double allowedPositiveDeviationFromReference_degrees = 0.0;
	double allowedNegativeDeviationFromReference_degrees = 0.0;

	//Default Manual Movement
	double defaultManualVelocity_degreesPerSecond = 10.0;
	double defaultManualAcceleration_degreesPerSecondSquared = 5.0;

	//============== CONTROL VARIABLES ===================

	//motion profile generator variables
	double previousProfilePointTime_seconds = 0.0; //used to calculate deltaT
	double currentProfilePointTime_seconds = 0.0;
	double currentProfilePointDeltaT_seconds = 0.0;
	double profilePosition_degrees = 0.0;
	double profileVelocity_degreesPerSecond = 0.0;
	double profileAcceleration_degreesPerSecondSquared = 0.0;

	enum class ControlMode {
		VELOCITY_TARGET,
		POSITION_TARGET,
		FOLLOW_CURVE,
		HOMING
	};

	ControlMode controlMode = ControlMode::VELOCITY_TARGET;

	double manualControlAcceleration_degreesPerSecond = 0.0;

	//Manual Velocity Control
	void setVelocity(double velocity_axisUnits);
	void velocityTargetControl();
	float manualVelocityTarget_degreesPerSecond = 0.0;

	//Manual Target Control
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits);
	void positionTargetControl();
	MotionCurve::CurveProfile targetCurveProfile;
	double targetPosition = 0.0;
	double targetVelocity = 0.0;
	double targetTime = 0.0;

	//Curve following
	void followCurveControl();

	//Homing Control
	void startHoming();
	void cancelHoming();
	bool isHoming();
	void homingControl();

	const size_t historyLength = 2048;
	CircularBuffer positionHistory = CircularBuffer(historyLength);
	CircularBuffer actualPositionHistory = CircularBuffer(historyLength);
	CircularBuffer positionErrorHistory = CircularBuffer(historyLength);
	CircularBuffer velocityHistory = CircularBuffer(historyLength);
	CircularBuffer accelerationHistory = CircularBuffer(historyLength);
	CircularBuffer loadHistory = CircularBuffer(historyLength);

	//Axis State Control
	void enable();
	void onEnable();
	void disable();
	bool isEnabled();
	bool b_enabled = false;
	bool areAllDevicesReady();
	void enableAllActuators();
	void disableAllActuators();

	const char* getAxisUnitStringSingular() { return getPositionUnitType(axisPositionUnit)->displayName; }
	const char* getAxisUnitStringPlural() { return getPositionUnitType(axisPositionUnit)->displayNamePlural; }

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
	void controlsGui();
	void settingsGui();
	void devicesGui();
	void metricsGui();

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);

};

























class StateMachineAxis : public Node {
public:

	DEFINE_AXIS_NODE("State Machine Axis", StateMachineAxis);

	std::shared_ptr<NodePin> deviceLink = std::make_shared<NodePin>(NodeData::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuators", NodePinFlags_AcceptMultipleInputs);
	std::shared_ptr<NodePin> state0ref = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 0 Feedback");
	std::shared_ptr<NodePin> state1ref = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 1 Feedback");
	std::shared_ptr<NodePin> state2ref = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "State 2 Feedback");

	std::shared_ptr<NodePin> state0Command = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> state1Command = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", NodePinFlags_DisableDataField);
	std::shared_ptr<NodePin> state2Command = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "State 0 Command", NodePinFlags_DisableDataField);

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

