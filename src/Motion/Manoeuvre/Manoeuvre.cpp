#include <pch.h>

#include "Manoeuvre.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Project/Plot.h"

#include <tinyxml2.h>

//Copy Constructor
Manoeuvre::Manoeuvre(const Manoeuvre& original) {
	sprintf(name, "%s*", original.name);
	sprintf(description, "(copy) %s", original.description);
	type = original.type;
	parentPlot = original.parentPlot;
	for (auto& track : original.tracks) {
		std::shared_ptr<ParameterTrack> copiedTrack = std::make_shared<ParameterTrack>(*track);
		tracks.push_back(copiedTrack);
	}
}

void Manoeuvre::addTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	std::shared_ptr<Manoeuvre> thisManoeuvre = shared_from_this();
	std::shared_ptr<ParameterTrack> newTrack = std::make_shared<ParameterTrack>(parameter, thisManoeuvre);
	tracks.push_back(newTrack);
	switch (type) {
	case ManoeuvreType::Type::KEY_POSITION:
			newTrack->setSequenceType(SequenceType::Type::NO_MOVE);
			break;
		case ManoeuvreType::Type::TIMED_MOVEMENT:
		case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
			newTrack->originIsPreviousTarget = true;
			newTrack->setSequenceType(SequenceType::Type::TIMED_MOVE);
	}
	parentPlot->refreshChainingDependencies();
	newTrack->refreshAfterParameterEdit();
}

void Manoeuvre::removeTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	for (int i = 0; i < tracks.size(); i++) {
		if (tracks[i]->parameter == parameter) {
			tracks.erase(tracks.begin() + i);
			break;
		}
	}
	parentPlot->refreshChainingDependencies();
}

bool Manoeuvre::hasTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	for (auto& p : tracks) {
		if (p->parameter == parameter) return true;
	}
	return false;
}

std::shared_ptr<ParameterTrack> Manoeuvre::getTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	for (auto& track : tracks) {
		if (track->parameter == parameter) return track;
	}
	return nullptr;
}

float Manoeuvre::getPlaybackProgress() {
	float timeInManoeuvre = playbackPosition_seconds;
	float manoeuvreLength = getLength_seconds();
	float progress = timeInManoeuvre / manoeuvreLength;
	if (progress < 0.0) return 0.0;
	else if (progress > 1.0) return 1.0;
	return progress;
}











void Manoeuvre::setType(ManoeuvreType::Type t) {
	type = t;
	switch (type) {
		case ManoeuvreType::Type::KEY_POSITION:
			for (auto& track : tracks) {
				track->setSequenceType(SequenceType::Type::NO_MOVE);
			}
			break;
		case ManoeuvreType::Type::TIMED_MOVEMENT:
			for (auto& track : tracks) {
				track->originIsPreviousTarget = true;
				track->setSequenceType(SequenceType::Type::TIMED_MOVE);
			}
			break;
		case ManoeuvreType::Type::MOVEMENT_SEQUENCE:
			for (auto& track : tracks) {
				track->setSequenceType(SequenceType::Type::TIMED_MOVE);
			}
			break;
	default:
		break;
	}
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





void Manoeuvre::refresh() {
	b_valid = true;
	for (auto& track : tracks) {
		if (!track->b_valid) { b_valid = false; break; }
	}
}







bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	using namespace tinyxml2;
	manoeuvreXML->SetAttribute("Name", name);
	manoeuvreXML->SetAttribute("Description", description);
	manoeuvreXML->SetAttribute("Type", getManoeuvreType(type)->saveName);
	for (auto& track : tracks) {
		XMLElement* trackXML = manoeuvreXML->InsertNewChildElement("Track");
		track->save(trackXML);
	}
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
	const char* manoeuvreTypeString;
	if(manoeuvreXML->QueryStringAttribute("Type", &manoeuvreTypeString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre type");
	if (getManoeuvreType(manoeuvreTypeString) == nullptr) return Logger::warn("Could not identify Manoeuvre type");
	type = getManoeuvreType(manoeuvreTypeString)->type;

	XMLElement* trackXML = manoeuvreXML->FirstChildElement("Track");
	while (trackXML != nullptr) {
		std::shared_ptr<ParameterTrack> track = std::make_shared<ParameterTrack>(shared_from_this());
		if (!track->load(trackXML)) return false;
		tracks.push_back(track);
		trackXML = trackXML->NextSiblingElement("Track");
	}

	return true;
}








std::vector<ManoeuvreType> manoeuvreTypes = {
	{ManoeuvreType::Type::KEY_POSITION,		"Key Position",		"KeyPosition",		"KEY"},
	{ManoeuvreType::Type::TIMED_MOVEMENT,	"Timed Movement",	"TimedMovement",	"TIM"},
	{ManoeuvreType::Type::MOVEMENT_SEQUENCE,"Movement Sequence","MovementSequence", "SEQ"}
};

std::vector<ManoeuvreType>& getManoeuvreTypes() {
	return manoeuvreTypes;
}
ManoeuvreType* getManoeuvreType(ManoeuvreType::Type t) {
	for (auto& manoeuvreType : manoeuvreTypes) {
		if (t == manoeuvreType.type) return &manoeuvreType;
	}
	return nullptr;
}
ManoeuvreType* getManoeuvreType(const char* saveName) {
	for (auto& manoeuvreType : manoeuvreTypes) {
		if (strcmp(manoeuvreType.saveName, saveName) == 0) return &manoeuvreType;
	}
	return nullptr;
}