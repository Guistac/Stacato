#pragma once

#include "Axis.h"

#include "Motion/MotionTypes.h"
#include "Utilities/CircularBuffer.h"

class Device;

class VelocityControlledAxis : public Axis {
public:

	DEFINE_AXIS_NODE(VelocityControlledAxis, "Velocity Controlled Axis", "VelocityControlledAxis")

	virtual MotionCommand::Type getMotionCommandType() { return MotionCommand::Type::VELOCITY_COMMAND; }

	//======== SETTINGS ========

	PositionUnit::Unit positionUnit = PositionUnit::Unit::DEGREE;
	PositionUnit::Type positionUnitType = PositionUnit::Type::ANGULAR;

	double getVelocityLimit_axisUnitsPerSecond() { return velocityLimit_axisUnitsPerSecond; }
	double getAccelerationLimit_axisUnitsPerSecondSquared() { return accelerationLimit_axisUnitsPerSecondSquared; }
	double velocityLimit_axisUnitsPerSecond = 0.0;
	double accelerationLimit_axisUnitsPerSecondSquared = 0.0;

	double actuatorUnitsPerAxisUnits = 0.0;

	double defaultManualAcceleration_axisUnitsPerSecondSquared = 5.0;

	//========= DEVICES ========

	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output);
	
	bool isActuatorDeviceConnected() { return actuatorDeviceLink->isConnected(); }
	std::shared_ptr<ActuatorDevice> getActuatorDevice() { return actuatorDeviceLink->getConnectedPins().front()->getActuatorDevice(); }

	//========= STATE ==========

	bool isEnabled() { return b_enabled; }
	bool b_enabled = false;

	virtual bool isReady();
	virtual bool isMoving();

	virtual void enable();
	virtual void disable();

	virtual void onEnable();
	virtual void onDisable();

	//========= MOTION PROFILE =========

	double profileTime_seconds = 0.0;
	double profileTimeDelta_seconds = 0.0;
	double profileVelocity_axisUnitsPerSecond = 0.0;
	double profileAcceleration_axisUnitsPerSecondSquared = 0.0;

	double getProfileVelocity_axisUnitsPerSecondSquared() { return profileVelocity_axisUnitsPerSecond; }
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

	virtual void setVelocity(double velocity_positionUnitsPerSecondSquare);
	virtual void fastStop();

	bool isAxisPinConnected() { return velocityControlledAxisLink->isConnected(); }
	virtual void sendActuatorCommands();

	//======= NODE ========

	std::shared_ptr<NodePin> actuatorDeviceLink = std::make_shared<NodePin>(NodeData::ACTUATOR_DEVICELINK, DataDirection::NODE_INPUT, "Actuator");
	
	std::shared_ptr<NodePin> velocityControlledAxisLink = std::make_shared<NodePin>(NodeData::VELOCITY_CONTROLLED_AXIS_LINK, DataDirection::NODE_OUTPUT, "Velocity Controlled Axis");
	std::shared_ptr<NodePin> velocity = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, "Velocity");

	//======= GUI ========

	virtual void nodeSpecificGui();
	virtual void controlsGui();
	virtual void settingsGui();
	virtual void devicesGui();
	virtual void metricsGui();

};