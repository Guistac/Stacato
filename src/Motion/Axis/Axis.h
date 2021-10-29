#pragma once

#include "NodeGraph/Node.h"
#include "Motion/Subdevice.h"
#include "Motion/MotionTypes.h"
#include "Motion/Curves/Curve.h"
#include "Utilities/CircularBuffer.h"


#define DEFINE_AXIS_NODE(className, nodeName, saveName, commandType) public:						\
	virtual const char* getSaveName() { return saveName; }											\
	virtual const char* getNodeCategory() { return "Axis"; }										\
	className(){ setName(nodeName); }																\
	virtual Node::Type getType() { return Node::Type::AXIS; }										\
	virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }	\
	virtual void assignIoData();																	\
	virtual void process();																			\
	/*Axis specific*/																				\
	virtual bool isEnabled();																		\
	virtual bool isReady();																			\
	virtual void enable();																			\
	virtual void disable();																			\
	virtual void controlsGui();																		\
	virtual void settingsGui();																		\
	virtual void devicesGui();																		\
	virtual void metricsGui();																		\
	virtual bool isMoving();																		\
	virtual double getLowPositionLimit();															\
	virtual double getHighPositionLimit();															\
	virtual void setActuatorCommands();																\
	virtual bool isHomeable();																		\
	virtual void startHoming();																		\
	virtual void cancelHoming();																	\
	virtual bool isHoming();																		\
	virtual bool didHomingSucceed();																\
	virtual bool didHomingFail();																	\
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output);							\

class Axis : public Node {
public:

	virtual MotionCommand::Type getCommandType() = 0;

	PositionUnit::Unit axisPositionUnit = PositionUnit::Unit::DEGREE;
	PositionUnit::Type axisPositionUnitType = PositionUnit::Type::ANGULAR;

	virtual void nodeSpecificGui();

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void devicesGui() = 0;
	virtual void metricsGui() = 0;

	virtual bool isEnabled() = 0;
	virtual bool isReady() = 0;
	virtual bool isMoving() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;

	//actual machine state based on feedback data
	double actualPosition_axisUnits;
	double actualVelocity_axisUnitsPerSecond;

	//motion profile generator variables
	double previousProfilePointTime_seconds = 0.0;
	double currentProfilePointTime_seconds = 0.0;
	double currentProfilePointDeltaT_seconds = 0.0;
	double profilePosition_axisUnits = 0.0;
	double profileVelocity_axisUnitsPerSecond = 0.0;
	double profileAcceleration_axisUnitsPerSecondSquared = 0.0;
	double actualLoad = 0.0;

	//Kinematic Limits
	double velocityLimit_axisUnitsPerSecond = 0.0;
	double accelerationLimit_axisUnitsPerSecondSquared = 0.0;

	//Position Limits
	virtual double getLowPositionLimit() = 0;
	virtual double getHighPositionLimit() = 0;
	double getPositionProgress() {
		double low = getLowPositionLimit();
		double high = getHighPositionLimit();
		return (actualPosition_axisUnits - low) / (high - low);
	}

	virtual bool isHomeable() = 0;
	virtual void startHoming() = 0;
	virtual void cancelHoming() = 0;
	virtual bool isHoming() = 0;
	virtual bool didHomingSucceed() = 0;
	virtual bool didHomingFail() = 0;

	virtual void setActuatorCommands() = 0;

	bool b_manualControlsEnabled = false;
	bool hasManualControlsEnabled() { return b_manualControlsEnabled; }

	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) = 0;

	//Fast Stop
	//Braking Position
};