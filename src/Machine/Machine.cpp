#include <pch.h>

#include "Machine.h"

#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Environnement/Environnement.h"

bool Machine::isReady(){
	if(Environnement::isSimulating()) return isSimulationReady() && Environnement::isRunning();
	else return isHardwareReady();
}

void Machine::enable(){
	if(Environnement::isSimulating() && isSimulationReady()){
		onEnableSimulation();
		b_enabled = true;
	}else {
		enableHardware();
	}
}

void Machine::disable(){
	if(Environnement::isSimulating()){
		b_enabled = false;
		onDisableSimulation();
	}else {
		disableHardware();
	}
}

bool Machine::isEnabled(){
	return b_enabled;
}

bool Machine::isSimulating(){
	return Environnement::isSimulating();
}

void Machine::addAnimatable(std::shared_ptr<Animatable> animatable) {
	auto thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	animatable->setMachine(thisMachine);
	if(animatable->isComposite()){
		auto animatableComposite = animatable->toComposite();
		for(auto& childAnimatable : animatableComposite->getChildren()){
			childAnimatable->setMachine(thisMachine);
		}
	}
	animatables.push_back(animatable);
}

void Machine::startAnimationPlayback(std::shared_ptr<Animation> animation) {
	auto animationAnimatable = animation->getAnimatable();
	for (auto& animatable : animatables) {
		if (animationAnimatable == animatable) {
			//stop playback of this parameter if it is already playing
			if(animatable->hasAnimation()) animatable->stopAnimationPlayback();
			animatable->setAnimation(animation);
			onAnimationPlaybackStart(animatable);
		}
	}
}

void Machine::interruptAnimationPlayback(std::shared_ptr<Animatable> playingAnimatable) {
	for (auto& animatable : animatables) {
		if (playingAnimatable == animatable) {
			animatable->setAnimation(nullptr);
			onAnimationPlaybackInterrupt(animatable);
		}
	}
}

void Machine::endAnimationPlayback(std::shared_ptr<Animatable> playingAnimatable){
	for (auto& animatable : animatables) {
		if (playingAnimatable == animatable) {
			animatable->setAnimation(nullptr);
			onAnimationPlaybackEnd(animatable);
		}
	}
}

bool Machine::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	xml->SetAttribute("ShortName", shortName);
	saveMachine(xml);
	return true;
}

bool Machine::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	const char* sName;
	if(xml->QueryStringAttribute("ShortName", &sName) != tinyxml2::XML_SUCCESS) return Logger::warn("Could not find machine short name");
	sprintf(shortName, "%s", sName);
	loadMachine(xml);
	return true;
}
