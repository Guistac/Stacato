#include <pch.h>

#include "Manoeuvre.h"
#include "Motion/ParameterTrack.h"

Manoeuvre::Manoeuvre(const Manoeuvre& original) {
	for (auto& track : original.tracks) {
		std::shared_ptr<ParameterTrack> copiedTrack = std::make_shared<ParameterTrack>(*track);
		tracks.push_back(copiedTrack);
	}
}

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

double Manoeuvre::getLength_seconds() {
	double longestTrack = 0.0;
	for (auto& track : tracks) {
		double trackTime = track->getLength_seconds();
		if (trackTime > longestTrack) {
			longestTrack = trackTime;
		}
	}
	return longestTrack;
}