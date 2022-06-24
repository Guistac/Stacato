#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Animation/Animatable.h"

namespace Motion {
	struct ControlPoint;
	class Interpolation;
	class Curve;
}

class Animation;
class TargetAnimation;
class Device;

namespace tinyxml2{ struct XMLElement; }

#define DEFINE_MACHINE_NODE(className, nodeName, saveName, category) public:\
	DEFINE_NODE(className, nodeName, saveName, Node::Type::MACHINE, category)\
	virtual void inputProcess();\
	virtual void outputProcess();\
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
	virtual void simulateInputProcess();\
	virtual void simulateOutputProcess();\
	virtual bool saveMachine(tinyxml2::XMLElement* xml);\
	virtual bool loadMachine(tinyxml2::XMLElement* xml);\
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output);\
	virtual void rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value);\
	virtual void cancelAnimatableRapid(std::shared_ptr<Animatable> animatable); \
	virtual float getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable); \
	virtual bool isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value);\
	virtual void onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable);\
	virtual void onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable);\
	virtual void onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable);\
	virtual std::shared_ptr<AnimationValue> getActualAnimatableValue(std::shared_ptr<Animatable> animatable);\
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation);\
	virtual bool validateAnimation(std::shared_ptr<Animation> animation);\
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> targetAnimation);\

#define DEFINE_HOMEABLE_MACHINE \
	virtual bool isHomeable() override { return true; }\
	virtual bool canStartHoming() override;\
	virtual bool isHoming() override;\
	virtual void startHoming() override;\
	virtual void stopHoming() override;\
	virtual bool didHomingSucceed() override;\
	virtual bool didHomingFail() override;\
	virtual float getHomingProgress() override;\
	virtual const char* getHomingStateString() override;\

class Machine : public Node {
public:
	
	//===== BASIC PROPRIETIES =====
	char shortName[16] = "M";
	const char* getShortName() { return shortName; }

	enum class State{
		EMERGENCY_STOP,
		DISABLED,
		READY,
		ENABLED,
		HALTED
	};
	
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
	virtual bool canStartHoming(){ return false; }
	virtual bool isHoming(){ return false; }
	virtual void startHoming(){}
	virtual void stopHoming(){}
	virtual bool didHomingSucceed(){}
	virtual bool didHomingFail(){}
	virtual float getHomingProgress(){ return 0.0; }
	virtual const char* getHomingStateString(){ return "default state string (something went wrong)"; }
	
	//===== PROCESSING =====
	virtual void inputProcess() override = 0;
	virtual void outputProcess() override = 0;
	virtual void simulateInputProcess() = 0;
	virtual void simulateOutputProcess() = 0;
	virtual bool needsOutputProcess() override { return true; }

	//===== PARAMETERS =====
	void addAnimatable(std::shared_ptr<Animatable> parameter);
	std::vector<std::shared_ptr<Animatable>> animatables;

	//===== RAPIDS =====
	virtual void rapidAnimatableToValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) = 0;
	virtual void cancelAnimatableRapid(std::shared_ptr<Animatable> animatable) = 0;
	virtual float getAnimatableRapidProgress(std::shared_ptr<Animatable> animatable) = 0;
	virtual bool isAnimatableReadyToStartPlaybackFromValue(std::shared_ptr<Animatable> animatable, std::shared_ptr<AnimationValue> value) = 0;

	//===== PLAYBACK CONTROL ======
	void startAnimationPlayback(std::shared_ptr<Animation> animation);
	void interruptAnimationPlayback(std::shared_ptr<Animatable> animatable);
	void endAnimationPlayback(std::shared_ptr<Animatable> animatable);
	virtual void onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) = 0;
	virtual void onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) = 0;
	virtual void onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) = 0;

	//====== PARAMETER VALUE =======
	virtual std::shared_ptr<AnimationValue> getActualAnimatableValue(std::shared_ptr<Animatable> animatable) = 0;

	//======= ANIMATION ======
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation) = 0;
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) = 0;
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> targetAnimation) = 0;

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
