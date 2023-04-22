#include <pch.h>
#include "AnimatableOwner.h"

#include "Animatable.h"

namespace AnimationSystem{

	

	void AnimatableOwner::onConstruction(){
		animatables = Legato::ListComponent<Animatable>::createInstance();
		animatables->setSaveString("Animatables");
		animatables->setEntrySaveString("Animatable");
	}

	bool AnimatableOwner::onSerialization(){
		bool success = true;
		success &= serializeAttribute("AnimatableOwnerUniqueID", animatableOwnerUniqueID);
		
		std::function<void(std::shared_ptr<Animatable>, Serializable*)> saveAnimatable = [&](std::shared_ptr<Animatable> animatable, Serializable* parent){
			Serializable animatableData;
			animatableData.setSaveString("Animatable");
			animatableData.serializeIntoParent(parent);
			
			animatableData.serializeAttribute("Type", getAnimatableType(animatable->getType())->getSaveString());
			animatableData.serializeAttribute("SaveString", animatable->getSaveString());
			
			if(animatable->isCompositeAnimatable()){
				Serializable childAnimatablesData;
				childAnimatablesData.setSaveString("ChildAnimatables");
				childAnimatablesData.serializeIntoParent(animatableData);
				auto compositeAnimatable = animatable->downcasted_shared_from_this<CompositeAnimatable>();
				for(auto childAnimatable : compositeAnimatable->getChildAnimatables()){
					saveAnimatable(childAnimatable, &childAnimatablesData);
				}
			}
		};
		
		Serializable animatablesData;
		animatablesData.setSaveString("Animatables");
		animatablesData.serializeIntoParent(this);
		for(auto animatable : animatables->getEntries()){
			saveAnimatable(animatable, &animatablesData);
		}
		
		return success;
	}

	bool AnimatableOwner::onDeserialization(){
		bool success = true;
		success &= deserializeAttribute("AnimatableOwnerUniqueID", animatableOwnerUniqueID);

		Serializable animatableData;
		animatableData.setSaveString("Animatables");
		animatableData.deserializeFromParent(this);
		
		
		
		std::vector<Serializable> abstractAnimatables;
		animatableData.deserializeList("Animatable", abstractAnimatables);

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
