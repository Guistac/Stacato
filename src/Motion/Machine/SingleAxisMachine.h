#pragma once

#include "Machine.h"

class SingleAxisMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(SingleAxisMachine, "Single Axis Machine", "SingleAxisMachine", Machine::Type::SINGLE_AXIS_MACHINE);

	//Device Links
	std::shared_ptr<NodePin> actuatorDeviceLink = std::make_shared<NodePin>(NodeData::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuator");
	std::shared_ptr<NodePin> positionFeedbackDeviceLink = std::make_shared<NodePin>(NodeData::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> referenceDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Reference Device");

	//Inputs
	std::shared_ptr<NodePin> positionReferences = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "References", NodePinFlags_AcceptMultipleInputs);

	//Outputs
	//std::shared_ptr<NodePin> actuatorCommand = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Actuator Command", NodePinFlags_DisableDataField);
	//std::shared_ptr<NodePin> resetPositionFeedback = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Reset Position Feedback", NodePinFlags_DisableDataField | NodePinFlags_HidePin);

	//==================== AXIS DATA ====================

	//Machine Type
	PositionUnit::Type machinePositionUnitType = PositionUnit::Type::LINEAR;
	PositionUnit::Unit machinePositionUnit = PositionUnit::Unit::METER;
	MotionControlType::Type motionControlType = MotionControlType::Type::CLOSED_LOOP_CONTROL;

	//Unit Conversions
	double feedbackUnitsPerMachineUnits = 0.0;
	double actuatorUnitsPerMachineUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference and Homing Type
	PositionLimitType::Type positionLimitType = PositionLimitType::Type::NO_LIMIT;
	HomingDirection::Type homingDirectionType = HomingDirection::Type::NEGATIVE;

	//Kinematic Limits
	double velocityLimit_machineUnitsPerSecond = 0.0;
	double accelerationLimit_machineUnitsPerSecondSquared = 0.0;

	//Reference Deviation and Homing Velocity
	double homingVelocity_machineUnitsPerSecond = 0.0;
	double allowedPositiveDeviationFromReference_machineUnits = 0.0;
	double allowedNegativeDeviationFromReference_machineUnits = 0.0;

	//Default Manual Movement
	double defaultManualVelocity_machineUnitsPerSecond = 10.0;
	double defaultManualAcceleration_machineUnitsPerSecondSquared = 5.0;

	//============== CONTROL VARIABLES ===================

	//motion profile generator variables
	double previousProfilePointTime_seconds = 0.0; //used to calculate deltaT
	double currentProfilePointTime_seconds = 0.0;
	double currentProfilePointDeltaT_seconds = 0.0;
	double profilePosition_machineUnits = 0.0;
	double profileVelocity_machineUnitsPerSecond = 0.0;
	double profileAcceleration_machineUnitsPerSecondSquared = 0.0;

	//actual machine state based on feedback data
	double actualPosition_machineUnits;
	double actualVelocity_machineUnitsPerSecond;

	enum class ControlMode {
		VELOCITY_TARGET,
		POSITION_TARGET,
		FOLLOW_CURVE,
		HOMING
	};

	ControlMode controlMode = ControlMode::VELOCITY_TARGET;

	double manualControlAcceleration_machineUnitsPerSecond = 0.0;

	//Manual Velocity Control
	void setVelocity(double velocity_machineUnits);
	void velocityTargetControl();
	float manualVelocityTarget_machineUnitsPerSecond = 0.0;

	//Manual Target Control
	void moveToPositionWithVelocity(double position_machineUnits, double velocity_machineUnits, double acceleration_machineUnits);
	void moveToPositionInTime(double position_machineUnits, double movementTime_seconds, double acceleration_machineUnits);
	void positionTargetControl();
	MotionCurve::CurveProfile targetCurveProfile;
	double targetPosition_machineUnits = 0.0;
	double targetVelocity_machineUnitsPerSecond = 0.0;
	double targetTime_seconds = 0.0;

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
	void onEnable();
	bool b_enabled = false;

	bool needsPositionFeedbackDevice();
	bool isPositionFeedbackDeviceConnected();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();
	double getPositionFeedback();

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected();
	std::shared_ptr<GpioDevice> getReferenceDevice();

	bool isActuatorDeviceConnected();
	std::shared_ptr<ActuatorDevice> getActuatorDevice();

	virtual void assignIoData() {
		//inputs
		addIoData(actuatorDeviceLink);
		addIoData(positionFeedbackDeviceLink);
		addIoData(referenceDeviceLink);
		addIoData(positionReferences);
		//outputs
		//addIoData(actuatorCommand);
		//addIoData(resetPositionFeedback);
	}

	virtual void process();

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};