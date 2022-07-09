#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"
#include "Animation/AnimationConstraint.h"

//core state structure
struct AnimatableStateStruct{
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};



struct AnimatableStateValue : public AnimationValue{
	virtual AnimatableType getType() override { return AnimatableType::STATE; }
	AnimatableStateStruct* value;
	std::vector<AnimatableStateStruct>* values;
};



class AnimatableState_StateConstraint : public AnimationConstraint{
public:
	
	virtual AnimatableType getType() override { return AnimatableType::STATE; }
	
	AnimatableState_StateConstraint(std::string name, std::vector<AnimatableStateStruct>* values_)
	: AnimationConstraint(name), values(values_) {}
	
	void allowAllStates(){
		allowedStates.clear();
		allowedStates.reserve(values->size());
		auto& stateValues = *values;
		for(auto& value : stateValues) allowedStates.push_back(&value);
	}
	void forbidAllStates(){
		allowedStates.clear();
	}
	void allowState(AnimatableStateStruct* state){
		allowedStates.push_back(state);
	}
	void forbidState(AnimatableStateStruct* state){
		for(int i = allowedStates.size() - 1; i >= 0; i--){
			if(allowedStates[i] == state){
				allowedStates.erase(allowedStates.begin() + i);
				break;
			}
		}
	}
	
	std::vector<AnimatableStateStruct>* values;
	std::vector<AnimatableStateStruct*> allowedStates;
	
};



class AnimatableState : public Animatable{
public:
	
	//construction & type identification
	AnimatableState(const char* name, std::vector<AnimatableStateStruct>* stateValues) : Animatable(name), states(stateValues){
		auto defaultValue = AnimationValue::makeState();
		defaultValue->value = &stateValues->front();
		updateActualValue(defaultValue);
	};
	static std::shared_ptr<AnimatableState> make(std::string name, std::vector<AnimatableStateStruct>* stateValues){
		return std::make_shared<AnimatableState>(name.c_str(), stateValues);
	}
	virtual AnimatableType getType() override { return AnimatableType::STATE; }

	virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() override;
	virtual int getCurveCount() override;
	virtual std::shared_ptr<Parameter> makeParameter() override;
	virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) override;
	virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) override;
	virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) override;
	virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) override;
	virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) override;
	virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) override;

	virtual bool isReadyToMove() override {}
	virtual bool isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue) override {}
	virtual bool isInRapid() override {}
	virtual float getRapidProgress() override {}
	virtual void cancelRapid() override {}
	
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) override {}
	virtual void onPlaybackStart() override{}
	virtual void onPlaybackPause() override{}
	virtual void onPlaybackStop() override{}
	virtual void onPlaybackEnd() override{}
	virtual void stopMovement() override {}
	
	virtual bool hasManualControls() override { return true; }
	virtual void onSetManualControlTarget(float x, float y, float z) override {}
	
	
	virtual void updateTargetValue(double time_seconds, double deltaTime_seconds) override{}
	virtual std::shared_ptr<AnimationValue> getTargetValue() override{}
	
	virtual void updateActualValue(std::shared_ptr<AnimationValue> newActualValue) override{}
	virtual std::shared_ptr<AnimationValue> getActualValue() override{
		auto actual = AnimationValue::makeState();
		actual->value = &states->front();
		return actual;
	}
	
	virtual void followActualValue(double time_seconds, double deltaTime_seconds) override {};
	
	
	
	
	
	
	
	
	virtual bool generateTargetAnimation(std::shared_ptr<TargetAnimation> animation) override {
		return false;
	}
	
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) override {
		return false;
	}
	
	std::vector<AnimatableStateStruct>& getStates() { return *states; }
	
private:
	std::vector<AnimatableStateStruct>* states;
};

