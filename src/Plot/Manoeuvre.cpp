#include <pch.h>

#include "Manoeuvre.h"
#include "Motion/ParameterTrack.h"


void Manoeuvre::addTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	std::shared_ptr<ParameterTrack> parameterSequence = std::make_shared<ParameterTrack>(parameter);
	tracks.push_back(parameterSequence);
}

void Manoeuvre::removeTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	for (int i = 0; i < tracks.size(); i++) {
		if (tracks[i]->parameter == parameter) {
			tracks.erase(tracks.begin() + i);
			break;
		}
	}
}

bool Manoeuvre::hasTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	for (auto& p : tracks) {
		if (p->parameter == parameter) return true;
	}
	return false;
}


bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	manoeuvreXML->SetAttribute("Name", name);
	manoeuvreXML->SetAttribute("Description", description);
	return true;
}

bool Manoeuvre::load(tinyxml2::XMLElement* manoeuvreXML) {
	using namespace tinyxml2;
	const char* nameString;
	if (manoeuvreXML->QueryStringAttribute("Name", &nameString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre Name Attribute");
	strcpy(name, nameString);
	const char* descriptionString;
	if (manoeuvreXML->QueryStringAttribute("Description", &descriptionString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre Description Attribute");
	strcpy(description, descriptionString);
	return true;
}