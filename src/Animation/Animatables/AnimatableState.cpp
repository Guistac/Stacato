#include <pch.h>
#include "AnimatableState.h"

#include "Project/Editor/Parameter.h"
#include "Animation/Animation.h"
#include "Motion/MotionTypes.h"

std::vector<InterpolationType>& AnimatableState::getCompatibleInterpolationTypes(){
	static std::vector<InterpolationType> compatibleInterpolationTypes = { InterpolationType::STEP };
	return compatibleInterpolationTypes;
}

int AnimatableState::getCurveCount(){
	return 1;
}

std::shared_ptr<Parameter> AnimatableState::makeParameter(){
	return std::make_shared<StateParameter>(&states->front(), states, "DefaultName", "DefaultSaveString");
}

void AnimatableState::setParameterValueFromAnimationValue(std::shared_ptr<Parameter> parameter, std::shared_ptr<AnimationValue> value){
	std::static_pointer_cast<StateParameter>(parameter)->overwrite(value->toState()->value);
}

void AnimatableState::copyParameterValue(std::shared_ptr<Parameter> from, std::shared_ptr<Parameter> to){
	std::static_pointer_cast<StateParameter>(to)->overwrite(std::static_pointer_cast<StateParameter>(from)->value);
}

std::shared_ptr<AnimationValue> AnimatableState::parameterValueToAnimationValue(std::shared_ptr<Parameter> parameter){
	auto output = AnimationValue::makeState();
	output->value = std::static_pointer_cast<StateParameter>(parameter)->value;
	return output;
}

bool AnimatableState::isParameterValueEqual(std::shared_ptr<AnimationValue> value1, std::shared_ptr<AnimationValue> value2){
	return value1->toState()->value == value2->toState()->value;
}

std::shared_ptr<AnimationValue> AnimatableState::getValueAtAnimationTime(std::shared_ptr<Animation> animation, double time_seconds){
	auto output = AnimationValue::makeState();
	output->values = states;
	int integer = std::round(animation->getCurves().front().getPointAtTime(time_seconds).position);
	for(int i = 0; i < output->values->size(); i++){
		if(output->values->at(i).integerEquivalent == i){
			output->value = &output->values->at(i);
			break;
		}
	}
	return output;
}

std::vector<double> AnimatableState::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	return std::vector<double>{(double)value->toState()->value->integerEquivalent};
}

