#pragma once

#include "NodeGraph/Node.h"

#include "Motion/MotionTypes.h"

#define DEFINE_AXIS_NODE(className, nodeName, saveName)	public:											\
		virtual const char * getSaveName() { return saveName; }											\
		virtual const char* getNodeCategory() { return "Axis"; }										\
		className(){ setName(nodeName); }																\
		virtual Node::Type getType() { return Node::Type::AXIS; }										\
		virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }	\
		virtual void assignIoData();\
		virtual void process();\

class Axis : public Node {
public:

	virtual MotionCommand::Type getMotionCommandType() = 0;

};