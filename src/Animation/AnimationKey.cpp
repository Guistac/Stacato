#include "Animation.h"

#include "Machine/Machine.h"

std::shared_ptr<AnimationKey> AnimationKey::copy(){
	auto copy = std::make_shared<AnimationKey>(getAnimatable());
	getAnimatable()->copyParameterValue(target, copy->target);
	return copy;
}

bool AnimationKey::onSave(tinyxml2::XMLElement* xml){
	target->save(xml);
	return true;
}

std::shared_ptr<AnimationKey> AnimationKey::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	auto animationKey = std::make_shared<AnimationKey>(animatable);
	if(!animationKey->target->load(xml)){
		Logger::warn("Could not load Target parameter of Parameter track {}", animatable->getName());
		return nullptr;
	}
	return animationKey;
}

bool AnimationKey::isAtPlaybackPosition(){
	auto animatable = getAnimatable();
	auto actualValue = animatable->getActualValue();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	return animatable->isParameterValueEqual(actualValue, targetValue);
}

void AnimationKey::rapidToPlaybackPosition(){
	auto animatable = getAnimatable();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	animatable->getMachine()->rapidAnimatableToValue(animatable, targetValue);
}

