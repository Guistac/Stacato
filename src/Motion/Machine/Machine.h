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

	bool isEnabled() { return b_isEnabled; }
	bool isReady() { return b_isReady; }
	void enable() { b_setEnable = true; }

private:

	bool b_setEnable = false;
	bool b_isReady = false;
	bool b_isEnabled = false;
};