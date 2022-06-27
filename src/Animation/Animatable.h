#pragma once

#include "Motion/MotionTypes.h"

class Parameter;
class Machine;
class Animation;

class AnimatableComposite;
class AnimatableNumber;

class AnimatableState;
class AnimatablePosition;
class AnimatableBoolean;

class AnimationValue;

class Animatable : public std::enable_shared_from_this<Animatable>{
public:
	
	//———————————— Contruction & Type Identification ————————————
	
	Animatable(std::string name_) : name(name_){}
	const char* getName(){ return name.c_str(); }
	
	void setMachine(std::shared_ptr<Machine> machine_){ machine = machine_; }
	std::shared_ptr<Machine> getMachine(){ return machine; }
	
	bool hasParentComposite(){ return parentComposite != nullptr; }
	void setParentComposite(std::shared_ptr<AnimatableComposite> composite_){ parentComposite = composite_; }
	
	virtual bool isComposite(){ return false; }
	virtual bool isNumber(){ return false; }
	virtual AnimatableType getType() = 0;
	
	std::shared_ptr<AnimatableComposite> toComposite(){ return std::dynamic_pointer_cast<AnimatableComposite>(shared_from_this()); }
	std::shared_ptr<AnimatableNumber> toNumber(){ return std::dynamic_pointer_cast<AnimatableNumber>(shared_from_this()); }
	
	std::shared_ptr<AnimatableState> toState();
	std::shared_ptr<AnimatablePosition> toPosition();
	std::shared_ptr<AnimatableBoolean> toBoolean();
	
	//———————————— Animations ————————————
		
	//animations which animate this animatable
	std::shared_ptr<Animation> makeAnimation(ManoeuvreType manoeuvreType);
	void subscribeAnimation(std::shared_ptr<Animation> animation);
	void unsubscribeAnimation(std::shared_ptr<Animation> animation);
	std::vector<std::shared_ptr<Animation>>& getAnimations(){ return animations; }
	
	//animation currently animating this animatable
	bool hasAnimation(){ return currentAnimation != nullptr; }
	std::shared_ptr<Animation> getAnimation(){ return currentAnimation; }
	void setAnimation(std::shared_ptr<Animation> animation){ currentAnimation = animation; }
	void stopAnimationPlayback();
	
	//animatable value retrieval
	std::shared_ptr<AnimationValue> getAnimationValue();
	std::shared_ptr<AnimationValue> getActualValue();
	
	//—————————————— Virtual Methods ——————————
	
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
	
private:
	
	std::string name;
	std::shared_ptr<Machine> machine;
	std::shared_ptr<AnimatableComposite> parentComposite;
	std::shared_ptr<Animation> currentAnimation;
	std::vector<std::shared_ptr<Animation>> animations;
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
