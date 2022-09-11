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
	int integer = std::round(animation->getCurves().front()->getPointAtTime(time_seconds).position);
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

std::vector<std::string> curveNames_animatableState = {
	"State"
};

std::vector<std::string>& AnimatableState::getCurveNames(){
	return curveNames_animatableState;
}

void AnimatableState::fillControlPointDefaults(std::shared_ptr<Motion::ControlPoint> controlpoint){
	controlpoint->inAcceleration = 0.0;
	controlpoint->outAcceleration = 0.0;
	controlpoint->velocity = 0.0;
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
	//const std::lock_guard<std::mutex> lock(mutex);
	auto rapidTarget = AnimationValue::makeState();
	rapidTarget->value = targetValue->value;
	return rapidTarget;
}




void AnimatableState::onRapidToValue(std::shared_ptr<AnimationValue> animationValue){
	const std::lock_guard<std::mutex> lock(mutex);
	auto newTargetValue = AnimationValue::makeState();
	newTargetValue->value = animationValue->toState()->value;
	targetValue = newTargetValue;
	b_inRapid = true;
}

void AnimatableState::onPlaybackStart(std::shared_ptr<Animation> animation){
	const std::lock_guard<std::mutex> lock(mutex);
	currentAnimation = animation;
}
void AnimatableState::onPlaybackPause(){
	const std::lock_guard<std::mutex> lock(mutex);
	currentAnimation = nullptr;
}
void AnimatableState::onPlaybackStop(){
	const std::lock_guard<std::mutex> lock(mutex);
	currentAnimation = nullptr;
}
void AnimatableState::onPlaybackEnd(){}
void AnimatableState::stopMovement(){
	targetValue->value = stoppedState;
}





std::shared_ptr<AnimationValue> AnimatableState::getActualValue(){
	const std::lock_guard<std::mutex> lock(mutex);
	std::shared_ptr<AnimatableStateValue> copy = AnimationValue::makeState();
	copy->value = actualValue->value;
	return copy;
}

void AnimatableState::updateActualValue(std::shared_ptr<AnimationValue> newActualValue){
	const std::lock_guard<std::mutex> lock(mutex);
	actualValue = newActualValue->toState();
}

std::shared_ptr<AnimationValue> AnimatableState::getTargetValue() {
	const std::lock_guard<std::mutex> lock(mutex);
	std::shared_ptr<AnimatableStateValue> copy = AnimationValue::makeState();
	copy->value = targetValue->value;
	return copy;
}

void AnimatableState::updateTargetValue(double time_seconds, double deltaTime_seconds){
	const std::lock_guard<std::mutex> lock(mutex);
	if(isHalted()) {
		targetValue->value = stoppedState;
		stopMovement();
		stopAnimation();
	}else{
		if(hasAnimation() && getAnimation()->isPlaying()){
			b_inRapid = false;
			targetValue->value = getAnimationValue()->toState()->value;
		}
		if(b_inRapid && targetValue->value == actualValue->value) b_inRapid = false;
	}
}

void AnimatableState::followActualValue(double time_seconds, double deltaTime_seconds){
	const std::lock_guard<std::mutex> lock(mutex);
	targetValue->value = actualValue->value;
}

