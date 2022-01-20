#pragma once

#include "NodeGraph/Node.h"

#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"

class Device;

class VelocityControlledAxis : public Node {
public:

	DEFINE_NODE(VelocityControlledAxis, "Velocity Controlled Axis", "VelocityControlledAxis", Node::Type::AXIS, "")

	virtual MotionCommand getMotionCommandType() { return MotionCommand::VELOCITY; }

	//======== SETTINGS ========

	PositionUnit positionUnit = PositionUnit::DEGREE;
	PositionUnitType positionUnitType = PositionUnitType::ANGULAR;

	double getVelocityLimit_axisUnitsPerSecond() { return velocityLimit_axisUnitsPerSecond; }
	double getAccelerationLimit_axisUnitsPerSecondSquared() { return accelerationLimit_axisUnitsPerSecondSquared; }
	double velocityLimit_axisUnitsPerSecond = 0.0;
	double accelerationLimit_axisUnitsPerSecondSquared = 0.0;

	double actuatorUnitsPerAxisUnits = 0.0;

	double defaultManualAcceleration_axisUnitsPerSecondSquared = 5.0;

	//========= DEVICES ========

	void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	bool isActuatorDeviceConnected() { return actuatorDeviceLink->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorDeviceLink->getConnectedPins().front()->getSharedPointer<ActuatorDevice>(); }

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
	double profileVelocity_axisUnitsPerSecond = 0.0;
	double profileAcceleration_axisUnitsPerSecondSquared = 0.0;

	double getProfileVelocity_axisUnitsPerSecond() { return profileVelocity_axisUnitsPerSecond; }
	double getProfileAcceleration_axisUnitsPerSecondSquared() { return profileAcceleration_axisUnitsPerSecondSquared; }

	double actualVelocity_axisUnitsPerSecond = 0.0;
	double actualAcceleration_axisUnitsPerSecond = 0.0;
	double actualLoad_normalized = 0.0;

	double getActualVelocity_axisUnitsPerSecond() { return actualVelocity_axisUnitsPerSecond; }
	double getActualAcceleration_axisUnitsPerSecondSquared() { return actualAcceleration_axisUnitsPerSecond; }

	float getActualVelocityNormalized() { return actualVelocity_axisUnitsPerSecond / velocityLimit_axisUnitsPerSecond; }
	float getActualAccelerationNormalized() { return actualAcceleration_axisUnitsPerSecond / accelerationLimit_axisUnitsPerSecondSquared; }

	//========== CONTROL =========

	double manualControlAcceleration_axisUnitsPerSecond = 0.0;

	virtual void setVelocityTarget(double velocity_positionUnitsPerSecondSquare);
	float manualVelocityTarget_axisUnitsPerSecond = 0.0;

	virtual void fastStop();
	virtual void fastStopControl();

	virtual bool isAxisPinConnected() { return velocityControlledAxisLink->isConnected(); }
	virtual void sendActuatorCommands();

	//======= NODE ========

	std::shared_ptr<NodePin> actuatorDeviceLink = std::make_shared<NodePin>(NodePin::DataType::ACTUATOR, NodePin::Direction::NODE_INPUT, "Actuator");
	
	std::shared_ptr<NodePin> velocityControlledAxisLink = std::make_shared<NodePin>(NodePin::DataType::VELOCITY_CONTROLLED_AXIS, NodePin::Direction::NODE_OUTPUT, "Velocity Controlled Axis");
	std::shared_ptr<NodePin> velocity = std::make_shared<NodePin>(NodePin::DataType::REAL, NodePin::Direction::NODE_OUTPUT, "Velocity");
	
	std::shared_ptr<double> velocityPinValue = std::make_shared<double>(0.0);

	//======= GUI ========

	virtual void nodeSpecificGui();
	virtual void controlsGui();
	virtual void settingsGui();
	virtual void devicesGui();
	virtual void metricsGui();
	
	virtual void process();

};
