#include <pch.h>

#include "Machine/Machine.h"
#include "Animation/Animation.h"
#include "AnimationValue.h"

#include "Animatables/AnimatableState.h"
#include "Animatables/AnimatablePosition.h"
#include "Animatables/AnimatableBoolean.h"


//——————————————————— Type Casting ————————————————————

std::shared_ptr<AnimatableState> Animatable::toState(){ return std::dynamic_pointer_cast<AnimatableState>(shared_from_this()); }
std::shared_ptr<AnimatablePosition> Animatable::toPosition(){ return std::dynamic_pointer_cast<AnimatablePosition>(shared_from_this()); }
std::shared_ptr<AnimatableBoolean> Animatable::toBoolean(){ return std::dynamic_pointer_cast<AnimatableBoolean>(shared_from_this()); }




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



void Animatable::startAnimation(std::shared_ptr<Animation> animation){
	assert(animation->getAnimatable() == shared_from_this());
	//stop playback of current animation if there is one playing
	interruptAnimation();
	//set animation as current
	currentAnimation = animation;
	//notify machine of playback start
	getMachine()->onAnimationPlaybackStart(shared_from_this());
}

void Animatable::interruptAnimation(){
	if(!hasAnimation()) return;
	currentAnimation = nullptr;
	getMachine()->onAnimationPlaybackInterrupt(shared_from_this());
}

void Animatable::endAnimation(){
	if(!hasAnimation()) return;
	getMachine()->onAnimationPlaybackEnd(shared_from_this());
}

void Animatable::rapidToValue(std::shared_ptr<AnimationValue> animationValue){
	stop();
	onRapidToValue(animationValue);
}

void Animatable::stop(){
	if(isInRapid()) cancelRapid();
	else if(hasAnimation()) interruptAnimation();
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
