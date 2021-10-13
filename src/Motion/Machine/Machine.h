#pragma once

#include "NodeGraph/Node.h"
#include "Motion/Subdevice.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Utilities/CircularBuffer.h"


#define DEFINE_MACHINE_NODE(nodeName, className, machineType)	public:																							\
																virtual const char* getNodeName() { return nodeName; }											\
																virtual const char* getNodeCategory() { return "Machine"; }										\
																className(){ setName(nodeName); }																\
																virtual Node::Type getType() { return Node::Type::MACHINE; }									\
																virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }	\
																/*machine specific*/																			\
																virtual Machine::Type getMachineType(){ return machineType; }									\
																virtual bool isEnabled();																		\
																virtual bool isReady();																			\
																virtual void enable();																			\
																virtual void disable();																			\
																virtual void controlsGui();																		\
																virtual void settingsGui();																		\
																virtual void devicesGui();																		\
																virtual void metricsGui();																		\
																virtual void miniatureGui();																	\

class Machine : public Node {
public:

	enum class Type {
		SINGLE_AXIS_MACHINE,
		STATE_MACHINE
	};

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void devicesGui() = 0;
	virtual void metricsGui() = 0;
	virtual void miniatureGui() = 0;

	virtual void nodeSpecificGui();

	//reference to stage geometry
	//reference to a parent axis
	//vector of animatable parameters
	//machine unique number

	virtual bool isEnabled() = 0;
	virtual bool isReady() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
};