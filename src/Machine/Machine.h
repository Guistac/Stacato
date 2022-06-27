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



#define DEFINE_MACHINE_NODE(className, nodeName, saveName, category) public:																	\
	/*Node Specific*/																															\
	virtual const char* getSaveName() override { return saveName; }																				\
	virtual const char* getNodeCategory() override { return category; }																			\
	className(){ setName(nodeName); }																											\
	virtual Node::Type getType() override { return Node::Type::MACHINE; }																		\
	virtual std::shared_ptr<Node> getNewInstance() override {																					\
		std::shared_ptr<Machine> newMachineInstance = std::make_shared<className>();															\
		newMachineInstance->addNodePin(newMachineInstance->deadMansSwitchPin);																	\
		newMachineInstance->initialize();																										\
		return newMachineInstance;																												\
	}																																			\
	virtual void initialize() override;																											\
	virtual void inputProcess() override;																										\
	virtual void outputProcess()override;																										\
	/*Machine Specific*/																														\
	virtual void controlsGui() override;																										\
	virtual void settingsGui() override;																										\
	virtual void axisGui() override;																											\
	virtual void deviceGui() override;																											\
	virtual void metricsGui() override;																											\
	virtual bool isMoving() override;																											\
	virtual bool isHardwareReady() override;																									\
	virtual bool isSimulationReady() override;																									\
	virtual void enableHardware() override;																										\
	virtual void disableHardware() override;																									\
	virtual void onEnableHardware() override;																									\
	virtual void onDisableHardware() override;																									\
	virtual void onEnableSimulation() override;																									\
	virtual void onDisableSimulation() override;																								\
	virtual void simulateInputProcess() override;																								\
	virtual void simulateOutputProcess() override;																								\
	virtual bool saveMachine(tinyxml2::XMLElement* xml) override;																				\
	virtual bool loadMachine(tinyxml2::XMLElement* xml) override;																				\
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) override;																\
	/*AnimatableOwner Specific*/																												\
	virtual void onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) override;														\
	virtual void onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) override;													\
	virtual void onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) override;														\
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation) override;															\

#define DEFINE_HOMEABLE_MACHINE 							\
	virtual bool isHomeable() override { return true; }		\
	virtual bool canStartHoming() override;					\
	virtual bool isHoming() override;						\
	virtual void startHoming() override;					\
	virtual void stopHoming() override;						\
	virtual bool didHomingSucceed() override;				\
	virtual bool didHomingFail() override;					\
	virtual float getHomingProgress() override;				\
	virtual const char* getHomingStateString() override;	\

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
	
	//===== ANIMATABLES ======
		
	virtual void onAnimationPlaybackStart(std::shared_ptr<Animatable> animatable) = 0;
	virtual void onAnimationPlaybackInterrupt(std::shared_ptr<Animatable> animatable) = 0;
	virtual void onAnimationPlaybackEnd(std::shared_ptr<Animatable> animatable) = 0;
	
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation) = 0;
	
	void addAnimatable(std::shared_ptr<Animatable> animatable);
	std::vector<std::shared_ptr<Animatable>>& getAnimatables(){ return animatables; }
	
	//===== ATTACHED DEVICES =====
	
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) = 0;
	
	std::shared_ptr<NodePin> deadMansSwitchPin = std::make_shared<NodePin>(NodePin::DataType::DEAD_MANS_SWITCH,
																		   NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																		   "Dead Man's Switch", "DeadMansSwitch",
																		   NodePin::Flags::AcceptMultipleInputs);
	
	//===== GUI STUFF =====
	virtual void nodeSpecificGui() override;
	virtual void stateControlGui();
	virtual void generalSettingsGui();
	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void axisGui() = 0;
	virtual void deviceGui() = 0;
	virtual void metricsGui() = 0;
	
	void machineHeaderGui(float width);
	void machineStateControlGui(float width);
	
	//===== SAVING & LOADING =====
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool saveMachine(tinyxml2::XMLElement* xml) = 0;
	virtual bool loadMachine(tinyxml2::XMLElement* xml) = 0;
	
private:
	std::vector<std::shared_ptr<Animatable>> animatables;

};

//animatable controls the virtual target value
//rapidAnimatableToValue()
//cancelAnimatableRapid()
//getAnimatableRapidProgress()
//startAnimationPlayback()
//interruptAnimationPlayback()
//endAnimationPlayback()

//animatable owner: generates the outputs to follow the virtual target
//isAnimatableReadyToStartPlaybackFromValue()
//also is ready to rapid / move ?
//onAnimationPlaybackStart()
//onAnimationPlaybackInterrupt()
//onAnimationPlaybackEnd()
//getActualAnimatableValue()
//fillAnimationDefaults()
//validateAnimation()
//generateTargetAnimation()
