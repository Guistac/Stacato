#pragma once

#include "Environnement/NodeGraph/Node.h"

#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"
#include "Motion/Curve/Profile.h"

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
	std::shared_ptr<NodePin> lowSlowdownPin = std::make_shared<NodePin>(lowLimitSignal, NodePin::Direction::NODE_INPUT, "Low Slowdown");
	std::shared_ptr<NodePin> highLimitPin = std::make_shared<NodePin>(lowLimitSignal, NodePin::Direction::NODE_INPUT, "High Limit");
	std::shared_ptr<NodePin> highSlowdownPin = std::make_shared<NodePin>(lowLimitSignal, NodePin::Direction::NODE_INPUT, "High Slowdown");
	
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
	
private:
	bool isActuatorDeviceConnected() { return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorDevice>(); }
	
	bool isAxisPinConnected(){ return velocityControlledAxisPin->isConnected(); }
	
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	//======== SETTINGS ========

private:
	MovementType movementType = MovementType::ROTARY;
	Unit positionUnit = Units::AngularDistance::Degree;
	double actuatorUnitsPerAxisUnits = 0.0;
	double velocityLimit = 0.0;
	double accelerationLimit = 0.0;
	double manualAcceleration = 0.0;

	void setMovementType(MovementType type);
	void setPositionUnit(Unit unit);
	double actuatorUnitsToAxisUnits(double actuatorValue){ return actuatorValue / actuatorUnitsPerAxisUnits; }
	double axisUnitsToActuatorUnits(double axisUnits){ return axisUnits * actuatorUnitsPerAxisUnits; }
	
public:
	void sanitizeParameters();
	
	Unit getPositionUnit(){ return positionUnit; }
	MovementType getMovementType() { return movementType; }
	
	double getVelocityLimit() { return velocityLimit; }
	double getAccelerationLimit() { return accelerationLimit; }

	//========= STATE ==========

	bool isEnabled() { return b_enabled; }
	void enable();
	void disable();
	bool isReady();
	bool isMoving();

private:
	bool b_enabled = false;
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
	virtual void inputProcess() override;
	virtual void outputProcess() override;
	virtual bool needsOutputProcess() override { return !isAxisPinConnected(); }
	
	void setVelocityCommand(double velocity);
	
	double getProfileVelocity_axisUnitsPerSecond() { return motionProfile.getVelocity(); }
	double getProfileAcceleration_axisUnitsPerSecondSquared() { return motionProfile.getAcceleration(); }

	//======= GUI ========

	virtual void nodeSpecificGui();
	void controlsGui();
	void settingsGui();
	void devicesGui();
	void metricsGui();
	float manualVelocityDisplay = 0.0;
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);

};
