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
		success &= animatables->serializeIntoParent(this);
		return success;
	}

	bool AnimatableOwner::onDeserialization(){
		bool success = true;
		
		auto loadedAnimatables = Legato::ListComponent<Animatable>::createInstance();
		loadedAnimatables->setSaveString("Animatables");
		loadedAnimatables->setEntrySaveString("Animatable");
		loadedAnimatables->setEntryConstructor([](Serializable& abstract) -> std::shared_ptr<Animatable>{
			return Animatable::createInstanceFromAbstractSerializable(abstract);
		});
		success &= loadedAnimatables->deserializeFromParent(this);
		
		
		std::function<bool(std::vector<std::shared_ptr<Animatable>>&,std::vector<std::shared_ptr<Animatable>>&)> copySaveData;
		copySaveData = [&](std::vector<std::shared_ptr<Animatable>>& destinationAnimatables,
						  std::vector<std::shared_ptr<Animatable>>& sourceAnimatables) -> bool {
			
			bool b_allFound = true;
			for(auto destinationAnimatable : destinationAnimatables){
				bool b_foundInSource = false;
				for(auto sourceAnimatable : sourceAnimatables){
					if(destinationAnimatable->getSaveString() == sourceAnimatable->getSaveString() && destinationAnimatable->getType() == sourceAnimatable->getType()){
						b_foundInSource = true;
						destinationAnimatable->uniqueID = sourceAnimatable->uniqueID;
						if(destinationAnimatable->isCompositeAnimatable()){
							auto destinationComposite = destinationAnimatable->downcasted_shared_from_this<CompositeAnimatable>();
							auto sourceComposite = sourceAnimatable->downcasted_shared_from_this<CompositeAnimatable>();
							copySaveData(destinationComposite->getChildAnimatables(), sourceComposite->getChildAnimatables());
						}
						break;
					}
				}
				if(!b_foundInSource){
					b_allFound = false;
					Logger::warn("Animatable {} was not found in save file", destinationAnimatable->getName());
				}
			}
			
			return b_allFound;
		};
		
		success &= copySaveData(getAnimatables(), loadedAnimatables->getEntries());
		
		//this function should be called after the owning class has all its animatables constructed
		//all it does is relink the unique ids to the animatables
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
