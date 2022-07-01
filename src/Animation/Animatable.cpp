#include <pch.h>

#include "Machine/Machine.h"
#include "Animation/Animation.h"
#include "AnimationValue.h"

#include "Animatables/AnimatableState.h"
#include "Animatables/AnimatablePosition.h"


//——————————————————— Type Casting ————————————————————

std::shared_ptr<AnimatableState> Animatable::toState(){ return std::dynamic_pointer_cast<AnimatableState>(shared_from_this()); }
std::shared_ptr<AnimatablePosition> Animatable::toPosition(){ return std::dynamic_pointer_cast<AnimatablePosition>(shared_from_this()); }



//——————————————————— Animations ————————————————————

std::shared_ptr<Animation> Animatable::makeAnimation(ManoeuvreType manoeuvreType){
	auto track = Animation::create(shared_from_this(), manoeuvreType);
	track->subscribeToMachineParameter();
	return track;
}

void Animatable::subscribeAnimation(std::shared_ptr<Animation> animation){
	animations.push_back(animation);
}

void Animatable::unsubscribeAnimation(std::shared_ptr<Animation> animation){
	for(int i = 0; i < animations.size(); i++){
		if(animations[i] == animation) {
			animations.erase(animations.begin() + i);
			break;
		}
	}
}


//—————————————— MOTION COMMANDS ——————————————————


void Animatable::rapidToValue(std::shared_ptr<AnimationValue> animationValue){
	stopAnimation();
	onRapidToValue(animationValue);
}

void Animatable::stopAnimation(){
	if(isInRapid()) cancelRapid();
	else if(hasAnimation()) getAnimation()->stop();
}


//——————————————————— Value Retrieval ————————————————————

std::shared_ptr<AnimationValue> Animatable::getAnimationValue(){
	return currentAnimation->getValueAtPlaybackTime();
}


//——————————————————— Number ————————————————————

bool AnimatableNumber::isReal(){
	switch(getType()){
		case AnimatableType::INTEGER:
		case AnimatableType::BOOLEAN:
		case AnimatableType::STATE: return false;
		default: return true;
	}
}

void AnimatableNumber::setUnit(Unit u){
	unit = u;
	for(auto animation : getAnimations()) animation->setUnit(u);
}




//——————————————————— Composite ————————————————————

void AnimatableComposite::setChildren(std::vector<std::shared_ptr<Animatable>> children_){
	children = children_;
	for(auto& childAnimatable : children) childAnimatable->setParentComposite(toComposite());
}
