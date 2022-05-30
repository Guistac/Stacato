#include <pch.h>

#include "Manoeuvre.h"
#include "Animation.h"

#include "Machine/Machine.h"
#include "Animation/Animatable.h"
#include "Environnement/Environnement.h"

#include <tinyxml2.h>


bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	
	using namespace tinyxml2;
	
	name->save(manoeuvreXML);
	description->save(manoeuvreXML);
	type->save(manoeuvreXML);
	
	for (auto& animation : animations) {
		//tracks which have a group parent are listed in the manoeuvres track vector
		//but they don't get saved in the main manoeuvre
		//instead they are saved by their parent parameter track
		if (animation->hasParentComposite()) continue;
		XMLElement* animationXML = manoeuvreXML->InsertNewChildElement("Animation");
		animation->save(animationXML);
	}
	 
	return true;
}

std::shared_ptr<Manoeuvre> Manoeuvre::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	auto manoeuvre = Manoeuvre::make(ManoeuvreType::KEY);
	
	manoeuvre->name->load(xml);
	manoeuvre->description->load(xml);
	manoeuvre->type->load(xml);
	
	XMLElement* animationXML = xml->FirstChildElement("Animation");
	while (animationXML != nullptr) {
		auto newAnimation = Animation::load(animationXML);
		if(newAnimation == nullptr) return nullptr;
		newAnimation->setManoeuvre(manoeuvre);
		manoeuvre->animations.push_back(newAnimation);
		animationXML = animationXML->NextSiblingElement("Animation");
	}
	
	manoeuvre->validateAllParameterTracks();
	
	return manoeuvre;
}




bool Animation::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Machine", animatable->getMachine()->getName());
	xml->SetAttribute("MachineUniqueID", animatable->getMachine()->getUniqueID());
	xml->SetAttribute("Parameter", animatable->getName());
	xml->SetAttribute("IsComposite", isComposite());
	if(!isComposite()) xml->SetAttribute("Type", Enumerator::getSaveString(getType()));
	return onSave(xml);
}


std::shared_ptr<Animation> Animation::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	const char* machineName;
	int machineUniqueID;
	const char* animatableName;
	
	//we need to find the exact parameter object linked with the parameter track
	//to do this we match the machine name, machine unique id and then parse the machines parameters and match the parameter name
	
	if(xml->QueryStringAttribute("Machine", &machineName) != XML_SUCCESS) {
		Logger::warn("could not load machine name");
		return nullptr;
	}
	
	if(xml->QueryIntAttribute("MachineUniqueID", &machineUniqueID) != XML_SUCCESS){
		Logger::warn("Could not load machine unique id");
		return nullptr;
	}
	
	if(xml->QueryStringAttribute("Parameter", &animatableName) != XML_SUCCESS){
		Logger::warn("Could not load parameter name");
		return nullptr;
	}
	
	
	std::shared_ptr<Machine> machine = nullptr;
	for (auto& m : Environnement::getMachines()) {
		if (strcmp(machineName, m->getName()) == 0 && machineUniqueID == m->getUniqueID()) {
			machine = m;
			break;
		}
	}
	if (machine == nullptr) {
		Logger::warn("Could not identify machine {} with unique ID {}", machineName, machineUniqueID);
		return nullptr;
	}

	std::shared_ptr<Animatable> animatable = nullptr;
	for (auto& a : machine->animatables) {
		if (strcmp(animatableName, a->getName()) == 0) {
			animatable = a;
			break;
		}
	}
	if (animatable == nullptr) {
		Logger::warn("Could not find animatable named {} in machine {}", animatableName, machine->getName());
		return nullptr;
	}
	
	//once we have the parameter object, we can create and load the parameter track object:
	auto loadedTrack = load(xml, animatable);
	if(loadedTrack == nullptr) {
		Logger::warn("Failed to load parameter track {} of machine {}", animatableName, machine->getName());
		return nullptr;
	}
	loadedTrack->subscribeToMachineParameter();
	
	return loadedTrack;
}



std::shared_ptr<Animation> Animation::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	using namespace tinyxml2;
	
	bool isGroup = false;
	ManoeuvreType manoeuvreType;
	
	xml->QueryBoolAttribute("IsGroup", &isGroup);
	
	if(!isGroup){
		const char* typeSaveString;
		if(xml->QueryStringAttribute("Type", &typeSaveString) != XML_SUCCESS){
			Logger::warn("could not load type save string");
			return nullptr;
		}
		if(!Enumerator::getEnumeratorFromSaveString(typeSaveString, manoeuvreType)){
			Logger::warn("could not identify track type save string");
			return nullptr;
		}
	}
	
	std::shared_ptr<Animation> animation = nullptr;
	if(isGroup) AnimationComposite::load(xml, animatable->toComposite());
	else{
		switch(manoeuvreType){
			case ManoeuvreType::KEY: animation = AnimationKey::load(xml, animatable); break;
			case ManoeuvreType::TARGET: animation = TargetAnimation::load(xml, animatable); break;
			case ManoeuvreType::SEQUENCE: animation = SequenceAnimation::load(xml, animatable); break;
		}
	}
	if(animation == nullptr){
		Logger::warn("Could not load parameter track");
		return nullptr;
	}
	return animation;
}



















bool AnimationKey::onSave(tinyxml2::XMLElement* xml){
	target->save(xml);
	return true;
}

std::shared_ptr<AnimationKey> AnimationKey::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	auto animationKey = std::make_shared<AnimationKey>(animatable);
	if(!animationKey->target->load(xml)){
		Logger::warn("Could not load Target parameter of Parameter track {}", animatable->getName());
		return nullptr;
	}
	return animationKey;
}




















