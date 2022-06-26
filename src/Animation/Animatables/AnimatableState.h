#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"

//core state structure
struct AnimatableStateStruct{
	int integerEquivalent;
	const char displayName[64];
	const char saveName[64];
};

class AnimatableState : public Animatable{
public:
	
	//type specifier
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

	std::vector<AnimatableStateStruct>& getStates() { return *states; }
	
	//construction
	AnimatableState(const char* name, std::vector<AnimatableStateStruct>* stateValues) : Animatable(name), states(stateValues){};
	static std::shared_ptr<AnimatableState> make(std::string name, std::vector<AnimatableStateStruct>* stateValues){
		return std::make_shared<AnimatableState>(name.c_str(), stateValues);
	}
	
private:
	std::vector<AnimatableStateStruct>* states;
};



struct AnimatableStateValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::STATE; }
	AnimatableStateStruct* value;
	std::vector<AnimatableStateStruct>* values;
};

