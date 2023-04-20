#include <pch.h>
#include "AnimatableOwner.h"

#include "Animatable.h"

namespace AnimationSystem{

	void AnimatableOwner::onConstruction(){
		animatables = Legato::ListComponent<Animatable>::createInstance();
	}

	bool AnimatableOwner::onSerialization(){
		bool success = true;
		return success;
	}

	bool AnimatableOwner::onDeserialization(){
		bool success = true;
		return success;
	}

	void AnimatableOwner::addAnimatable(std::shared_ptr<Animatable> animatable){
		animatable->owner = downcasted_shared_from_this<AnimatableOwner>();
		animatables->addEntry(animatable);
	}

	void AnimatableOwner::removeAnimatable(std::shared_ptr<Animatable> animatable){
		animatable->owner = nullptr;
		animatables->removeEntry(animatable);
	}

}
