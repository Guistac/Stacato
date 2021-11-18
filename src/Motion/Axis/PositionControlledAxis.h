#pragma once

#include "Axis.h"

#include "VelocityControlledAxis.h"


class PositionControlledAxis_temp : public VelocityControlledAxis {
public:

	DEFINE_AXIS_NODE(PositionControlledAxis_temp, "Position Controlled Axis", "PositionControlledAxis")
	virtual MotionCommand::Type getMotionCommandType() { return MotionCommand::Type::POSITION_COMMAND; }

	//Device Links
	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodeData::SERVO_ACTUATOR_DEVICE_LINK, DataDirection::NODE_INPUT, "Servo Actuator");
	std::shared_ptr<NodePin> positionFeedbackDeviceLink = std::make_shared<NodePin>(NodeData::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> referenceDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Reference Device");

	//Reference Signals
	std::shared_ptr<NodePin> lowLimitSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Low Limit Signal");
	std::shared_ptr<NodePin> highLimitSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "High Limit Signal");
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Reference Signal");

	//Outputs
	std::shared_ptr<NodePin> position = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position");

	//==================== AXIS DATA ====================

	//Control Type
	PositionControl::Type positionControl = PositionControl::Type::CLOSED_LOOP;
	void setPositionControlType(PositionControl::Type type);

	//Unit Conversions
	double feedbackUnitsPerAxisUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal::Type positionReferenceSignal = PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	void setPositionReferenceSignalType(PositionReferenceSignal::Type type);
	HomingDirection::Type homingDirection = HomingDirection::Type::NEGATIVE;
	double homingVelocity_axisUnitsPerSecond = 0.0;

	//Position Limits
	double maxPositiveDeviation_axisUnits = 0.0;
	double maxNegativeDeviation_axisUnits = 0.0;
	double limitClearance_axisUnits = 0.0;
	bool limitToFeedbackWorkingRange = true;
	bool enableNegativeLimit = true;
	bool enablePositiveLimit = true;

	//Default Manual Movement
	double defaultManualVelocity_axisUnitsPerSecond = 10.0;

	//============== CONTROL VARIABLES ===================

	//controller variables
	double positionError_axisUnits = 0.0;
	double proportionalGain = 0.0;
	double integralGain = 0.0;
	double derivativeGain = 0.0;
	double maxPositionError_axisUnits = 0.0;

	//limit and reference signals
	void updateReferenceSignals();
	bool lowLimitSignal = false;
	bool previousLowLimitSignal = false;
	bool highLimitSignal = false;
	bool previousHighLimitSignal = false;
	bool referenceSignal = false;
	bool previousReferenceSignal = false;

	//position limits
	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();

	//limits origin and scale setting
	void setCurrentPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);

	bool b_isHoming = false;

	ControlMode::Mode controlMode = ControlMode::Mode::MANUAL_VELOCITY_TARGET;

	//Fast Stop Control
	void fastStopControl();

	//Manual Velocity Control
	void setVelocity(double velocity_axisUnits);
	void velocityTargetControl();
	float manualVelocityTarget_axisUnitsPerSecond = 0.0;

	//Manual Target Control
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits);
	void positionTargetControl();
	std::shared_ptr<Motion::Interpolation> targetInterpolation = std::make_shared<Motion::Interpolation>();
	double targetPosition_axisUnits = 0.0;
	double targetVelocity_axisUnitsPerSecond = 0.0;
	double targetTime_seconds = 0.0;

	//Homing Control
	void homingControl();
	void onHomingSuccess();
	void onHomingError();
	Homing::Step homingStep = Homing::Step::NOT_STARTED;
	Homing::Error homingError = Homing::Error::NONE;

	//display data to set the machine coupling
	double machineScalingPosition_axisUnits = 0.0;

	//============= METRICS ============

	void updateMetrics();
	const size_t historyLength = 2048;
	CircularBuffer positionHistory = CircularBuffer(historyLength);
	CircularBuffer actualPositionHistory = CircularBuffer(historyLength);
	CircularBuffer positionErrorHistory = CircularBuffer(historyLength);
	CircularBuffer velocityHistory = CircularBuffer(historyLength);
	CircularBuffer accelerationHistory = CircularBuffer(historyLength);
	CircularBuffer loadHistory = CircularBuffer(historyLength);

	//=========== MACHINE STATE ==============

	void onEnable();
	bool b_enabled = false;

	//========= SUBDEVICES ===========

	bool needsPositionFeedbackDevice();
	bool isPositionFeedbackDeviceConnected();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected();
	std::shared_ptr<GpioDevice> getReferenceDevice();

	bool needsActuatorDevice();
	bool isActuatorDeviceConnected();
	std::shared_ptr<ActuatorDevice> getActuatorDevice();

	bool needsServoActuatorDevice();
	bool isServoActuatorDeviceConnected();
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice();

	//============ NODE DATA ==============

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);



};
















class PositionControlledAxis : public Axis {
public:

	DEFINE_AXIS_NODE(PositionControlledAxis, "Position Controlled Axis", "PositionControllerAxis", MotionCommand::Type::POSITION_COMMAND);
	virtual MotionCommand::Type getCommandType() { return MotionCommand::Type::POSITION_COMMAND; }
	virtual PositionUnit::Type getMotionType() { return axisPositionUnitType; }

	//Device Links
	std::shared_ptr<NodePin> actuatorDeviceLink = std::make_shared<NodePin>(NodeData::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuator");
	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodeData::SERVO_ACTUATOR_DEVICE_LINK, DataDirection::NODE_INPUT, "Servo Actuator");
	std::shared_ptr<NodePin> positionFeedbackDeviceLink = std::make_shared<NodePin>(NodeData::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> referenceDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Reference Device");

	//Reference Signals
	std::shared_ptr<NodePin> lowLimitSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Low Limit Signal");
	std::shared_ptr<NodePin> highLimitSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "High Limit Signal");
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Reference Signal");

	//Outputs
	std::shared_ptr<NodePin> axisLink = std::make_shared<NodePin>(NodeData::AXIS_LINK, DataDirection::NODE_OUTPUT, "Axis");
	std::shared_ptr<NodePin> position = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocity = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity");

	//==================== AXIS DATA ====================

	//Control Type
	PositionControl::Type positionControl = PositionControl::Type::CLOSED_LOOP;
	void setPositionControlType(PositionControl::Type type);

	//Unit Conversions
	double feedbackUnitsPerAxisUnits = 0.0;
	double actuatorUnitsPerAxisUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal::Type positionReferenceSignal = PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	void setPositionReferenceSignalType(PositionReferenceSignal::Type type);
	HomingDirection::Type homingDirection = HomingDirection::Type::NEGATIVE;
	double homingVelocity_axisUnitsPerSecond = 0.0;

	//Position Limits
	double maxPositiveDeviation_axisUnits = 0.0;
	double maxNegativeDeviation_axisUnits = 0.0;
	double limitClearance_axisUnits = 0.0;
	bool limitToFeedbackWorkingRange = true;
	bool enableNegativeLimit = true;
	bool enablePositiveLimit = true;

	//Default Manual Movement
	double defaultManualVelocity_axisUnitsPerSecond = 10.0;
	double defaultManualAcceleration_axisUnitsPerSecondSquared = 5.0;

	//============== CONTROL VARIABLES ===================

	//controller variables
	double positionError_axisUnits = 0.0;
	double proportionalGain = 0.0;
	double integralGain = 0.0;
	double derivativeGain = 0.0;
	double maxPositionError_axisUnits = 0.0;

	//limit and reference signals
	void updateReferenceSignals();
	bool lowLimitSignal = false;
	bool previousLowLimitSignal = false;
	bool highLimitSignal = false;
	bool previousHighLimitSignal = false;
	bool referenceSignal = false;
	bool previousReferenceSignal = false;

	//position limits
	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();

	//limits origin and scale setting
	void setCurrentPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);

	bool b_isHoming = false;

	ControlMode::Mode controlMode = ControlMode::Mode::MANUAL_VELOCITY_TARGET;

	double manualControlAcceleration_axisUnitsPerSecond = 0.0;

	//Fast Stop Control
	void fastStopControl();

	//Manual Velocity Control
	void setVelocity(double velocity_axisUnits);
	void velocityTargetControl();
	float manualVelocityTarget_axisUnitsPerSecond = 0.0;

	//Manual Target Control
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits);
	void positionTargetControl();
	std::shared_ptr<Motion::Interpolation> targetInterpolation = std::make_shared<Motion::Interpolation>();
	double targetPosition_axisUnits = 0.0;
	double targetVelocity_axisUnitsPerSecond = 0.0;
	double targetTime_seconds = 0.0;

	//Homing Control
	void homingControl();
	void onHomingSuccess();
	void onHomingError();
	Homing::Step homingStep = Homing::Step::NOT_STARTED;
	Homing::Error homingError = Homing::Error::NONE;

	//display data to set the machine coupling
	double machineScalingPosition_axisUnits = 0.0;

	//============= METRICS ============

	void updateMetrics();
	const size_t historyLength = 2048;
	CircularBuffer positionHistory = CircularBuffer(historyLength);
	CircularBuffer actualPositionHistory = CircularBuffer(historyLength);
	CircularBuffer positionErrorHistory = CircularBuffer(historyLength);
	CircularBuffer velocityHistory = CircularBuffer(historyLength);
	CircularBuffer accelerationHistory = CircularBuffer(historyLength);
	CircularBuffer loadHistory = CircularBuffer(historyLength);

	//=========== MACHINE STATE ==============

	void onEnable();
	bool b_enabled = false;

	//========= SUBDEVICES ===========

	bool needsPositionFeedbackDevice();
	bool isPositionFeedbackDeviceConnected();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected();
	std::shared_ptr<GpioDevice> getReferenceDevice();

	bool needsActuatorDevice();
	bool isActuatorDeviceConnected();
	std::shared_ptr<ActuatorDevice> getActuatorDevice();

	bool needsServoActuatorDevice();
	bool isServoActuatorDeviceConnected();
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice();

	//============ NODE DATA ==============

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};