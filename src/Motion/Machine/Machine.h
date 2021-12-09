#pragma once

#include "NodeGraph/Node.h"
#include "Motion/AnimatableParameter.h"

namespace Motion {
	struct ControlPoint;
	class Interpolation;
	class Curve;
}

class ParameterTrack;
class Device;

namespace tinyxml2{ struct XMLElement; }

#define DEFINE_MACHINE_NODE(className, nodeName, saveName, category) public:						\
	virtual const char* getSaveName() { return saveName; }											\
	virtual const char* getNodeCategory() { return category; }										\
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
	virtual void rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual void cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter); \
	virtual float getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter); \
	virtual bool isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual void onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter);\
	virtual void onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter);\
	virtual void getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual bool validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack);\
	virtual void getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves);\
	virtual bool getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit);\
	virtual void enterSimulationMode();\
	virtual void exitSimulationMode();\
	virtual bool isInSimulationMode();\

class Machine : public Node {
public:

	char shortName[16] = "M";
	const char* getShortName() { return shortName; }

	void addAnimatableParameter(std::shared_ptr<AnimatableParameter> parameter);

	virtual void nodeSpecificGui();
	virtual void stateControlGui();
	virtual void generalSettingsGui();
	void miniatureGui();

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void axisGui() = 0;
	virtual void deviceGui() = 0;
	virtual void metricsGui() = 0;

	virtual float getMiniatureWidth() = 0;
	virtual void machineSpecificMiniatureGui() = 0;

	virtual bool isEnabled() = 0;
	virtual bool isReady() = 0;
	virtual bool isMoving() = 0;

	virtual void enable() = 0;
	virtual void disable() = 0;
	
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	
	virtual bool saveMachine(tinyxml2::XMLElement* xml){ return true; }
	virtual bool loadMachine(tinyxml2::XMLElement* xml){ return true; }
	
	

	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) = 0;



	//interface to plots / manoeuvres / tracks
	std::vector<std::shared_ptr<AnimatableParameter>> animatableParameters;

	//===== RAPIDS =====
	virtual void rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;
	virtual void cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual float getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual bool isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;

	//===== PLAYBACK CONTROL ======
	void startParameterPlayback(std::shared_ptr<ParameterTrack> track);
	void stopParameterPlayback(std::shared_ptr<AnimatableParameter> parameter);
	virtual void onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual void onParameterPlaybackStop(std::shared_ptr<AnimatableParameter> parameter) = 0;

	//====== PARAMETER VALUE =======
	virtual void getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;

	//======= PARAMETER TRACK VALIDATION ======
	virtual bool validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) = 0;
	virtual bool getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) = 0;
	
	//======= TIMED MOVEMENT ======
	virtual void getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) = 0;


	//TODO: reference to stage geometry, mesh index, available meshes...

};
