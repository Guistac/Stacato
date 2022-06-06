#include "Animation.h"

#include "Machine/Machine.h"

AnimationKey::AnimationKey(std::shared_ptr<Animatable> animatable) : Animation(animatable){
	target = animatable->makeParameter();
	target->setName("Target");
	target->setSaveString("Target");
	if(animatable->isNumber()) setUnit(animatable->toNumber()->getUnit());
	target->setEditCallback([this](std::shared_ptr<Parameter> thisParameter){ this->validate(); });
}

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

void AnimationKey::setUnit(Unit unit) {
	if(getAnimatable()->isNumber()){
		target->toNumber()->setUnit(unit);
	}
}

void AnimationKey::getCurvePositionRange(double& min, double& max){
	double mi = DBL_MAX;
	double ma = DBL_MIN;
	auto animatable = getAnimatable();
	auto targetValue = animatable->parameterValueToAnimationValue(target);
	auto curvePositions = animatable->getCurvePositionsFromAnimationValue(targetValue);
	for(double value : curvePositions){
		mi = std::min(mi, value);
		ma = std::max(ma, value);
	}
	min = mi;
	max = ma;
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

