#include <pch.h>
#include "AnimatableState.h"

#include "Project/Editor/Parameter.h"
#include "Animation/Animation.h"
#include "Motion/MotionTypes.h"

#include "Machine/Machine.h"

std::vector<InterpolationType>& AnimatableState::getCompatibleInterpolationTypes(){
	static std::vector<InterpolationType> compatibleInterpolationTypes = { InterpolationType::STEP };
	return compatibleInterpolationTypes;
}

int AnimatableState::getCurveCount(){
	return 1;
}

std::shared_ptr<Parameter> AnimatableState::makeParameter(){
	return std::make_shared<StateParameter>(stoppedState, *selectableStates, "DefaultName", "DefaultSaveString");
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
	int integer = std::round(animation->getCurves().front().getPointAtTime(time_seconds).position);
	for(int i = 0; i < selectableStates->size(); i++){
		auto state = selectableStates->at(i);
		if(state->integerEquivalent == integer){
			output->value = selectableStates->at(i);
			break;
		}
	}
	return output;
}

std::vector<double> AnimatableState::getCurvePositionsFromAnimationValue(std::shared_ptr<AnimationValue> value){
	return std::vector<double>{(double)value->toState()->value->integerEquivalent};
}

bool AnimatableState::isControlledManuallyOrByAnimation(){
	return hasAnimation() || isInRapid();
}




bool AnimatableState::isReadyToMove(){
	auto machine = getMachine();
	return machine->isEnabled() && !machine->isHalted();
}
bool AnimatableState::isReadyToStartPlaybackFromValue(std::shared_ptr<AnimationValue> animationValue){
	return animationValue->toState()->value == actualValue->value;
}
bool AnimatableState::isInRapid(){ return b_inRapid; }
float AnimatableState::getRapidProgress(){ return .5f; }
void AnimatableState::cancelRapid(){
	targetValue->value = stoppedState;
	b_inRapid = false;
}

std::shared_ptr<AnimationValue> AnimatableState::getRapidTarget(){
	auto rapidTarget = AnimationValue::makeState();
	rapidTarget->value = targetValue->value;
	return rapidTarget;
}




void AnimatableState::onRapidToValue(std::shared_ptr<AnimationValue> animationValue){
	auto newTargetValue = AnimationValue::makeState();
	newTargetValue->value = animationValue->toState()->value;
	targetValue = newTargetValue;
	b_inRapid = true;
}

void AnimatableState::onPlaybackStart(){}
void AnimatableState::onPlaybackPause(){}
void AnimatableState::onPlaybackStop(){}
void AnimatableState::onPlaybackEnd(){}
void AnimatableState::stopMovement(){
	targetValue->value = stoppedState;
}







void AnimatableState::updateTargetValue(double time_seconds, double deltaTime_seconds){
	if(hasAnimation() && getAnimation()->isPlaying()){
		b_inRapid = false;
		targetValue->value = getAnimationValue()->toState()->value;
	}
	if(b_inRapid && targetValue->value == actualValue->value) b_inRapid = false;
}

void AnimatableState::followActualValue(double time_seconds, double deltaTime_seconds){
	targetValue->value = actualValue->value;
}

