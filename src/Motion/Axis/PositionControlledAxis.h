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
	std::shared_ptr<NodePin> servoActuatorPin = std::make_shared<NodePin>(NodePin::DataType::SERVO_ACTUATOR, NodePin::Direction::NODE_INPUT, "Servo Actuator");
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

	bool needsReferenceDevice();
	bool isReferenceDeviceConnected(){ return gpioPin->isConnected(); }
	std::shared_ptr<GpioDevice> getReferenceDevice() { return gpioPin->getConnectedPin()->getSharedPointer<GpioDevice>(); }

	bool isServoActuatorDeviceConnected(){ return servoActuatorPin->isConnected(); }
	std::shared_ptr<ServoActuatorDevice> getServoActuatorDevice() { return servoActuatorPin->getConnectedPin()->getSharedPointer<ServoActuatorDevice>(); }

public:
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	//========== PROCESSING =============
public:
	virtual void process();
	
	//==================== PARAMETERS ====================
private:
	//units
	PositionUnitType positionUnitType = PositionUnitType::ANGULAR;
	PositionUnit positionUnit = PositionUnit::DEGREE;
	double servoActuatorUnitsPerAxisUnits = 0.0;

	//Reference Signals and Homing
	PositionReferenceSignal positionReferenceSignal = PositionReferenceSignal::SIGNAL_AT_LOWER_AND_UPPER_LIMIT;
	HomingDirection homingDirection = HomingDirection::NEGATIVE;
	double homingVelocityCoarse = 0.0;
	double homingVelocityFine = 0.0;
	
	//kinematic limits
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
	
	void sanitizeParameters();
	
	void setPositionUnitType(PositionUnitType t);
	void setPositionUnit(PositionUnit u);
	void setPositionReferenceSignalType(PositionReferenceSignal type);
	
	double servoActuatorUnitsToAxisUnits(double actuatorValue) { return actuatorValue / servoActuatorUnitsPerAxisUnits; }
	double axisUnitsToServoActuatorUnits(double axisValue) { return axisValue * servoActuatorUnitsPerAxisUnits; }
	
public:
	PositionUnit getPositionUnit(){ return positionUnit; }
	PositionUnitType getPositionUnitType(){ return positionUnitType; }
	double getVelocityLimit() { return velocityLimit; }
	double getAccelerationLimit() { return accelerationLimit; }
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
	
	//========= MOTION
private:
	Motion::Profile motionProfile;
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	
	//Manual Controls
	void setVelocityTarget(double velocity);
	void fastStop();
	void moveToPositionWithVelocity(double position, double velocity);
	void moveToPositionInTime(double position, double movementTime);
	float manualVelocityTarget = 0.0;
	
	//actuator update
	void sendActuatorCommands();

	//======= MOTION INTERFACE
public:
	void setMotionCommand(double position, double velocity);
	double getProfileVelocity() { return motionProfile.getVelocity(); }
	double getProfilePosition() { return motionProfile.getPosition(); }
	double getActualVelocity() { return *actualVelocityValue; }
	double getActualPosition() { return *actualPositionValue; }
	double getActualFollowingError();
	float getActualFollowingErrorNormalized();
	float getActualVelocityNormalized() { return *actualVelocityValue / velocityLimit; }
	float getActualPositionNormalized() {
		double low = getLowPositionLimit();
		double high = getHighPositionLimit();
		return (*actualPositionValue - low) / (high - low);
	}
	
	//============== AXIS SETUP
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
	double getRange();

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
