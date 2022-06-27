#pragma once

#include "Animation/Animatable.h"
#include "Animation/AnimationValue.h"

class AnimatableBoolean : public Animatable{
public:
	
	//construction
	AnimatableBoolean(const char* name) : Animatable(name) {};
	static std::shared_ptr<AnimatableBoolean> make(std::string name){ return std::make_shared<AnimatableBoolean>(name.c_str()); }
	virtual AnimatableType getType() override { return AnimatableType::BOOLEAN; }

	virtual std::vector<InterpolationType>& getCompatibleInterpolationTypes() override;
	virtual int getCurveCount() override;
	virtual std::shared_ptr<Parameter> makeParameter() override;
	virtual void setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value) override;
	virtual void copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to) override;
	virtual std::shared_ptr<AnimationValue> parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter) override;
	virtual bool isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2) override;
	virtual std::shared_ptr<AnimationValue> getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds) override;
	virtual std::vector<double> getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value) override;
	
};



struct AnimatableBooleanValue : public AnimationValue{
	virtual AnimatableType getType(){ return AnimatableType::BOOLEAN; }
	bool value;
};

