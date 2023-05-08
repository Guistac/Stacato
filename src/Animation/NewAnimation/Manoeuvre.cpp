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
	}
	void Manoeuvre::onCopyFrom(std::shared_ptr<Prototype> source){
		Component::onCopyFrom(source);
	}
	bool Manoeuvre::onSerialization(){
		bool success = Component::onSerialization();
		success &= animations->serializeIntoParent(this);
		return success;
	}
	bool Manoeuvre::onDeserialization(){
		bool success = Component::onDeserialization();
		success &= animations->deserializeFromParent(this);
		return success;
	}

	void Manoeuvre::addAnimation(std::shared_ptr<Animation> animation){
		animations->addEntry(animation);
	}

	void Manoeuvre::removeAnimation(std::shared_ptr<Animation> animation){
		animations->removeEntry(animation);
	}

};
