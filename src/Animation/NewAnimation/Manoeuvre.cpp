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
		animations->setEntryConstructor([](Serializable& abstract) -> std::shared_ptr<Animation> {
			bool b_isComposite;
			abstract.deserializeAttribute("IsComposite", b_isComposite);
			std::string typeString;
			if(abstract.deserializeAttribute("Type", typeString)){
				return Animation::createInstanceFromTypeString(b_isComposite, typeString);
			}
			else return nullptr;
		});
	}
	void Manoeuvre::onCopyFrom(std::shared_ptr<PrototypeBase> source){
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
