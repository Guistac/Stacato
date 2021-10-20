#pragma once

#include "Machine.h"

class SingleAxisMachine : public Machine {
public:

	DEFINE_MACHINE_NODE(SingleAxisMachine, "Single Axis Machine", "SingleAxisMachine", Machine::Type::SINGLE_AXIS_MACHINE);

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
	std::shared_ptr<NodePin> position = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocity = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity");

	virtual void assignIoData() {
		//inputs
		//add default inputs for closed loop control with limit signals
		//these pins get removed if another motion contorl mode or position limit mode gets loaded or selected
		addIoData(actuatorDeviceLink);
		addIoData(servoActuatorDeviceLink);
		addIoData(positionFeedbackDeviceLink);
		addIoData(referenceDeviceLink);
		addIoData(lowLimitSignalPin);
		addIoData(highLimitSignalPin);
		addIoData(referenceSignalPin);
		//outputs
		//these pins are always present
		addIoData(position);
		addIoData(velocity);
		setPositionControlType(positionControl);
		setPositionReferenceSignalType(positionReferenceSignal);
	}

	//==================== AXIS DATA ====================

	//Machine Type
	PositionUnit::Type machinePositionUnitType = PositionUnit::Type::ANGULAR;
	PositionUnit::Unit machinePositionUnit = PositionUnit::Unit::DEGREE;
	PositionControl::Type positionControl = PositionControl::Type::CLOSED_LOOP;
	void setPositionControlType(PositionControl::Type type);

	//Unit Conversions
	double feedbackUnitsPerMachineUnits = 0.0;
	double actuatorUnitsPerMachineUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal::Type positionReferenceSignal = PositionReferenceSignal::Type::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	void setPositionReferenceSignalType(PositionReferenceSignal::Type type);
	HomingDirection::Type homingDirection = HomingDirection::Type::NEGATIVE;
	double homingVelocity_machineUnitsPerSecond = 0.0;

	//Kinematic Limits
	double velocityLimit_machineUnitsPerSecond = 0.0;
	double accelerationLimit_machineUnitsPerSecondSquared = 0.0;

	//Position Limits
	double maxPositiveDeviation_machineUnits = 0.0;
	double maxNegativeDeviation_machineUnits = 0.0;
	bool limitToFeedbackWorkingRange = true;
	bool enableNegativeLimit = true;
	bool enablePositiveLimit = true;

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

	//limit and reference signals
	void updateReferenceSignals();
	bool lowLimitSignal = false;
	bool previousLowLimitSignal = false;
	bool highLimitSignal = false;
	bool previousHighLimitSignal = false;
	bool referenceSignal = false;
	bool previousReferenceSignal = false;

	//actual machine state based on feedback data
	double actualPosition_machineUnits;
	double actualVelocity_machineUnitsPerSecond;

	bool isMoving();
	double getLowAxisPositionLimit();
	double getHighAxisPositionLimit();
	double getAxisPositionProgress();
	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();
	void setCurrentAxisPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);

	enum class ControlMode {
		VELOCITY_TARGET,
		POSITION_TARGET,
		FOLLOW_CURVE
	};

	bool b_isHoming = false;

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
	bool didHomingSucceed();
	bool didHomingFail();
	void homingControl();
	void onHomingSuccess();
	void onHomingError();
	Homing::Step homingStep = Homing::Step::NOT_STARTED;
	Homing::Error homingError = Homing::Error::NONE;

	//display data to set the machine coupling
	double machineScalingPosition_machineUnits = 0.0;

	//============= METRICS ============

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

	virtual void process();

	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);
};