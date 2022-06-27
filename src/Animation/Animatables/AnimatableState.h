#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"

//core state structure
struct AnimatableStateStruct{
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};

struct AnimatableStateValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::STATE; }
	AnimatableStateStruct* value;
	std::vector<AnimatableStateStruct>* values;
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
	virtual void onRapidToValue(std::shared_ptr<AnimationValue> animationValue) override {}
	virtual bool isInRapid() override {}
	virtual float getRapidProgress() override {}
	virtual void cancelRapid() override {}
	
	
	virtual bool generateTargetAnimation(std::shared_ptr<Animation> animation) override {
		return false;
	}
	
	virtual bool validateAnimation(std::shared_ptr<Animation> animation) override {
		return false;
	}
	
	std::vector<AnimatableStateStruct>& getStates() { return *states; }
	
private:
	std::vector<AnimatableStateStruct>* states;
};

