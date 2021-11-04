#pragma once

#include "NodeGraph/Node.h"

class AnimatableParameter;
class AnimatableParameterValue;
class Device;

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
	virtual float getMiniatureWidth();																\
	virtual void machineSpecificMiniatureGui();														\
	virtual bool isEnabled();																		\
	virtual bool isReady();																			\
	virtual bool isMoving();																		\
	virtual void enable();																			\
	virtual void disable();																			\
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output);							\
	virtual void primeParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual float getParameterPrimingProgress(std::shared_ptr<AnimatableParameter> parameter);\
	virtual bool isParameterPrimedToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\

class Machine : public Node {
public:

	const char shortName[16] = "M-1";
	const char* getShortName() { return shortName; }

	void powerControlGui();
	virtual void nodeSpecificGui();

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void axisGui() = 0;
	virtual void deviceGui() = 0;
	virtual void metricsGui() = 0;

	void miniatureGui();

	virtual float getMiniatureWidth() = 0;
	virtual void machineSpecificMiniatureGui() = 0;

	virtual bool isEnabled() = 0;
	virtual bool isReady() = 0;
	virtual bool isMoving() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;




	//interface to plots / manoeuvres / tracks
	std::vector<std::shared_ptr<AnimatableParameter>> animatableParameters;

	virtual void primeParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;
	virtual float getParameterPrimingProgress(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual bool isParameterPrimedToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;
	virtual void stopParameterPlayback();

	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) = 0;


	//TODO: reference to stage geometry

	//TODO: a way to be interrogated about the limits of a certain animatable parameter
	//be it spatial limits of the actual parameter
	//or kinematic limits like velocity and acceleration



};

