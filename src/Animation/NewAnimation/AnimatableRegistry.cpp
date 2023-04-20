#include <pch.h>
#include "AnimatableRegistry.h"

#include "AnimatableOwner.h"
#include "Animatable.h"

namespace AnimationSystem{

	void AnimatableRegistry::registerAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		for(auto animatable : animatableOwner->getAnimatables()){
			if(animatable->uniqueID == -1){
				animatable->uniqueID = uniqueIDCounter;
				uniqueIDCounter++;
			}
		}
		animatableOwners.push_back(animatableOwner);
	}

	void AnimatableRegistry::unregisterAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		for(int i = (int)animatableOwners.size() - 1; i >= 0; i--){
			if(animatableOwners[i] == animatableOwner){
				animatableOwners.erase(animatableOwners.begin() + i);
				break;
			}
		}
	}

}
