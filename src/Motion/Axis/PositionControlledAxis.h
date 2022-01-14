#pragma once

#include "VelocityControlledAxis.h"

namespace Motion {
	class Interpolation;
}

class PositionControlledAxis : public VelocityControlledAxis {
public:

	DEFINE_AXIS_NODE(PositionControlledAxis, "Position Controlled Axis", "PositionControlledAxis")
	virtual MotionCommand getMotionCommandType() { return MotionCommand::POSITION; }

	//==================== SETTINGS ====================

	//movement type
	void setPositionUnitType(PositionUnitType t);
	
	//position unit
	void setPositionUnit(PositionUnit u);
	
	//Control Type
	PositionControlType positionControlType = PositionControlType::CLOSED_LOOP;
	void setPositionControlType(PositionControlType type);

	//Unit Conversions
	double feedbackUnitsPerAxisUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal positionReferenceSignal = PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	void setPositionReferenceSignalType(PositionReferenceSignal type);
	HomingDirection homingDirection = HomingDirection::NEGATIVE;
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

	//========= DEVICES ===========

	bool needsPositionFeedbackDevice();
	bool isPositionFeedbackDeviceConnected();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected();
	std::shared_ptr<GpioDevice> getReferenceDevice();

	bool needsActuatorDevice();

	bool needsServoActuatorDevice();
	bool isServoActuatorDeviceConnected();
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice();

	virtual void sendActuatorCommands();

	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output);

	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();

	//========== STATE ===========

	virtual bool isReady();
	virtual bool isMoving();

	virtual void enable();
	virtual void disable();

	virtual void onEnable();
	virtual void onDisable();

	//========= MOTION PROFILE =========

	double profilePosition_axisUnits = 0.0;
	double getProfilePosition_axisUnits() { return profilePosition_axisUnits; }

	double actualPosition_axisUnits = 0.0;
	double getActualPosition_axisUnits() { return actualPosition_axisUnits; }

	float getActualPosition_normalized();

	//============== CONTROL ===================

	//pid controller variables
	double positionError_axisUnits = 0.0;
	double proportionalGain = 0.0;
	double integralGain = 0.0;
	double derivativeGain = 0.0;
	double maxPositionError_axisUnits = 0.0;

	//limits origin and scale setting
	void setCurrentPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);

	ControlMode::Mode controlMode = ControlMode::Mode::VELOCITY_TARGET;
	virtual bool isAxisPinConnected() { return positionControlledAxisPin->isConnected(); }

	//Fast Stop Control
	void fastStop();
	void fastStopControl();
	double getFastStopBrakingPosition();

	//Manual Velocity Control
	void setVelocityTarget(double velocity_axisUnits);
	void velocityTargetControl();

	//Manual Target Control
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits);
	void positionTargetControl();
	std::shared_ptr<Motion::Interpolation> targetInterpolation;
	double targetPosition_axisUnits = 0.0;
	double targetVelocity_axisUnitsPerSecond = 0.0;
	double targetTime_seconds = 0.0;

	//Homing Control
	bool b_isHoming = false;
	bool isHomeable();
	void startHoming();
	void cancelHoming();
	bool isHoming();
	bool didHomingSucceed();
	bool didHomingFail();
	void homingControl();
	void onHomingSuccess();
	void onHomingError();
	HomingStep homingStep = HomingStep::NOT_STARTED;
	HomingError homingError = HomingError::NONE;

	//display data to set the machine coupling
	double machineScalingPosition_axisUnits = 0.0;

	//============ POSITION REFERENCES AND LIMITS ==============

	//limit and reference signals
	void updateReferenceSignals();
	bool lowLimitSignal = false;
	bool previousLowLimitSignal = false;
	bool highLimitSignal = false;
	bool previousHighLimitSignal = false;
	bool referenceSignal = false;
	bool previousReferenceSignal = false;

	//feedback position limits
	double getLowPositionLimit();
	double getHighPositionLimit();
	double getLowPositionLimitWithoutClearance();
	double getHighPositionLimitWithoutClearance();
	double getRange_axisUnits();

	//============= METRICS ============

	void updateMetrics();
	const size_t historyLength = 2048;
	CircularBuffer positionHistory = CircularBuffer(historyLength);
	CircularBuffer actualPositionHistory = CircularBuffer(historyLength);
	CircularBuffer positionErrorHistory = CircularBuffer(historyLength);
	CircularBuffer velocityHistory = CircularBuffer(historyLength);
	CircularBuffer accelerationHistory = CircularBuffer(historyLength);
	CircularBuffer loadHistory = CircularBuffer(historyLength);

	//============ NODE ==============

	//Device Links
	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodeData::SERVO_ACTUATOR_DEVICE_LINK, DataDirection::NODE_INPUT, "Servo Actuator");
	std::shared_ptr<NodePin> positionFeedbackDeviceLink = std::make_shared<NodePin>(NodeData::POSITIONFEEDBACK_DEVICELINK, DataDirection::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> referenceDeviceLink = std::make_shared<NodePin>(NodeData::GPIO_DEVICELINK, DataDirection::NODE_INPUT, "Reference Device");

	//Reference Signals
	std::shared_ptr<NodePin> lowLimitSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Low Limit Signal");
	std::shared_ptr<NodePin> highLimitSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "High Limit Signal");
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Reference Signal");

	//Outputs
	std::shared_ptr<NodePin> positionControlledAxisPin = std::make_shared<NodePin>(NodeData::POSITION_CONTROLLED_AXIS_LINK, DataDirection::NODE_OUTPUT, "Position Controlled Axis");
	std::shared_ptr<NodePin> position = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Position");


	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);

	//============== GUI ================

	virtual void nodeSpecificGui();

	virtual void stateControlGui();
	virtual void controlsGui();
	virtual void setupGui();
	virtual void settingsGui();
	virtual void devicesGui();
	virtual void metricsGui();

	float getFeedbackGuiHeight();
	virtual void feedbackGui();

};
