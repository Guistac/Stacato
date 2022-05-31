#include <pch.h>

#include "Animation.h"

#include "Machine/Machine.h"
#include "Animation/Animatable.h"
#include "Motion/Curve/Curve.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Environnement/Environnement.h"
#include "Animation/Manoeuvre.h"
#include "Plot/Plot.h"

#include "Project/Editor/Parameter.h"


std::shared_ptr<Animation> Animation::create(std::shared_ptr<Animatable> animatable, ManoeuvreType manoeuvreType){
	if(animatable->isComposite()) return std::make_shared<AnimationComposite>(animatable->toComposite(), manoeuvreType);
	switch(manoeuvreType){
		case ManoeuvreType::KEY:
			return std::make_shared<AnimationKey>(animatable);
		case ManoeuvreType::TARGET:
			return std::make_shared<TargetAnimation>(animatable);
		case ManoeuvreType::SEQUENCE:
			return std::make_shared<SequenceAnimation>(animatable);
	}
}

std::shared_ptr<Animation> Animation::copy(){
	std::shared_ptr<Animation> copy;
	if(isComposite()) copy = toComposite()->copy();
	else{
		switch(getType()){
			case ManoeuvreType::KEY:		copy = toKey()->copy(); break;
			case ManoeuvreType::TARGET: 	copy = toTarget()->copy(); break;
			case ManoeuvreType::SEQUENCE: 	copy = toSequence()->copy(); break;
		}
	}
	copy->subscribeToMachineParameter();
	return copy;
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







void Animation::subscribeToMachineParameter(){
	animatable->subscribeAnimation(shared_from_this());
}

void Animation::unsubscribeFromMachineParameter(){
	animatable->unsubscribeTrack(shared_from_this());
}

void Animation::validate(){
	validationErrorString = "";
	b_valid = getAnimatable()->getMachine()->validateAnimation(shared_from_this());
	if(hasManoeuvre()) manoeuvre->updateTrackSummary();
}


void Animation::updatePlaybackStatus(){
	if(playbackPosition_seconds >= duration_seconds){
		auto animatable = getAnimatable();
		if(animatable->hasAnimation()) animatable->getMachine()->endAnimationPlayback(animatable);
	}
}


std::shared_ptr<AnimationValue> Animation::getValueAtPlaybackTime(){
	return animatable->getValueAtAnimationTime(shared_from_this(), playbackPosition_seconds);
}


bool Animation::isMachineEnabled(){ return animatable->getMachine()->isEnabled(); }


bool Animation::isInRapid(){
	return getRapidProgress() < 1.0;
}

float Animation::getRapidProgress(){
	return animatable->getMachine()->getAnimatableRapidProgress(animatable);
}


bool Animation::isPlaying(){
	return getAnimatable()->getAnimation() == shared_from_this();
}

void Animation::startPlayback(){
	if(!isReadyToStartPlayback()) return;
	animatable->getMachine()->startAnimationPlayback(shared_from_this());
}

void Animation::interruptPlayback(){
	if(isPlaying()) animatable->getMachine()->interruptAnimationPlayback(animatable);
	animatable->getMachine()->cancelAnimatableRapid(animatable);
}

void Animation::stopPlayback(){
	if(isPlaying()) animatable->getMachine()->endAnimationPlayback(animatable);
	animatable->getMachine()->cancelAnimatableRapid(animatable);
	setPlaybackPosition(0.0);
	if(hasManoeuvre()) getManoeuvre()->onTrackPlaybackStop();
}
