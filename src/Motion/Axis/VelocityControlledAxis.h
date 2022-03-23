#pragma once

#include "Environnement/Node.h"

#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"

#include "Motion/Profile.h"

class Device;

class VelocityControlledAxis : public Node {
public:

	DEFINE_NODE(VelocityControlledAxis, "Velocity Controlled Axis", "VelocityControlledAxis", Node::Type::AXIS, "")

	//======= NODE ========

	//inputs
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT, "Actuator");
	
	//outputs
	std::shared_ptr<double> actualVelocity = std::make_shared<double>(0.0);
	std::shared_ptr<double> actualLoad = std::make_shared<double>(0.0);
	
	std::shared_ptr<NodePin> velocityControlledAxisPin = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS, NodePin::Direction::NODE_OUTPUT, "Velocity Controlled Axis");
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(actualVelocity, NodePin::Direction::NODE_OUTPUT, "Velocity");
	std::shared_ptr<NodePin> loadPin = std::make_shared<NodePin>(actualLoad, NodePin::Direction::NODE_OUTPUT, "Load");
	
	//========= DEVICES ========
	
private:
	bool isActuatorDeviceConnected() { return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorDevice>(); }
	
	bool isAxisPinConnected(){ return velocityControlledAxisPin->isConnected(); }
	
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	//======== SETTINGS ========

private:
	PositionUnitType positionUnitType = PositionUnitType::ANGULAR;
	PositionUnit positionUnit = PositionUnit::DEGREE;
	double actuatorUnitsPerAxisUnits = 0.0;
	double velocityLimit = 0.0;
	double accelerationLimit = 0.0;
	double manualAcceleration = 0.0;

	void setPositionUnitType(PositionUnitType type);
	void setPositionUnit(PositionUnit unit);
	double actuatorUnitsToAxisUnits(double actuatorValue){ return actuatorValue / actuatorUnitsPerAxisUnits; }
	double axisUnitsToActuatorUnits(double axisUnits){ return axisUnits * actuatorUnitsPerAxisUnits; }
	
public:
	void sanitizeParameters();
	
	PositionUnit getPositionUnit(){ return positionUnit; }
	PositionUnitType getPositionUnitType() { return positionUnitType; }
	
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
	virtual void process();
	
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
