#pragma once

#include "NodeGraph/Node.h"
#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"
#include "Motion/Profile/Profile.h"

class Device;
namespace Motion { class Interpolation; }

class PositionControlledAxis : public Node {
public:

	DEFINE_NODE(PositionControlledAxis, "Position Controlled Axis", "PositionControlledAxis", Node::Type::AXIS, "")

	//==================== SETTINGS ====================

	//movement type
	PositionUnitType positionUnitType = PositionUnitType::ANGULAR;
	void setPositionUnitType(PositionUnitType t);
	
	//position unit
	PositionUnit positionUnit = PositionUnit::DEGREE;
	void setPositionUnit(PositionUnit u);
	
	//Control Type
	PositionControlType positionControlType = PositionControlType::CLOSED_LOOP;
	void setPositionControlType(PositionControlType type);

	//Unit Conversions
	double feedbackUnitsPerAxisUnits = 0.0;
	double actuatorUnitsPerAxisUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal positionReferenceSignal = PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	void setPositionReferenceSignalType(PositionReferenceSignal type);
	HomingDirection homingDirection = HomingDirection::NEGATIVE;
	double homingVelocity_axisUnitsPerSecond = 0.0;
	
	//velocity limit
	double velocityLimit_axisUnitsPerSecond = 0.0;
	double getVelocityLimit_axisUnitsPerSecond() { return velocityLimit_axisUnitsPerSecond; }
	
	//acceleration limit
	double accelerationLimit_axisUnitsPerSecondSquared = 0.0;
	double getAccelerationLimit_axisUnitsPerSecondSquared() { return accelerationLimit_axisUnitsPerSecondSquared; }

	//Position Limits
	double maxPositiveDeviation_axisUnits = 0.0;
	double maxNegativeDeviation_axisUnits = 0.0;
	double limitClearance_axisUnits = 0.0;
	bool limitToFeedbackWorkingRange = true;
	bool enableNegativeLimit = true;
	bool enablePositiveLimit = true;
	
	//pid controlled settings
	double proportionalGain = 0.0;
	double integralGain = 0.0;
	double derivativeGain = 0.0;
	double maxPositionError_axisUnits = 0.0;

	//========= DEVICES ===========

	bool needsPositionFeedbackDevice();
	bool isPositionFeedbackDeviceConnected();
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice();

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected();
	std::shared_ptr<GpioDevice> getReferenceDevice();

	bool needsServoActuatorDevice();
	bool isServoActuatorDeviceConnected();
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice();

	bool needsActuatorDevice();
	bool isActuatorDeviceConnected() { return actuatorDeviceLink->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorDeviceLink->getConnectedPins().front()->getSharedPointer<ActuatorDevice>(); }

	void sendActuatorCommands();
	
	void getDevices(std::vector<std::shared_ptr<Device>>& output);

	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();
	
	//========== STATE ===========

	bool isEnabled() { return b_enabled; }
	bool b_enabled = false;

	bool isReady();
	bool isMoving();

	void enable();
	void disable();

	void onEnable();
	void onDisable();
	
	//========= MOTION PROFILE =========

	Motion::Profile motionProfile;
	
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	
	double getProfilePosition_axisUnits() { return motionProfile.getPosition(); }
	double getProfileVelocity_axisUnitsPerSecond() { return motionProfile.getVelocity(); }
	double getProfileAcceleration_axisUnitsPerSecondSquared() { return motionProfile.getAcceleration(); }
	
	//TODO: Reimplement these
	double getActualPosition_axisUnits() { return 0.0; }
	double getActualVelocity_axisUnitsPerSecond() { return 0.0; }
	double getActualAcceleration_axisUnitsPerSecondSquared() { return 0.0; }

	float getActualPosition_normalized();
	float getActualVelocityNormalized() { return getActualVelocityNormalized() / velocityLimit_axisUnitsPerSecond; }
	float getActualAccelerationNormalized() { return getActualAccelerationNormalized() / accelerationLimit_axisUnitsPerSecondSquared; }

	//============== CONTROL ===================

	//pid controller variables
	double positionError_axisUnits = 0.0;

	//limits origin and scale setting
	void setCurrentPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);

	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	
	virtual void process();
	
	//master acceleration for manual controls
	float manualControlAcceleration_axisUnitsPerSecondSquared = 0.0;
	
	//Manual Velocity Control
	void setVelocityTarget(double velocity_axisUnits);
	float manualVelocityTarget_axisUnitsPerSecond = 0.0;
	
	//fast stop control
	void fastStop();
	
	//Manual Target Control
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits, double acceleration_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds, double acceleration_axisUnits);
	
	std::shared_ptr<Motion::Interpolation> targetInterpolation = std::make_shared<Motion::Interpolation>();
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
	bool previousLowLimitSignal = false;
	bool previousHighLimitSignal = false;
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

	//Inputs
	std::shared_ptr<NodePin> actuatorDeviceLink = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT, "Actuator");
	std::shared_ptr<NodePin> servoActuatorDeviceLink = std::make_shared<NodePin>(NodePin::DataType::SERVO_ACTUATOR, NodePin::Direction::NODE_INPUT, "Servo Actuator");
	std::shared_ptr<NodePin> positionFeedbackDeviceLink = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> referenceDeviceLink = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Reference Device");
	
	std::shared_ptr<bool> lowLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> lowLimitSignalPin = std::make_shared<NodePin>(lowLimitSignal, NodePin::Direction::NODE_INPUT, "Low Limit Signal");
	
	std::shared_ptr<bool> highLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> highLimitSignalPin = std::make_shared<NodePin>(highLimitSignal, NodePin::Direction::NODE_INPUT, "High Limit Signal");
	
	std::shared_ptr<bool> referenceSignal = std::make_shared<bool>(false);
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(referenceSignal, NodePin::Direction::NODE_INPUT, "Reference Signal");
	
	
	//Outputs
	std::shared_ptr<NodePin> positionControlledAxisPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_OUTPUT, "Position Controlled Axis");
	
	std::shared_ptr<double> actualPositionValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(actualPositionValue, NodePin::Direction::NODE_OUTPUT, "Position");
	
	std::shared_ptr<double> actualVelocityValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(actualVelocityValue, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	std::shared_ptr<double> actualLoadValue = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> loadPin = std::make_shared<NodePin>(actualLoadValue, NodePin::Direction::NODE_OUTPUT, "Load");
	
	bool isAxisPinConnected(){ return positionControlledAxisPin->isConnected(); }
	
	//============ LOADING & SAVING ============

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
	
	float manualVelocityTarget_displayValue = 0.0;

};
