#include <pch.h>

#include "Animation.h"
#include "Animatable.h"
#include "AnimatableRegistry.h"

namespace AnimationSystem{


	std::shared_ptr<Animation> Animation::createInstanceFromAbstractSerializable(Serializable& abstract){
		
		bool b_success = true;
		
		bool b_isComposite;
		b_success &= abstract.deserializeAttribute("IsComposite", b_isComposite);
		
		std::string typeString;
		b_success &= abstract.deserializeAttribute("Type", typeString);
		
		Legato::Option* typeOption = getAnimationType(typeString);
		if(typeOption == nullptr || !b_success) return nullptr;
		
		AnimationType animationType = AnimationType(typeOption->getInt());
		
		if(b_isComposite){
			auto compositeAnimation = CompositeAnimation::createInstance();
			compositeAnimation->setType(animationType);
			return compositeAnimation;
		}
		else{
			switch(animationType){
				case TARGET:	return TargetAnimation::createInstance();
				case SEQUENCE:	return SequenceAnimation::createInstance();
				case STOP:		return StopAnimation::createInstance();
			}
		}
		
	}



	bool Animation::onSerialization(){
		bool success = true;
		
		success &= serializeAttribute("Type", getAnimationType(getType())->getSaveString());
		success &= serializeAttribute("IsComposite", isCompositeAnimation());
		success &= serializeAttribute("AnimatableUniqueID", animatable->getUniqueID());
		
		return success;
	}

	bool Animation::onDeserialization(){
		bool success = true;
		success &= deserializeAttribute("AnimatableUniqueID", animatableUniqueID);
		//we can't find the animatable here, since composite children cannot find the animatable in the registry
		//maybe we should relink all animatables somewhere else...
		animatable = animatableRegistry->getAnimatable(animatableUniqueID);
		success &= animatable != nullptr;
		return success;
	}

	void CompositeAnimation::onConstruction(){
		Animation::onConstruction();
		childAnimations = Legato::ListComponent<Animation>::createInstance();
		childAnimations->setSaveString("ChildAnimations");
		childAnimations->setEntrySaveString("Animation");
		childAnimations->setEntryConstructor([this](Serializable& abstract) -> std::shared_ptr<Animation> {
			auto newAnimation = Animation::createInstanceFromAbstractSerializable(abstract);
			newAnimation->setAnimatableRegistry(animatableRegistry);
			return newAnimation;
		});
	}

	bool CompositeAnimation::onSerialization(){
		bool success = Animation::onSerialization();
		success &= childAnimations->serializeIntoParent(this);
		return success;
	}

	bool CompositeAnimation::onDeserialization(){
		bool success = Animation::onDeserialization();
		
		success &= childAnimations->deserializeFromParent(this);
		
		std::function<bool(std::shared_ptr<CompositeAnimation>)> relinkCompositeAnimatable;
		relinkCompositeAnimatable = [](std::shared_ptr<CompositeAnimation> compositeAnimation) -> bool {
			bool b_allAnimatablesFound = true;
			for(auto unlinkedChildAnimation : compositeAnimation->getChildAnimations()){
				bool b_animatableFound = false;
				auto compositeAnimatable = compositeAnimation->animatable->downcasted_shared_from_this<CompositeAnimatable>();
				for(auto childAnimatable : compositeAnimatable->getChildAnimatables()){
					if(unlinkedChildAnimation->getAnimatableUniqueID() == childAnimatable->getUniqueID()){
						unlinkedChildAnimation->setAnimatable(childAnimatable);
						b_animatableFound = true;
						break;
					}
				}
				if(!b_animatableFound) b_allAnimatablesFound = false;
			}
			return b_allAnimatablesFound;
		};
		
		if(animatable == nullptr) return false;
		
		auto thisCompositeAnimation = downcasted_shared_from_this<CompositeAnimation>();
		success &= relinkCompositeAnimatable(thisCompositeAnimation);
		
		return success;
	}


};
