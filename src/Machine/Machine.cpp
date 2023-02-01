#include <pch.h>

#include "Machine.h"

#include "Animation/Animatable.h"
#include "Animation/Animation.h"

#include "Environnement/Environnement.h"

#include "Motion/Safety/DeadMansSwitch.h"

bool Machine::isReady(){
	if(Environnement::isSimulating()) return isSimulationReady() && Environnement::isRunning();
	else return isHardwareReady();
}

void Machine::enable(){
	if(Environnement::isSimulating() && isSimulationReady()){
		onEnableSimulation();
		state = DeviceState::ENABLED;
		//b_enabled = true;
	}else {
		enableHardware();
	}
	for(auto animatable : animatables) animatable->stopAnimation();
}

void Machine::disable(){
	if(Environnement::isSimulating()){
		state = DeviceState::READY;
		//b_enabled = false;
		onDisableSimulation();
	}else {
		disableHardware();
	}
	for(auto animatable : animatables) animatable->stopAnimation();
}

bool Machine::isEnabled(){
	//return b_enabled;
	return state == DeviceState::ENABLED;
}

bool Machine::isSimulating(){
	return Environnement::isSimulating();
}

void Machine::addAnimatable(std::shared_ptr<Animatable> animatable){
	auto thisMachine = std::static_pointer_cast<Machine>(shared_from_this());
	animatable->setMachine(thisMachine);
	if(animatable->isComposite()){
		auto animatableComposite = animatable->toComposite();
		for(auto& childAnimatable : animatableComposite->getChildren()){
			childAnimatable->setMachine(thisMachine);
		}
	}
	animatables.push_back(animatable);
}

void Machine::removeAnimatable(std::shared_ptr<Animatable> removedAnimatable){
	//TODO: test
	for(int i = 0; i < animatables.size(); i++){
		if(animatables[i] == removedAnimatable){
			animatables.erase(animatables.begin() + i);
			break;
		}
	}
	removedAnimatable->deleteAllAnimations();
}


bool Machine::isMotionAllowed(){
	if(!deadMansSwitchPin->isConnected()) return true;
	for(auto deadMansSwitchPin : deadMansSwitchPin->getConnectedPins()){
		auto deadMansSwitch = deadMansSwitchPin->getSharedPointer<DeadMansSwitch>();
		if(!deadMansSwitch->isPressed()) return false;
	}
	return true;
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
	if(!loadMachine(xml)) return false;
	return true;
}
