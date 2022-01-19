#pragma once

#include "NodeGraph/Node.h"

#include "Motion/MotionTypes.h"

#define DEFINE_AXIS_NODE(className, nodeName, saveName)	\
		DEFINE_NODE(className, nodeName, saveName, Node::Type::AXIS, "")\
		virtual void process();\

class Axis : public Node {
public:

	virtual MotionCommand getMotionCommandType() = 0;

};
