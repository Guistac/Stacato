#pragma once

#include "Environnement/Node.h"
#include "Machine/AnimatableParameter.h"

namespace Motion {
	struct ControlPoint;
	class Interpolation;
	class Curve;
}

class ParameterTrack;
class Device;

namespace tinyxml2{ struct XMLElement; }

#define DEFINE_MACHINE_NODE(className, nodeName, saveName, category) public:\
	DEFINE_NODE(className, nodeName, saveName, Node::Type::MACHINE, category)\
	virtual void process();\
	/*Machine Specific*/\
	virtual void controlsGui();\
	virtual void settingsGui();\
	virtual void axisGui();\
	virtual void deviceGui();\
	virtual void metricsGui();\
	virtual float getMiniatureWidth();\
	virtual void machineSpecificMiniatureGui();\
	virtual bool isMoving();\
	virtual bool isHardwareReady();\
	virtual bool isSimulationReady();\
	virtual void enableHardware();\
	virtual void disableHardware();\
	virtual void onEnableHardware();\
	virtual void onDisableHardware();\
	virtual void onEnableSimulation();\
	virtual void onDisableSimulation();\
	virtual void simulateProcess();\
	virtual bool saveMachine(tinyxml2::XMLElement* xml);\
	virtual bool loadMachine(tinyxml2::XMLElement* xml);\
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output);\
	virtual void rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual void cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter); \
	virtual float getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter); \
	virtual bool isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual void onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter);\
	virtual void onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter);\
	virtual void onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter);\
	virtual void getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value);\
	virtual bool validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack);\
	virtual void getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves);\
	virtual bool getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit);\

#define DEFINE_HOMEABLE_MACHINE \
	virtual bool isHomeable(){ return true; }\
	virtual bool isHoming();\
	virtual void startHoming();\
	virtual void stopHoming();\
	virtual bool didHomingSucceed();\
	virtual bool didHomingFail();\
	virtual float getHomingProgress();\
	virtual const char* getHomingStateString();\

class Machine : public Node {
public:

	//===== BASIC PROPRIETIES =====
	char shortName[16] = "M";
	const char* getShortName() { return shortName; }

	//===== STATE CONTROL & MONITORING =====
	bool b_enabled = false;
	bool isReady();
	void enable();
	void disable();
	bool isEnabled();
	bool isSimulating();
	virtual bool isHardwareReady() = 0;
	virtual bool isSimulationReady() = 0;
	virtual void enableHardware() = 0;
	virtual void disableHardware() = 0;
	virtual bool isMoving() = 0;
	virtual void onEnableSimulation() = 0;
	virtual void onDisableSimulation() = 0;
	virtual void onEnableHardware() = 0;
	virtual void onDisableHardware() = 0;
	
	virtual bool isHomeable(){ return false; }
	virtual bool isHoming(){ return false; }
	virtual void startHoming(){}
	virtual void stopHoming(){}
	virtual bool didHomingSucceed(){}
	virtual bool didHomingFail(){}
	virtual float getHomingProgress(){ return 0.0; }
	virtual const char* getHomingStateString(){ return "default state string (something went wrong)"; }
	
	//===== PROCESSING =====
	virtual void process() = 0;
	virtual void simulateProcess() = 0;

	//===== PARAMETERS =====
	void addAnimatableParameter(std::shared_ptr<AnimatableParameter> parameter);
	std::vector<std::shared_ptr<AnimatableParameter>> animatableParameters;

	//===== RAPIDS =====
	virtual void rapidParameterToValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;
	virtual void cancelParameterRapid(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual float getParameterRapidProgress(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual bool isParameterReadyToStartPlaybackFromValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;

	//===== PLAYBACK CONTROL ======
	void startParameterPlayback(std::shared_ptr<ParameterTrack> track);
	void interruptParameterPlayback(std::shared_ptr<AnimatableParameter> parameter);
	void endParameterPlayback(std::shared_ptr<AnimatableParameter> parameter);
	virtual void onParameterPlaybackStart(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual void onParameterPlaybackInterrupt(std::shared_ptr<AnimatableParameter> parameter) = 0;
	virtual void onParameterPlaybackEnd(std::shared_ptr<AnimatableParameter> parameter) = 0;

	//====== PARAMETER VALUE =======
	virtual void getActualParameterValue(std::shared_ptr<AnimatableParameter> parameter, AnimatableParameterValue& value) = 0;

	//======= PARAMETER TRACK VALIDATION ======
	virtual bool validateParameterTrack(const std::shared_ptr<ParameterTrack> parameterTrack) = 0;
	virtual bool getCurveLimitsAtTime(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::Curve>>& parameterCurves, double time, const std::shared_ptr<Motion::Curve> queriedCurve, double& lowLimit, double& highLimit) = 0;
	
	//======= TIMED MOVEMENT ======
	virtual void getTimedParameterCurveTo(const std::shared_ptr<AnimatableParameter> parameter, const std::vector<std::shared_ptr<Motion::ControlPoint>> targetPoints, double time, double rampIn, const std::vector<std::shared_ptr<Motion::Curve>>& outputCurves) = 0;

	//===== ATTACHED DEVICES =====
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) = 0;
	
	//===== GUI STUFF =====
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
	
	//===== SAVING & LOADING =====
	virtual bool save(tinyxml2::XMLElement* xml);
	virtual bool load(tinyxml2::XMLElement* xml);
	virtual bool saveMachine(tinyxml2::XMLElement* xml) = 0;
	virtual bool loadMachine(tinyxml2::XMLElement* xml) = 0;

	//TODO: reference to stage geometry, mesh index, available meshes...
};
