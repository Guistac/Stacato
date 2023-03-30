#pragma once

#include "Environnement/NodeGraph/Node.h"
#include "Motion/MotionTypes.h"

#include "Animation/NewAnimation/Animatable.h"

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
	virtual std::string getStatusString() override;																								\
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
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation) override;															\

class Machine : public Node {
public:
	
	//===== BASIC PROPRIETIES =====
	char shortName[16] = "M";
	const char* getShortName() { return shortName; }
	
	DeviceState getState(){ return state; }
	DeviceState state = DeviceState::OFFLINE;
	virtual std::string getStatusString() = 0;
	
	bool b_emergencyStopActive = false;
	bool isEmergencyStopped(){ return b_emergencyStopActive; }
	
	//a machine can be halted by the dead mans switch or a constraint
	bool b_halted = false;
	bool isHalted(){ return b_halted; }
	
	//===== STATE CONTROL & MONITORING =====
	
public:
	
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
	
	virtual void addConstraints(){}
	
	//===== PROCESSING =====
	virtual void inputProcess() override = 0;
	virtual void outputProcess() override = 0;
	virtual void simulateInputProcess() = 0;
	virtual void simulateOutputProcess() = 0;
	virtual bool needsOutputProcess() override { return true; }
	
	//===== ANIMATABLES ======
	
	virtual void fillAnimationDefaults(std::shared_ptr<Animation> animation) = 0;
	
	//void addAnimatable(std::shared_ptr<AnimationSystem::Animatable> animatable);
	//void removeAnimatable(std::shared_ptr<AnimationSystem::Animatable> animatable);
	//std::vector<std::shared_ptr<AnimationSystem::Animatable>>& getAnimatables(){ return animatables; }
	
	//===== ATTACHED DEVICES =====
	
	virtual void getDevices(std::vector<std::shared_ptr<Device>>& output) = 0;
	
	std::shared_ptr<NodePin> deadMansSwitchPin = std::make_shared<NodePin>(NodePin::DataType::DEAD_MANS_SWITCH,
																		   NodePin::Direction::NODE_INPUT_BIDIRECTIONAL,
																		   "Dead Man's Switch", "DeadMansSwitch",
																		   NodePin::Flags::AcceptMultipleInputs);
	
	bool isMotionAllowed();
	
	//===== GUI STUFF =====
	virtual void nodeSpecificGui() override;
	virtual void stateControlGui();
	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void axisGui() = 0;
	virtual void deviceGui() = 0;
	virtual void metricsGui() = 0;
	
	virtual bool hasSetupGui(){ return false; }
	virtual void setupGui(){}
	
	virtual bool hasAxis(){ return false; }
	virtual void axisSetupGui(){}
	
	glm::vec2 reserveSpaceForMachineHeaderGui();
	void machineHeaderGui(glm::vec2 position, float width = 0.0);
	
	void machineHeaderGui(float width);
	void machineStateControlGui(float width);
	
	//===== SAVING & LOADING =====
	virtual bool save(tinyxml2::XMLElement* xml) override;
	virtual bool load(tinyxml2::XMLElement* xml) override;
	virtual bool saveMachine(tinyxml2::XMLElement* xml) = 0;
	virtual bool loadMachine(tinyxml2::XMLElement* xml) = 0;
	
private:
	std::vector<std::shared_ptr<AnimationSystem::Animatable>> animatables;

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
