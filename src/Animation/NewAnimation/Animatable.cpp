#include <pch.h>

#include "Animatable.h"
#include "Animation.h"

#include "PositionAnimatable.h"
#include "StateAnimatable.h"

namespace AnimationSystem{


std::shared_ptr<Animatable> Animatable::createInstanceFromAbstractSerializable(Serializable& abstract){
	
	std::string typeString;
	abstract.deserializeAttribute("Type", typeString);
	Legato::Option* typeOption = getAnimatableType(typeString);
	
	if(typeOption == nullptr) return nullptr;
	AnimatableType animatableType = AnimatableType(typeOption->getInt());
		
	switch(animatableType){
		case AnimatableType::COMPOSITE:	return CompositeAnimatable::createInstance();
		case AnimatableType::BOOLEAN:	return nullptr;
		case AnimatableType::INTEGER: 	return nullptr;
		case AnimatableType::REAL:		return nullptr;
		case AnimatableType::STATE:		return StateAnimatable::createInstance();
		case AnimatableType::POSITION:	return PositionAnimatable::createInstance();
		case AnimatableType::VELOCITY: 	return nullptr;
	}
}
	
std::shared_ptr<Animation> Animatable::makeAnimation(AnimationType type){
	
	std::shared_ptr<Animation> newAnimation;
	
	if(isCompositeAnimatable()){
		
		auto compositeAnimatable = downcasted_shared_from_this<CompositeAnimatable>();
		
		auto newCompositeAnimation = CompositeAnimation::createInstance();
		newCompositeAnimation->setType(type);
		
		for(auto childAnimatable : compositeAnimatable->getChildAnimatables()){
			auto childAnimation = childAnimatable->makeAnimation(type);
			newCompositeAnimation->addChildAnimation(childAnimation);
		}
		
		newAnimation = newCompositeAnimation;
		
	}else{
		switch(type){
			case AnimationType::TARGET:
				newAnimation = TargetAnimation::createInstance();
				break;
			case AnimationType::SEQUENCE:
				newAnimation = SequenceAnimation::createInstance();
				break;
			case AnimationType::STOP:
				newAnimation = StopAnimation::createInstance();
				break;
		}
	}
	
	newAnimation->animatable = downcasted_shared_from_this<Animatable>();
	
	return newAnimation;
}

bool Animatable::onSerialization(){
	bool success = true;
	success &= serializeAttribute("Type", getAnimatableType(getType())->getSaveString());
	success &= serializeAttribute("SaveString", saveString);
	success &= serializeAttribute("UniqueID", uniqueID);
	return success;
}
bool Animatable::onDeserialization(){
	bool success = true;
	success &= deserializeAttribute("SaveString", saveString);
	success &= deserializeAttribute("UniqueID", uniqueID);
	return success;
}


bool CompositeAnimatable::onSerialization() {
	bool success = Animatable::onSerialization();
	success &= childAnimatables->serializeIntoParent(this);
	return success;
}
bool CompositeAnimatable::onDeserialization() {
	bool success = Animatable::onDeserialization();
	
	auto loadedChildAnimatables = Legato::ListComponent<Animatable>::createInstance();
	loadedChildAnimatables->setSaveString("ChildAnimatables");
	loadedChildAnimatables->setEntrySaveString("Animatable");
	loadedChildAnimatables->setEntryConstructor([](Serializable& abstract) -> std::shared_ptr<Animatable>{
		return Animatable::createInstanceFromAbstractSerializable(abstract);
	});
	success &= loadedChildAnimatables->deserializeFromParent(this);
	
	for(auto loadedChildAnimatable : loadedChildAnimatables->getEntries()){
		addChildAnimatable(loadedChildAnimatable);
	}
	
	return success;
}


};
