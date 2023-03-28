#pragma once

/*
#include "Motion/MotionTypes.h"

class Parameter;
class Machine;
class Animation;
class TargetAnimation;
class AnimatableComposite;
class AnimatableNumber;

class AnimatableState;
class AnimatablePosition;
class AnimatableBoolean;
class AnimatableReal;

class AnimationValue;
class AnimationConstraint;

namespace Motion{
struct ControlPoint;
};



class Animatable : public std::enable_shared_from_this<Animatable>{
public:
	
	virtual std::string getTargetValueString() = 0;
	
	//———————————— Contruction & Type Identification ————————————
	
	Animatable(std::string name_) : name(name_){}
	const char* getName(){ return name.c_str(); }
	void setName(std::string newName){ name = newName; }
	
	void setMachine(std::shared_ptr<Machine> machine_){ machine = machine_; }
	std::shared_ptr<Machine> getMachine(){ return machine; }
	
	bool hasParentComposite(){ return parentComposite != nullptr; }
	void setParentComposite(std::shared_ptr<AnimatableComposite> composite_){ parentComposite = composite_; }
	
	virtual bool isComposite(){ return false; }
	virtual bool isNumber(){ return false; }
	virtual AnimatableType getType() = 0;
	
	std::shared_ptr<AnimatableComposite> toComposite(){ return std::static_pointer_cast<AnimatableComposite>(shared_from_this()); }
	std::shared_ptr<AnimatableNumber> toNumber(){ return std::static_pointer_cast<AnimatableNumber>(shared_from_this()); }
	
	std::shared_ptr<AnimatableState> toState();
	std::shared_ptr<AnimatablePosition> toPosition();
	std::shared_ptr<AnimatableBoolean> toBoolean();
	std::shared_ptr<AnimatableReal> toReal();
	
	enum class State{
		OFFLINE,
		NOT_READY,
        HALTED,
		READY
	};
	
	State state = State::OFFLINE;
	std::string getStatusString();
	std::function<std::string(void)> machineStatusReturnMethod = [](){ return ""; };
	
	//———————————— Animations ————————————
		
	//animations which animate this animatable
	std::shared_ptr<Animation> makeAnimation(ManoeuvreType manoeuvreType);
	void subscribeAnimation(std::shared_ptr<Animation> animation);
	void unsubscribeAnimation(std::shared_ptr<Animation> animation);
	std::vector<std::shared_ptr<Animation>>& getAnimations(){ return animations; }
	
	virtual std::vector<std::string>& getCurveNames() = 0;
	virtual void fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint) = 0;
	
	void deleteAllAnimations();
	
	//animation currently animating this animatable
	bool hasAnimation(){ return currentAnimation != nullptr; }
	std::shared_ptr<Animation> getAnimation(){ return currentAnimation; }
	
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) = 0;
	virtual void updateTargetValue(double time_seconds, double deltaTime_seconds) = 0;
	virtual void followActualValue(double time_seconds, double deltaTime_seconds) = 0;
	
	std::shared_ptr<AnimationValue> getAnimationValue();
	virtual std::shared_ptr<AnimationValue> getActualValue() = 0;
	virtual std::shared_ptr<AnimationValue> getTargetValue() = 0;
	
	//—————————————— Constraints —————————————
	
public:
	
	void addConstraint(std::shared_ptr<AnimationConstraint> newConstraint);
	void clearConstraints(){
		constraints.clear();
	}
	std::vector<std::shared_ptr<AnimationConstraint>>& getConstraints(){ return constraints; }
	
	bool isHalted();
	
private:
	std::vector<std::shared_ptr<AnimationConstraint>> constraints;
	
	//—————————————— Commands ———————————————
	
public:
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> animation) = 0;
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) = 0;
	
	virtual bool isReadyToMove() = 0;
	virtual bool isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) = 0;
	
	void rapidToValue(std::shared_ptr<AnimationValue> animationValue);
	virtual bool isInRapid() = 0;
	virtual float getRapidProgress() = 0;
	virtual void cancelRapid() = 0;
	virtual std::shared_ptr<AnimationValue> getRapidTarget() = 0;
	
	virtual bool isMoving() = 0;
	virtual void stopMovement() = 0;
	void stopAnimation();
	
private:
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) = 0;
	virtual void onPlaybackStart(std::shared_ptr<Animation> animation) = 0;
	virtual void onPlaybackPause() = 0;
	virtual void onPlaybackStop() = 0;
	virtual void onPlaybackEnd() = 0;
	
	//———————————— Manual Controls —————————————
	
public:
	virtual bool hasManualControls() = 0;
	void setManualControlTarget(float x, float y = 0.0, float z = 0.0){
		stopAnimation();
		onSetManualControlTarget(x, y, z);
	}
	virtual void onSetManualControlTarget(float x, float y, float z) = 0;
	
	//—————————————— Virtual Methods ——————————
	
public:
	//implemented by each animatable type
	virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() = 0;
	virtual int getCurveCount() = 0;
	virtual std::shared_ptr<Parameter> makeParameter() = 0;
	virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) = 0;
	virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) = 0;
	virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) = 0;
	virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) = 0;
	virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) = 0;
	virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) = 0;
	virtual bool isControlledManuallyOrByAnimation() = 0;
	
private:
	
	friend class Animation;
	
	std::shared_ptr<Machine> machine;
	std::shared_ptr<AnimatableComposite> parentComposite;
	std::vector<std::shared_ptr<Animation>> animations;
	
public:
	std::mutex mutex;
	
public:
	std::string name;
	std::shared_ptr<Animation> currentAnimation;
};


//——————————————————— Number ————————————————————

class AnimatableNumber : public Animatable{
public:
	
	AnimatableNumber(const char* name, Unit unit_) : Animatable(name){
		setUnit(unit_);
	}

	virtual bool isNumber() override { return true; }
	bool isReal();
	
	void setUnit(Unit u);
	Unit getUnit(){ return unit; }
	
	//std::shared_ptr<NumberParameter<int>> floatingPointDisplayPrecision = NumberParameter<int>::make(1, "Floating Point Display Precision", "Precision");
	//void setPrecision(int precision){ format = "%." + std::to_string(precision) + "f"; }
	//const char* getFormat(){ return format.c_str(); }
	
private:
	Unit unit;
};






//——————————————————— Composite ————————————————————

class AnimatableComposite : public Animatable{
public:
	
	//construction and type identification
	AnimatableComposite(const char* name) : Animatable(name){}
	virtual bool isComposite() override { return true; }
	virtual AnimatableType getType() override { return AnimatableType::COMPOSITE; }
	
	//composite children
	void setChildren(std::vector<std::shared_ptr<Animatable>> children_);
	std::vector<std::shared_ptr<Animatable>>& getChildren(){ return children; }

private:
	std::vector<std::shared_ptr<Animatable>> children;
};
*/
