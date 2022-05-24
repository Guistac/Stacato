#include <pch.h>

#include "Machine.h"

#include "Machine/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

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

void Machine::addParameter(std::shared_ptr<MachineParameter> parameter) {
	auto thisMachine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	parameter->setMachine(thisMachine);
	if(parameter->isGroup()){
		auto parameterGroup = parameter->castToGroup();
		for(auto& childParameter : parameterGroup->getChildren()){
			childParameter->setMachine(thisMachine);
		}
	}
	parameters.push_back(parameter);
}

void Machine::startParameterPlayback(std::shared_ptr<ParameterTrack> track) {
	auto trackParameter = track->getParameter();
	for (auto& parameter : parameters) {
		if (trackParameter == parameter) {
			//stop playback of this parameter if it is already playing
			if(parameter->hasActiveParameterTrack()) parameter->stopParameterPlayback();
			parameter->activeParameterTrack = track;
			onParameterPlaybackStart(parameter);
		}
	}
}

void Machine::interruptParameterPlayback(std::shared_ptr<MachineParameter> playingParameter) {
	for (auto& parameter : parameters) {
		if (playingParameter == parameter) {
			parameter->activeParameterTrack = nullptr;
			onParameterPlaybackInterrupt(parameter);
		}
	}
}

void Machine::endParameterPlayback(std::shared_ptr<MachineParameter> playingParameter){
	for (auto& parameter : parameters) {
		if (playingParameter == parameter) {
			parameter->activeParameterTrack = nullptr;
			onParameterPlaybackEnd(parameter);
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
