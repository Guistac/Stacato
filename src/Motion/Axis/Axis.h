#pragma once

#include "NodeGraph/Node.h"
#include "Motion/Subdevice.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Utilities/CircularBuffer.h"


#define DEFINE_AXIS_NODE(className, nodeName, saveName, axisType) public:							\
	virtual const char* getSaveName() { return saveName; }											\
	virtual const char* getNodeCategory() { return "Axis"; }										\
	className(){ setName(nodeName); }																\
	virtual Node::Type getType() { return Node::Type::AXIS; }										\
	virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }	\
	virtual void assignIoData();																	\
	virtual void process();																			\
	/*Axis specific*/																				\
	virtual Axis::Type getAxisType(){ return axisType; }											\
	virtual bool isEnabled();																		\
	virtual bool isReady();																			\
	virtual void enable();																			\
	virtual void disable();																			\
	virtual void controlsGui();																		\
	virtual void settingsGui();																		\
	virtual void devicesGui();																		\
	virtual void metricsGui();																		\
	virtual bool isMoving();																		\

class Axis : public Node {
public:

	enum class Type {
		POSITION_CONTROLLED_AXIS,
		VELOCITY_CONTROLLED_AXIS
	};

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void devicesGui() = 0;
	virtual void metricsGui() = 0;

	virtual void nodeSpecificGui();

	virtual bool isEnabled() = 0;
	virtual bool isReady() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual bool isMoving() = 0;

	//Profile Generator Variables
	//Command Timing
	//Motion Limits
	//Homing
	//Motion Controls
	//Fast Stop
	//Braking Position
};