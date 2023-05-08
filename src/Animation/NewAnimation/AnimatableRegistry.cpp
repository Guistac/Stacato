#include <pch.h>
#include "AnimatableRegistry.h"

#include "AnimatableOwner.h"

namespace AnimationSystem{



	void AnimatableRegistry::registerAnimatable(std::shared_ptr<Animatable> animatable){
		if(animatable->uniqueID == -1){
			animatable->uniqueID = uniqueIDCounter;
			uniqueIDCounter++;
		}
		if(animatable->isCompositeAnimatable()){
			auto composite = animatable->downcasted_shared_from_this<CompositeAnimatable>();
			for(auto childAnimatable : composite->getChildAnimatables()){
				registerAnimatable(childAnimatable);
			}
		}
	}

	void AnimatableRegistry::unregisterAnimatable(std::shared_ptr<Animatable> animatable){
		animatable->uniqueID = -1;
		if(animatable->isCompositeAnimatable()){
			auto composite = animatable->downcasted_shared_from_this<CompositeAnimatable>();
			for(auto childAnimatable : composite->getChildAnimatables()){
				unregisterAnimatable(childAnimatable);
			}
		}
	}


	void AnimatableRegistry::registerAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		for(auto animatable : animatableOwner->getAnimatables()){
			registerAnimatable(animatable);
		}
		animatableOwners.push_back(animatableOwner);
	}

	void AnimatableRegistry::unregisterAnimatableOwner(std::shared_ptr<AnimatableOwner> animatableOwner){
		for(int i = (int)animatableOwners.size() - 1; i >= 0; i--){
			for(auto animatable : animatableOwner->getAnimatables()){
				unregisterAnimatable(animatable);
			}
			if(animatableOwners[i] == animatableOwner){
				animatableOwners.erase(animatableOwners.begin() + i);
				break;
			}
		}
	}

	std::shared_ptr<Animatable> AnimatableRegistry::getAnimatable(int uniqueID){
		for(auto owner : animatableOwners){
			for(auto animatable : owner->getAnimatables()){
				if(animatable->getUniqueID() == uniqueID) return animatable;
			}
		}
		return nullptr;
	}

}
