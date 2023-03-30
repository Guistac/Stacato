#include <pch.h>

#include "Animatable.h"
#include "Animation.h"

namespace AnimationSystem{

	
std::shared_ptr<Animation> Animatable::makeAnimation(AnimationType type){
	
	std::shared_ptr<Animation> newAnimation;
	
	if(isCompositeAnimatable()){
		
		auto newCompositeAnimation = std::make_shared<CompositeAnimation>();
		
		for(auto childAnimatable : childAnimatables){
			auto childAnimation = childAnimatable->makeAnimation(type);
			newCompositeAnimation->addChildAnimation(childAnimation);
		}
		
		newAnimation = newCompositeAnimation;
		
	}else{
		switch(type){
			case AnimationType::TARGET:
				newAnimation = std::make_shared<TargetAnimation>();
				break;
			case AnimationType::SEQUENCE:
				newAnimation = std::make_shared<SequenceAnimation>();
				break;
			case AnimationType::STOP:
				newAnimation = std::make_shared<StopAnimation>();
				break;
		}
	}
	
	newAnimation->animatable = shared_from_this();
	
	return newAnimation;
}


};
