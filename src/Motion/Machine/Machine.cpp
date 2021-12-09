#include <pch.h>

#include "Machine.h"

#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

void Machine::addAnimatableParameter(std::shared_ptr<AnimatableParameter> parameter) {
	parameter->machine = std::dynamic_pointer_cast<Machine>(shared_from_this());
	if (!parameter->childParameters.empty()) {
		for (auto& childParameter : parameter->childParameters) {
			childParameter->machine = parameter->machine;
			childParameter->parentParameter = parameter;
			animatableParameters.push_back(childParameter);
		}
	}
	animatableParameters.push_back(parameter);
}

void Machine::startParameterPlayback(std::shared_ptr<ParameterTrack> track) {
	for (auto& p : animatableParameters) {
		if (track->parameter == p) {
			track->parameter->actualParameterTrack = track;
			onParameterPlaybackStart(track->parameter);
		}
	}
}

void Machine::stopParameterPlayback(std::shared_ptr<AnimatableParameter> parameter) {
	for (auto& p : animatableParameters) {
		if (parameter == p) {
			parameter->actualParameterTrack = nullptr;
			onParameterPlaybackStop(parameter);
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
