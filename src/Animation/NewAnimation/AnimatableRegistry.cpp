#include <pch.h>
#include "AnimatableRegistry.h"

#include "AnimatableOwner.h"

namespace AnimationSystem{

	void AnimatableRegistry::registerAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		if(animatableOwner->animatableOwnerUniqueID == -1){
			animatableOwner->animatableOwnerUniqueID = uniqueIDCounter;
			uniqueIDCounter++;
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
