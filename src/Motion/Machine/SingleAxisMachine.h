#pragma once

#include "Machine.h"

class SingleAxisMachine : public Machine {
public:

	DEFINE_AXIS_NODE("Single Axis Machine", SingleAxisMachine);

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

	//Machine Type
	UnitType machineUnitType = UnitType::LINEAR;
	PositionUnit::Unit machinePositionUnit = PositionUnit::Unit::METER;

	//Feedback Type
	PositionFeedback::Type positionFeedbackType = PositionFeedback::Type::ABSOLUTE_FEEDBACK;
	PositionUnit::Unit feedbackPositionUnit = PositionUnit::Unit::DEGREE;
	double feedbackUnitsPerMachineUnits = 0.0;

	//CommandType
	CommandType::Type commandType = CommandType::Type::POSITION_COMMAND;
	PositionUnit::Unit commandPositionUnit = PositionUnit::Unit::DEGREE;
	double commandUnitsPerMachineUnits = 0.0;

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
	void setVelocity(double velocity_machineUnits);
	void velocityTargetControl();
	float manualVelocityTarget_degreesPerSecond = 0.0;

	//Manual Target Control
	void moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits);
	void moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits);
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

	//Machine State Control
	void enable();
	void onEnable();
	void disable();
	void onDisable();
	bool isEnabled();
	bool b_enabled = false;
	bool areAllDevicesReady();
	bool areAllDevicesEnabled();
	void enableAllActuators();
	void disableAllActuators();

	const char* getMachineUnitStringSingular() { return getPositionUnitType(machinePositionUnit)->displayName; }
	const char* getMachineUnitStringPlural() { return getPositionUnitType(machinePositionUnit)->displayNamePlural; }

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

	virtual void controlsGui();
	virtual void settingsGui();
	virtual void devicesGui();
	virtual void metricsGui();
	virtual void miniatureGui() {};

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);

	virtual void machineStatusGui() {}
	virtual void machineControlsGui() {}

};