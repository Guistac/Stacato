#include <pch.h>

#include "Animatable.h"
#include "Animation.h"

namespace AnimationSystem{

	
std::shared_ptr<Animation> Animatable::makeAnimation(AnimationType type){
	
	std::shared_ptr<Animation> newAnimation;
	
	if(isCompositeAnimatable()){
		
		auto compositeAnimatable = downcasted_shared_from_this<CompositeAnimatable>();
		
		auto newCompositeAnimation = CompositeAnimation::createInstance();
		newCompositeAnimation->setType(type);
		
		for(auto childAnimatable : compositeAnimatable->getChildAnimatables()){
			auto childAnimation = childAnimatable->makeAnimation(type);
			newCompositeAnimation->addChildAnimation(childAnimation);
		}
		
		newAnimation = newCompositeAnimation;
		
	}else{
		switch(type){
			case AnimationType::TARGET:
				newAnimation = TargetAnimation::createInstance();
				break;
			case AnimationType::SEQUENCE:
				newAnimation = SequenceAnimation::createInstance();
				break;
			case AnimationType::STOP:
				newAnimation = StopAnimation::createInstance();
				break;
		}
	}
	
	newAnimation->animatable = downcasted_shared_from_this<Animatable>();
	
	return newAnimation;
}


};
