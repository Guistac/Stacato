#include "Animation.h"

#include "Machine/Machine.h"

std::shared_ptr<AnimationKey> AnimationKey::copy(){
	auto copy = std::make_shared<AnimationKey>(getAnimatable());
	getAnimatable()->copyParameterValue(target, copy->target);
	return copy;
}

bool AnimationKey::isAtTarget(){
	auto animatable = getAnimatable();
	auto actualValue = animatable->getActualValue();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	return animatable->isParameterValueEqual(actualValue, targetValue);
}

void AnimationKey::rapidToTarget(){
	auto animatable = getAnimatable();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	animatable->getMachine()->rapidAnimatableToValue(animatable, targetValue);
}

