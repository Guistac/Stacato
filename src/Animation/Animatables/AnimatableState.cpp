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

bool AnimatableState::generateTargetAnimation(std::shared_ptr<TargetAnimation> animation){
	
	
	auto target = parameterValueToAnimationValue(animation->target)->toState();
	TargetAnimation::Constraint constraint = animation->getConstraintType();
	if(constraint != TargetAnimation::Constraint::TIME) return false;
	double timeOffset = animation->timeConstraint->value;
	
	auto startPoint = std::make_shared<Motion::ControlPoint>();
	startPoint->time = 0.0;
	startPoint->position = actualValue->value->integerEquivalent;
	startPoint->velocity = 0.0;
	startPoint->outAcceleration = 0.0;
	startPoint->b_valid = true;
	
	auto endPoint = std::make_shared<Motion::ControlPoint>();
	endPoint->time = timeOffset;
	endPoint->position = target->value->integerEquivalent;
	endPoint->velocity = 0.0;
	endPoint->inAcceleration = 0.0;
	endPoint->b_valid = true;
	
	std::shared_ptr<Motion::StepInterpolation> interpolation = Motion::StepInterpolation::getInterpolation(startPoint, endPoint);
	if(!interpolation->b_valid) return false;
	
	auto& curve = animation->getCurves().front();
	auto& points = curve->getPoints();
	auto& interpolations = curve->getInterpolations();
	
	points.clear();
	points.push_back(interpolation->inPoint);
	points.push_back(interpolation->outPoint);
	
	interpolations.clear();
	interpolation->updateDisplayCurvePoints();
	interpolations.push_back(interpolation);
	
	curve->b_valid = true;
	
	return true;
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
	mutex.lock();
	auto newTargetValue = AnimationValue::makeState();
	newTargetValue->value = animationValue->toState()->value;
	targetValue = newTargetValue;
	b_inRapid = true;
	mutex.unlock();
}

void AnimatableState::onPlaybackStart(std::shared_ptr<Animation> animation){
	mutex.lock();
	currentAnimation = animation;
	mutex.unlock();
}
void AnimatableState::onPlaybackPause(){
	mutex.lock();
	currentAnimation = nullptr;
    stopMovement();
	mutex.unlock();
}
void AnimatableState::onPlaybackStop(){
	mutex.lock();
	currentAnimation = nullptr;
    stopMovement();
	mutex.unlock();
}
void AnimatableState::onPlaybackEnd(){}
void AnimatableState::stopMovement(){
	targetValue->value = stoppedState;
}





std::shared_ptr<AnimationValue> AnimatableState::getActualValue(){
	auto ret = AnimationValue::makeState();
	mutex.lock();
	ret->value = actualValue->value;
	mutex.unlock();
	return ret;
}

void AnimatableState::updateActualValue(std::shared_ptr<AnimationValue> newActualValue){
	mutex.lock();
	actualValue = newActualValue->toState();
	mutex.unlock();
}

std::shared_ptr<AnimationValue> AnimatableState::getTargetValue() {
	auto ret = AnimationValue::makeState();
	mutex.lock();
	ret->value = targetValue->value;
	mutex.unlock();
	return ret;
}

void AnimatableState::updateTargetValue(double time_seconds, double deltaTime_seconds){
	mutex.lock();
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
	mutex.unlock();
}

void AnimatableState::followActualValue(double time_seconds, double deltaTime_seconds){
	mutex.lock();
	targetValue->value = actualValue->value;
	mutex.unlock();
}

bool AnimatableState::isMoving(){
	mutex.lock();
	bool b_moving = actualValue->value != targetValue->value;
	mutex.unlock();
	return b_moving;
}
