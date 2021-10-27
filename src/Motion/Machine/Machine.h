#pragma once

#include "NodeGraph/Node.h"
#include "AnimatableParameter.h"

#define DEFINE_MACHINE_NODE(className, nodeName, saveName) public:									\
	virtual const char* getSaveName() { return saveName; }											\
	virtual const char* getNodeCategory() { return "Machine"; }										\
	className(){ setName(nodeName); }																\
	virtual Node::Type getType() { return Node::Type::MACHINE; }									\
	virtual std::shared_ptr<Node> getNewNodeInstance() { return std::make_shared<className>(); }	\
	virtual void assignIoData();																	\
	virtual void process();																			\
	/*Machine Specific*/																			\
	virtual void controlsGui();																		\
	virtual void settingsGui();																		\
	virtual void axisGui();																			\
	virtual void deviceGui();																		\
	virtual void metricsGui();																		\
	virtual void miniatureGui();																	\
	virtual bool isEnabled();																		\
	virtual bool isReady();																			\
	virtual bool isMoving();																		\
	virtual void enable();																			\
	virtual void disable();																			\
	virtual void moveToParameter();																	\

class Machine : public Node {

	virtual void nodeSpecificGui();

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void axisGui() = 0;
	virtual void deviceGui() = 0;
	virtual void metricsGui() = 0;
	virtual void miniatureGui() = 0;

	virtual bool isEnabled() = 0;
	virtual bool isReady() = 0;
	virtual bool isMoving() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;

	virtual void moveToParameter() = 0;

	std::vector<AnimatableParameter> animatableParameters;

	//reference to stage geometry
	//vector of animatable parameters
	//machine unique number
};

