#pragma once

#include "AnimatableParameterValue.h"
#include "Motion/Curve/Curve.h"
#include "Project/Editor/Parameter.h"

class Machine;
namespace tinyxml2 { class XMLElement; }


class Animation;
class AnimatableComposite;
class AnimatableNumber;
class AnimatableState;

class Animatable : public std::enable_shared_from_this<Animatable>{
public:
	
	//———————————— CONSTRUCTION & TYPE IDENTIFICATION ————————————
	
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
	std::shared_ptr<AnimatableState> toState(){ return std::dynamic_pointer_cast<AnimatableState>(shared_from_this()); }
	
	//———————————— ANIMATIONS ————————————
	
	std::shared_ptr<Animation> makeAnimation(ManoeuvreType manoeuvreType);
	std::vector<Motion::Interpolation::Type>& getCompatibleInterpolationTypes();
	
	std::vector<std::shared_ptr<Animation>>& getAnimations(){ return animations; }
	void subscribeAnimation(std::shared_ptr<Animation> animation){ animations.push_back(animation); }
	void unsubscribeTrack(std::shared_ptr<Animation> animation){
		for(int i = 0; i < animations.size(); i++){
			if(animations[i] == animation) {
				animations.erase(animations.begin() + i);
				break;
			}
		}
	}
	
	bool hasAnimation(){ return currentAnimation != nullptr; }
	std::shared_ptr<Animation> getAnimation(){ return currentAnimation; }
	void setAnimation(std::shared_ptr<Animation> animation){ currentAnimation = animation; }
	
	void stopAnimationPlayback();
	
	//———————————— ANIMATION VALUES ————————————
	
	std::shared_ptr<AnimationValue> getAnimationValue();
	std::shared_ptr<AnimationValue> getActualValue();
	
	int getCurveCount();
	std::shared_ptr<Parameter> makeParameter();
	void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value);
	void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to);
	std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter);
	bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2);
	std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds);
	std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value);
	
private:
	
	std::string name;
	std::shared_ptr<Machine> machine;
	std::shared_ptr<AnimatableComposite> parentComposite;
	std::shared_ptr<Animation> currentAnimation;
	std::vector<std::shared_ptr<Animation>> animations;
};













//———————————————————————————————————————————————
//				ANIMATABLE NUMBER
//———————————————————————————————————————————————

class AnimatableNumber : public Animatable{
public:
	
	AnimatableNumber(const char* name, AnimatableType type_, Unit unit_) : Animatable(name), type(type_){
		assert(type != AnimatableType::COMPOSITE);
		assert(type != AnimatableType::BOOLEAN);
		assert(type != AnimatableType::STATE);
		setUnit(unit_);
	}

	virtual bool isNumber() override { return true; }
	virtual AnimatableType getType() override { return type; }
	
	bool isReal(){
		switch(type){
			case AnimatableType::INTEGER: return false;
			default: return true;
		}
	}
	
	Unit getUnit(){ return unit; }
	void setUnit(Unit u);
	
	//std::shared_ptr<NumberParameter<int>> floatingPointDisplayPrecision = NumberParameter<int>::make(1, "Floating Point Display Precision", "Precision");
	//void setPrecision(int precision){ format = "%." + std::to_string(precision) + "f"; }
	//const char* getFormat(){ return format.c_str(); }
	
private:
	Unit unit;
	AnimatableType type;
};



//———————————————————————————————————————————————
//				ANIMATABLE STATE
//———————————————————————————————————————————————

class AnimatableState : public Animatable{
public:
	
	AnimatableState(const char* name, std::vector<StateAnimationValue::Value>* stateValues) : Animatable(name), states(stateValues){};
	
	virtual AnimatableType getType() override { return AnimatableType::STATE; }
	
	std::vector<StateAnimationValue::Value>& getStates() { return *states; }
	
private:
	std::vector<StateAnimationValue::Value>* states;
};



//———————————————————————————————————————————————
//				ANIMATABLE BOOLEAN
//———————————————————————————————————————————————

class AnimatableBoolean : public Animatable{
public:
	
	AnimatableBoolean(const char* name) : Animatable(name){}
	
	virtual AnimatableType getType() override { return AnimatableType::BOOLEAN; }
};



//———————————————————————————————————————————————
//				ANIMATABLE COMPOSITE
//———————————————————————————————————————————————

class AnimatableComposite : public Animatable{
public:
	
	AnimatableComposite(const char* name) : Animatable(name){}
	
	void setChildren(std::vector<std::shared_ptr<Animatable>> children_){
		children = children_;
		for(auto& childAnimatable : children) childAnimatable->setParentComposite(toComposite());
	}
	
	virtual bool isComposite() override { return true; }
	virtual AnimatableType getType() override { return AnimatableType::COMPOSITE; }
	
	std::vector<std::shared_ptr<Animatable>>& getChildren(){ return children; }

private:
	std::vector<std::shared_ptr<Animatable>> children;
};
