#include <pch.h>
#include "AnimatableBoolean.h"

#include "Project/Editor/Parameter.h"
#include "Animation/Animation.h"
#include "Motion/MotionTypes.h"

std::vector<InterpolationType>& AnimatableBoolean::getCompatibleInterpolationTypes(){
	static std::vector<InterpolationType> compatibleInterpolationTypes = { InterpolationType::STEP };
	return compatibleInterpolationTypes;
}

int AnimatableBoolean::getCurveCount(){
	return 1;
}

std::shared_ptr<Parameter> AnimatableBoolean::makeParameter(){
	return std::make_shared<BooleanParameter>(false, "DefaultName", "DefaultSaveString");
}

void AnimatableBoolean::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value){
	std::dynamic_pointer_cast<BooleanParameter>(parameter)->overwrite(value->toBoolean()->value);
}

void AnimatableBoolean::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	std::dynamic_pointer_cast<BooleanParameter>(to)->overwrite(std::dynamic_pointer_cast<BooleanParameter>(from)->value);
}

std::shared_ptr<AnimationValue> AnimatableBoolean::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter){
	auto output = AnimationValue::makeBoolean();
	output->value = std::dynamic_pointer_cast<BooleanParameter>(parameter)->value;
	return output;
}

bool AnimatableBoolean::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2){
	return value1->toBoolean()->value == value2->toBoolean()->value;
}

std::shared_ptr<AnimationValue> AnimatableBoolean::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds){
	auto output = AnimationValue::makeBoolean();
	output->value = animation->getCurves().front().getPointAtTime(time_seconds).position >= 1.0;
	return output;
}

std::vector<double> AnimatableBoolean::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	return std::vector<double>{value->toBoolean()->value ? 1.0 : 0.0};
}

