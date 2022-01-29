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

	//============ PINS ==============
private:
	//Inputs
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT, "Actuator");
	std::shared_ptr<NodePin> servoActuatorPin = std::make_shared<NodePin>(NodePin::DataType::SERVO_ACTUATOR, NodePin::Direction::NODE_INPUT, "Servo Actuator");
	std::shared_ptr<NodePin> positionFeedbackPin = std::make_shared<NodePin>(NodePin::DataType::POSITIONFEEDBACK, NodePin::Direction::NODE_INPUT, "Position Feedback");
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Reference Device");
	
	std::shared_ptr<bool> lowLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> highLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> referenceSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> lowLimitSignalPin = std::make_shared<NodePin>(lowLimitSignal, NodePin::Direction::NODE_INPUT, "Low Limit Signal");
	std::shared_ptr<NodePin> highLimitSignalPin = std::make_shared<NodePin>(highLimitSignal, NodePin::Direction::NODE_INPUT, "High Limit Signal");
	std::shared_ptr<NodePin> referenceSignalPin = std::make_shared<NodePin>(referenceSignal, NodePin::Direction::NODE_INPUT, "Reference Signal");
	
	//Outputs
	std::shared_ptr<NodePin> axisPin = std::make_shared<NodePin>(NodePin::DataType::POSITION_CONTROLLED_AXIS, NodePin::Direction::NODE_OUTPUT, "Position Controlled Axis");
	
	std::shared_ptr<double> actualPositionValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualVelocityValue = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualLoadValue = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> positionPin = std::make_shared<NodePin>(actualPositionValue, NodePin::Direction::NODE_OUTPUT, "Position");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(actualVelocityValue, NodePin::Direction::NODE_OUTPUT, "Velocity");
	std::shared_ptr<NodePin> loadPin = std::make_shared<NodePin>(actualLoadValue, NodePin::Direction::NODE_OUTPUT, "Load");
	
	//============= CHECK CONNECTIONS ==============
private:
	
	bool isAxisPinConnected(){ return axisPin->isConnected(); }
	
	bool areAllPinsConnected();
	
	bool needsPositionFeedbackDevice();
	bool isPositionFeedbackDeviceConnected(){ return positionFeedbackPin->isConnected(); }
	std::shared_ptr<PositionFeedbackDevice> getPositionFeedbackDevice(){ return positionFeedbackPin->getConnectedPin()->getSharedPointer<PositionFeedbackDevice>(); }

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected(){ return gpioPin->isConnected(); }
	std::shared_ptr<GpioDevice> getReferenceDevice() { return gpioPin->getConnectedPin()->getSharedPointer<GpioDevice>(); }

	bool needsServoActuatorDevice();
	bool isServoActuatorDeviceConnected(){ return servoActuatorPin->isConnected(); }
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice() { return servoActuatorPin->getConnectedPin()->getSharedPointer<ServoActuatorDevice>(); }

	bool needsActuatorDevice();
	bool isActuatorDeviceConnected() { return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorDevice>(); }

public:
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	//========== PROCESSING =============
public:
	virtual void process();
	
	//==================== SETTINGS ====================

private:
	//units
	PositionUnitType positionUnitType = PositionUnitType::ANGULAR;
	PositionUnit positionUnit = PositionUnit::DEGREE;
	PositionControlType positionControlType = PositionControlType::SERVO;
	
	//Unit Conversions
	double feedbackUnitsPerAxisUnits = 0.0;
	double actuatorUnitsPerAxisUnits = 0.0;
	bool feedbackAndActuatorConversionIdentical = false;

	//Reference Signals and Homing
	PositionReferenceSignal positionReferenceSignal = PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	HomingDirection homingDirection = HomingDirection::NEGATIVE;
	double homingVelocity_axisUnitsPerSecond = 0.0;
	
	//ramp limits
	double velocityLimit = 0.0;
	double accelerationLimit = 0.0;
	double manualAcceleration = 0.0;
	
	//position limits
	bool limitToFeedbackWorkingRange = true;
	double lowPositionLimit = 0.0;
	double lowLimitClearance = 0.0;
	bool b_enableLowLimit = true;
	double highPositionLimit = 0.0;
	double highLimitClearance = 0.0;
	bool b_enableHighLimit = true;
	
	//pid controlled settings
	double proportionalGain = 0.0;
	double integralGain = 0.0;
	double derivativeGain = 0.0;
	double maxPositionError_axisUnits = 0.0;
	
	void sanitizeParameters();
	
	void setPositionUnitType(PositionUnitType t);
	void setPositionUnit(PositionUnit u);
	void setPositionControlType(PositionControlType type);
	void setPositionReferenceSignalType(PositionReferenceSignal type);
	
	double feedbackUnitsToAxisUnits(double feedbackValue) { return feedbackValue / feedbackUnitsPerAxisUnits; }
	double axisUnitsToFeedbackUnits(double axisValue) { return axisValue * feedbackUnitsPerAxisUnits; }
	double actuatorUnitsToAxisUnits(double actuatorValue) { return actuatorValue / actuatorUnitsPerAxisUnits; }
	double axisUnitsToActuatorUnits(double axisValue) { return axisValue * actuatorUnitsPerAxisUnits; }
	
public:
	PositionUnit getPositionUnit(){ return positionUnit; }
	PositionUnitType getPositionUnitType(){ return positionUnitType; }
	double getVelocityLimit_axisUnitsPerSecond() { return velocityLimit; }
	double getAccelerationLimit_axisUnitsPerSecondSquared() { return accelerationLimit; }
	double getLowPositionLimit();
	double getHighPositionLimit();
	
	//========== STATE ===========
public:
	bool isEnabled() { return b_enabled; }
	bool isReady();
	bool isMoving();

	void enable();
	void disable();
	
private:
	bool b_enabled = false;
	void onEnable();
	void onDisable();
	
	//========= MOTION CONTROL =========

private:
	Motion::Profile motionProfile;
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	double positionError = 0.0;
	
	//Manual Controls
	void setVelocityTarget(double velocity_axisUnits);
	void fastStop();
	void moveToPositionWithVelocity(double position_axisUnits, double velocity_axisUnits);
	void moveToPositionInTime(double position_axisUnits, double movementTime_seconds);
	float manualVelocityTarget = 0.0;
	
	//actuator update
	void sendActuatorCommands();

public:
	
	void setMotionCommand(double position, double velocity);
		
	double getProfileVelocity_axisUnitsPerSecond() { return motionProfile.getVelocity(); }
	double getProfilePosition_axisUnits() { return motionProfile.getPosition(); }
	
	double getActualVelocity_axisUnitsPerSecond() { return *actualVelocityValue; }
	double getActualPosition_axisUnits() { return *actualPositionValue; }

	float getActualVelocityNormalized() { return *actualVelocityValue / velocityLimit; }
	float getActualPosition_normalized() {
		double low = getLowPositionLimit();
		double high = getHighPositionLimit();
		return (*actualPositionValue - low) / (high - low);
	}
	
	//============== AXIS SETUP =============
private:
	void setCurrentPosition(double distanceFromAxisOrigin);
	void setCurrentPositionAsNegativeLimit();
	void setCurrentPositionAsPositiveLimit();
	void scaleFeedbackToMatchPosition(double position_axisUnits);
	
public:
	bool isHomeable();
	void startHoming();
	void cancelHoming();
	bool isHoming();
	bool didHomingSucceed();
	bool didHomingFail();
	HomingStep homingStep = HomingStep::NOT_STARTED;
	HomingError homingError = HomingError::NONE;
	
private:
	bool b_isHoming = false;
	void homingControl();
	void onHomingSuccess();
	void onHomingError();

	//============ POSITION REFERENCES AND LIMITS ==============
private:
	//limit and reference signals
	void updateReferenceSignals();
	bool previousLowLimitSignal = false;
	bool previousHighLimitSignal = false;
	bool previousReferenceSignal = false;

	//feedback position limits
	double getLowPositionLimitWithoutClearance();
	double getHighPositionLimitWithoutClearance();
	double getLowFeedbackPositionLimit();
	double getHighFeedbackPositionLimit();
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
	
	//============ LOADING & SAVING ============
public:
	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool save(tinyxml2::XMLElement* xml);

	//============== GUI ================

	virtual void nodeSpecificGui();
	virtual void controlsGui();
	virtual void settingsGui();
	virtual void devicesGui();
	virtual void metricsGui();
	float manualVelocityTargetDisplay = 0.0;
	double interpolationPositionTarget = 0.0;
	double interpolationVelocityTarget = 0.0;
	double interpolationTimeTarget = 0.0;
	double axisScalingPosition = 0.0;

	
	
};
