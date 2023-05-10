#include <pch.h>

#include "Manoeuvre.h"
#include "Animatable.h"
#include "Animation.h"

namespace AnimationSystem{


	void Manoeuvre::onConstruction(){
		Component::onConstruction();
		animations = Legato::ListComponent<Animation>::createInstance();
		animations->setSaveString("Animations");
		animations->setEntrySaveString("Animation");
		animations->setEntryConstructor([this](Serializable& abstract) -> std::shared_ptr<Animation> {
			auto newAnimation = Animation::createInstanceFromAbstractSerializable(abstract);
			newAnimation->setAnimatableRegistry(animatableRegistry);
			return newAnimation;
		});
		setName("New Manoeuvre");
		descriptionParameter = Legato::StringParameter::createInstance("", "Manoeuvre description", "Description");
	}

	void Manoeuvre::onCopyFrom(std::shared_ptr<Prototype> source){
		Component::onCopyFrom(source);
	}

	bool Manoeuvre::onSerialization(){
		bool success = Component::onSerialization();
		success &= descriptionParameter->serializeIntoParent(this);
		success &= animations->serializeIntoParent(this);
		return success;
	}

	bool Manoeuvre::onDeserialization(){
		bool success = Component::onDeserialization();
		success &= descriptionParameter->deserializeFromParent(this);
		success &= animations->deserializeFromParent(this);
		return true;
	}

	void Manoeuvre::addAnimation(std::shared_ptr<Animation> animation){
		animations->addEntry(animation);
	}

	void Manoeuvre::removeAnimation(std::shared_ptr<Animation> animation){
		animations->removeEntry(animation);
	}

};
