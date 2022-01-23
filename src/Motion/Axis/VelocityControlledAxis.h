#pragma once

#include "NodeGraph/Node.h"

#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"

#include "Motion/Profile/Profile.h"

class Device;

class VelocityControlledAxis : public Node {
public:

	DEFINE_NODE(VelocityControlledAxis, "Velocity Controlled Axis", "VelocityControlledAxis", Node::Type::AXIS, "")

	//======= NODE ========

	//inputs
	std::shared_ptr<NodePin> actuatorPin = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT, "Actuator");
	
	//outputs
	std::shared_ptr<double> actualVelocity = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> velocityPin = std::make_shared<NodePin>(actualVelocity, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	std::shared_ptr<double> actualLoad = std::make_shared<double>(0.0);
	std::shared_ptr<NodePin> loadPin = std::make_shared<NodePin>(actualLoad, NodePin::Direction::NODE_OUTPUT, "Load");
	
	std::shared_ptr<NodePin> velocityControlledAxisPin = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS, NodePin::Direction::NODE_OUTPUT, "Velocity Controlled Axis");
	
	//========= DEVICES ========
	
	bool isActuatorDeviceConnected() { return actuatorPin->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorPin->getConnectedPin()->getSharedPointer<ActuatorDevice>(); }
	
	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	//======== SETTINGS ========

	PositionUnitType positionUnitType = PositionUnitType::ANGULAR;
	PositionUnit positionUnit = PositionUnit::DEGREE;
	
	double velocityLimit_axisUnitsPerSecond = 0.0;
	double accelerationLimit_axisUnitsPerSecondSquared = 0.0;
	double actuatorUnitsPerAxisUnits = 0.0;
	
	double manualControlAcceleration_axisUnitsPerSecond = 0.0;

	void setPositionUnitType(PositionUnitType type);
	void setPositionUnit(PositionUnit unit);
	
	double getVelocityLimit_axisUnitsPerSecond() { return velocityLimit_axisUnitsPerSecond; }
	double getAccelerationLimit_axisUnitsPerSecondSquared() { return accelerationLimit_axisUnitsPerSecondSquared; }
	
	double actuatorUnitsToAxisUnits(double actuatorValue){ return actuatorValue / actuatorUnitsPerAxisUnits; }
	double axisUnitsToActuatorUnits(double axisUnits){ return axisUnits * actuatorUnitsPerAxisUnits; }

	//========= STATE ==========

	bool isEnabled() { return b_enabled; }
	bool b_enabled = false;

	bool isReady();
	bool isMoving();

	void enable();
	void disable();

	void onEnable();
	void onDisable();

	//========= MOTION PROFILE =========

	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	
	Motion::Profile motionProfile;
	
	double getProfileVelocity_axisUnitsPerSecond() { return motionProfile.getVelocity(); }
	double getProfileAcceleration_axisUnitsPerSecondSquared() { return motionProfile.getAcceleration(); }

	double getActualVelocity_axisUnitsPerSecond() { return *actualVelocity; }
	float getActualVelocityNormalized() { return *actualVelocity / velocityLimit_axisUnitsPerSecond; }

	//========== CONTROL =========

	ControlMode controlMode = ControlMode::VELOCITY_TARGET;
	
	void setVelocity(double velocity_positionUnitsPerSecondSquare);
	float manualVelocityTarget_axisUnitsPerSecond = 0.0;

	bool isAxisPinConnected() { return velocityControlledAxisPin->isConnected(); }
	void sendActuatorCommands();

	//======= GUI ========

	virtual void nodeSpecificGui();
	void controlsGui();
	void settingsGui();
	void devicesGui();
	void metricsGui();
	
	virtual void process();
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);

};
