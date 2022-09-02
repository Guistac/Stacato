#pragma once

#include "Environnement/NodeGraph/Node.h"

#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"
#include "Motion/Curve/Profile.h"

#include "Project/Editor/Parameter.h"

class Device;

class VelocityControlledAxis : public Node {
public:

	DEFINE_NODE(VelocityControlledAxis, "Velocity Controlled Axis", "VelocityControlledAxis", Node::Type::AXIS, "")

	//======= NODE ========

	//inputs
	
	std::shared_ptr<bool> lowLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> lowSlowdownSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> highLimitSignal = std::make_shared<bool>(false);
	std::shared_ptr<bool> highSlowdownSignal = std::make_shared<bool>(false);
	
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT_BIDIRECTIONAL, "Actuator");
	std::shared_ptr<NodePin> gpioPin = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_INPUT, "Gpio");
	std::shared_ptr<NodePin> lowLimitPin = std::make_shared<NodePin>(lowLimitSignal, NodePin::Direction::NODE_INPUT, "Low Limit");
	std::shared_ptr<NodePin> lowSlowdownPin = std::make_shared<NodePin>(lowSlowdownSignal, NodePin::Direction::NODE_INPUT, "Low Slowdown");
	std::shared_ptr<NodePin> highLimitPin = std::make_shared<NodePin>(highLimitSignal, NodePin::Direction::NODE_INPUT, "High Limit");
	std::shared_ptr<NodePin> highSlowdownPin = std::make_shared<NodePin>(highSlowdownSignal, NodePin::Direction::NODE_INPUT, "High Slowdown");
	
	//outputs
	std::shared_ptr<double> actualVelocity = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualLoad = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> velocityControlledAxisPin = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS, NodePin::Direction::NODE_OUTPUT_BIDIRECTIONAL, "Velocity Controlled Axis");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(actualVelocity, NodePin::Direction::NODE_OUTPUT, "Velocity");
	std::shared_ptr<NodePin> loadPin = std::make_shared<NodePin>(actualLoad, NodePin::Direction::NODE_OUTPUT, "Load");
	
	
	MotionState getState(){ return state; }
	std::string getStatusString();
	bool isEmergencyStopActive(){ return b_emergencyStopActive; }
	
	MotionState state = MotionState::OFFLINE;
	bool b_emergencyStopActive = false;
	
	//========= DEVICES ========
	
public:
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
private:
	bool isActuatorDeviceConnected() { return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorDevice>(); }
	
	bool isAxisPinConnected(){ return velocityControlledAxisPin->isConnected(); }
	
	
	//======== SETTINGS ========

private:
	MovementType movementType = MovementType::ROTARY;
	Unit positionUnit = Units::AngularDistance::Degree;
	
	std::shared_ptr<NumberParameter<double>> actuatorUnitsPerAxisUnits = NumberParameter<double>::make(0.0, "Actuator Units Per Axis Units", "ActuatorUnitsPerAxisUnits",
																									   "%.6f", Units::None::None, false);
	std::shared_ptr<NumberParameter<double>> velocityLimit = NumberParameter<double>::make(0.0, "Velocity Limit", "VelocityLimit",
																						   "%.3f", Units::None::None, false, 0, 0, "", "/s");
	std::shared_ptr<NumberParameter<double>> accelerationLimit = NumberParameter<double>::make(0.0, "Acceleration Limit", "AccelerationLimit",
																							   "%.3f", Units::None::None, false, 0, 0, "", "/s\xc2\xb2");
	std::shared_ptr<NumberParameter<double>> decelerationLimit = NumberParameter<double>::make(0.0, "Deceleration Limit", "DecelerationLimit",
																							   "%.3f", Units::None::None, false, 0, 0, "", "/s\xc2\xb2");
	std::shared_ptr<NumberParameter<double>> manualAcceleration = NumberParameter<double>::make(0.0, "Manual Acceleration", "ManualAcceleration",
																								"%.3f", Units::None::None, false, 0, 0, "", "/s\xc2\xb2");
	std::shared_ptr<NumberParameter<double>> manualDeceleration = NumberParameter<double>::make(0.0, "Manual Deceleration", "ManualDeceleration",
																								"%.3f", Units::None::None, false, 0, 0, "", "/s\xc2\xb2");
	std::shared_ptr<NumberParameter<double>> slowdownVelocity = NumberParameter<double>::make(0.0, "Slowdown Velocity", "SlowdownVelocity",
																							  "%.3f", Units::None::None, false, 0, 0, "", "/s");
	

	void setMovementType(MovementType type);
	void setPositionUnit(Unit unit);
	double actuatorUnitsToAxisUnits(double actuatorValue){ return actuatorValue / actuatorUnitsPerAxisUnits->value; }
	double axisUnitsToActuatorUnits(double axisUnits){ return axisUnits * actuatorUnitsPerAxisUnits->value; }
	
public:
	void sanitizeParameters();
	
	Unit getPositionUnit(){ return positionUnit; }
	MovementType getMovementType() { return movementType; }
	
	double getVelocityLimit() { return velocityLimit->value; }
	bool decelerationLimitisAccelerationLimit(){ return accelerationLimit->value == decelerationLimit->value; }
	double getAccelerationLimit(){ return accelerationLimit->value; }
	double getDecelerationLimit(){ return decelerationLimit->value; }

	//========= STATE ==========

	bool isEnabled() { return state == MotionState::ENABLED; }
	void enable();
	void disable();
	bool isMoving();

private:
	void onEnable();
	void onDisable();

	//========== CONTROL =========

private:
	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	Motion::Profile motionProfile;
	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	
	void setVelocity(double velocity_positionUnitsPerSecondSquare);
	void fastStop();
	float manualVelocityTarget = 0.0;

	void sendActuatorCommands();
	
public:
	bool areAllPinsConnected();
	bool areAllDeviceReady();
	void updateAxisState();
	void updateReferenceSignals();
	
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	virtual bool needsOutputProcess() override { return !isAxisPinConnected(); }
	
	void setVelocityCommand(double velocity, double acceleration);
	double externalVelocityCommand = 0.0;
	double externalAccelerationCommand = 0.0;
	
	bool isAtUpperLimit();
	bool isAtLowerLimit();
	
	double getProfileVelocity_axisUnitsPerSecond() { return motionProfile.getVelocity(); }
	double getProfileAcceleration_axisUnitsPerSecondSquared() { return motionProfile.getAcceleration(); }

	//======= GUI ========

	virtual void nodeSpecificGui() override;
	void controlsGui();
	void settingsGui();
	void devicesGui();
	void metricsGui();
	float manualVelocityDisplay = 0.0;
	
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;

};
