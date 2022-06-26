#include <pch.h>
#include "AnimatablePosition.h"

#include "Project/Editor/Parameter.h"
#include "Animation/Animation.h"
#include "Motion/MotionTypes.h"

std::vector<InterpolationType>& AnimatablePosition::getCompatibleInterpolationTypes(){
	static std::vector<InterpolationType> compatibleInterpolationTypes = { InterpolationType::TRAPEZOIDAL };
	return compatibleInterpolationTypes;
}

int AnimatablePosition::getCurveCount(){
	return 1;
}

std::shared_ptr<Parameter> AnimatablePosition::makeParameter(){
	return NumberParameter<double>::make(0.0, "DefaultName");
}

void AnimatablePosition::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value){
	std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->overwrite(value->toPosition()->position);
}

void AnimatablePosition::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	std::dynamic_pointer_cast<NumberParameter<double>>(to)->overwrite(std::dynamic_pointer_cast<NumberParameter<double>>(from)->value);
}

std::shared_ptr<AnimationValue> AnimatablePosition::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter){
	auto output = AnimationValue::makePosition();
	output->position = std::dynamic_pointer_cast<NumberParameter<double>>(parameter)->value;
	return output;
}

bool AnimatablePosition::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2){
	return value1->toPosition()->position == value2->toPosition()->position;
}

std::shared_ptr<AnimationValue> AnimatablePosition::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds){
	auto output = AnimationValue::makePosition();
	Motion::Point point = animation->getCurves().front().getPointAtTime(time_seconds);
	output->position = point.position;
	output->velocity = point.velocity;
	output->acceleration = point.acceleration;
	return output;
}

std::vector<double> AnimatablePosition::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	return std::vector<double>{value->toPosition()->position};
}

