#include <pch.h>

#include "Animations.h"
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
		
		//relink animation with animatable
		//only top level animations should do this
		//composites handle their own relinking
		if(isTopLevelAnimation()){
			animatable = animatableRegistry->getAnimatable(animatableUniqueID);
			if(animatable == nullptr){
				success = false;
				Logger::critical("Could not relink animatable");
			}else{
				onSetAnimatable();
			}
		}
		
		return success;
	}

	void CompositeAnimation::onConstruction(){
		Animation::onConstruction();
		childAnimations = Legato::ListComponent<Animation>::createInstance();
		childAnimations->setSaveString("ChildAnimations");
		childAnimations->setEntrySaveString("Animation");
	}

	bool CompositeAnimation::onSerialization(){
		bool success = Animation::onSerialization();
		success &= childAnimations->serializeIntoParent(this);
		return success;
	}

	bool CompositeAnimation::onDeserialization(){
		bool success = Animation::onDeserialization();
		
		childAnimations->setEntryConstructor([this](Serializable& abstract) -> std::shared_ptr<Animation> {
			auto newAnimation = Animation::createInstanceFromAbstractSerializable(abstract);
			newAnimation->setAnimatableRegistry(animatableRegistry);
			
			int animatableUID;
			abstract.deserializeAttribute("AnimatableUniqueID", animatableUID);
			std::shared_ptr<Animatable> animatable;
			
			auto parentCompositeAnimation = downcasted_shared_from_this<CompositeAnimation>();
			newAnimation->setParentComposite(parentCompositeAnimation);
			auto parentCompositeAnimatable = parentCompositeAnimation->animatable->downcasted_shared_from_this<CompositeAnimatable>();
			for(auto childAnimatable : parentCompositeAnimatable->getChildAnimatables()){
				if(childAnimatable->getUniqueID() == animatableUID){
					newAnimation->setAnimatable(childAnimatable);
					newAnimation->onSetAnimatable();
					break;
				}
			}
			
			return newAnimation;
		});
		
		success &= childAnimations->deserializeFromParent(this);
		
		
		
		/*
		//relink all composite child animations with their animatable
		//only top level animations are able to do this
		if(isTopLevelAnimation()){
			
			std::function<bool(std::shared_ptr<CompositeAnimation>)> relinkCompositeAnimatable;
			relinkCompositeAnimatable = [&](std::shared_ptr<CompositeAnimation> compositeAnimation) -> bool {
				
				if(compositeAnimation->animatable == nullptr) {
					Logger::warn("Could not relink animatable of composite animation with ID {}, animatable was nullptr", compositeAnimation->getAnimatableUniqueID());
					return false;
				}
				
				bool b_allAnimatablesFound = true;
				for(auto unlinkedChildAnimation : compositeAnimation->getChildAnimations()){
					bool b_animatableFound = false;
					auto compositeAnimatable = compositeAnimation->animatable->downcasted_shared_from_this<CompositeAnimatable>();
					for(auto childAnimatable : compositeAnimatable->getChildAnimatables()){
						if(unlinkedChildAnimation->getAnimatableUniqueID() == childAnimatable->getUniqueID()){
							unlinkedChildAnimation->setAnimatable(childAnimatable);
							unlinkedChildAnimation->onSetAnimatable();
							b_animatableFound = true;
							//if the child animation is a composite, recursively relink all the lower level children with their animatable
							if(unlinkedChildAnimation->isCompositeAnimation()){
								auto childCompositeAnimation = unlinkedChildAnimation->downcasted_shared_from_this<CompositeAnimation>();
								b_allAnimatablesFound &= relinkCompositeAnimatable(childCompositeAnimation);
							}
							break;
						}
					}
					if(!b_animatableFound) {
						Logger::warn("Could not relink animatable of animation with ID {}, animatable not found", unlinkedChildAnimation->getAnimatableUniqueID());
						b_allAnimatablesFound = false;
					}
				}
				return b_allAnimatablesFound;
			};
			
			auto thisCompositeAnimation = downcasted_shared_from_this<CompositeAnimation>();
			success &= relinkCompositeAnimatable(thisCompositeAnimation);
			
		}
		*/
		
		return success;
	}


};
