#include <pch.h>
#include "AnimatableOwner.h"

#include "Animatable.h"

namespace AnimationSystem{

	void AnimatableOwner::addAnimatable(std::shared_ptr<Animatable> animatable){
		//animatable->owner = shared_from_this();
		animatables.push_back(animatable);
	}

	void AnimatableOwner::removeAnimatable(std::shared_ptr<Animatable> animatable){
		for(int i = (int)animatables.size(); i >= 0; i--){
			if(animatables[i] == animatable){
				//animatable->owner = nullptr;
				animatables.erase(animatables.begin() + i);
				break;
			}
		}
	}

}
