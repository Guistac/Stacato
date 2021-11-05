#include <pch.h>

#include "Manoeuvre.h"
#include "Motion/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"

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







void Manoeuvre::prime() {
	for (auto& track : tracks) track->prime();
	b_isPriming = true;
	std::thread manoeuvrePrimingStateWatcher([this]() {
		Logger::warn("Manoeuvre {} Started priming", name);
		bool allParametersPrimed = false;
		while (!allParametersPrimed) {
			allParametersPrimed = true;
			for (auto& track : tracks) {
				if (!track->isPrimed()) {
					allParametersPrimed = false;
					break;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		b_isPriming = false;
		Logger::warn("Manoeuvre {} finished priming", name);
	});
	manoeuvrePrimingStateWatcher.detach();
}

bool Manoeuvre::isPriming() {
	return b_isPriming;
}

bool Manoeuvre::isPrimed() {
	if (tracks.empty()) return false;
	for (auto& track : tracks) {
		if (!track->isPrimed()) return false;
	}
	return true;
}

float Manoeuvre::getPrimingProgress() {
	float lowestPrimingProgress = 1.0;
	for (auto& track : tracks) {
		float progress = track->getPrimingProgress();
		if (progress < lowestPrimingProgress) lowestPrimingProgress = progress;
	}
	return lowestPrimingProgress;
}



void Manoeuvre::startPlayback() {
	if (isPrimed()) {
		playbackStartTime_seconds = EtherCatFieldbus::getCycleProgramTime_seconds();
		for (auto& track : tracks) {
			track->parameter->actualParameterTrack = track;
			track->playbackStartTime_seconds = playbackStartTime_seconds;
		}
		b_isPlaying = true;
	}
}

void Manoeuvre::stopPlayback() {
	if (b_isPlaying) {
		b_isPlaying = false;
		for (auto& track : tracks) track->parameter->actualParameterTrack = nullptr;
	}
}

bool Manoeuvre::isPlaying() {
	return b_isPlaying;
}

double Manoeuvre::getPlaybackTime_seconds() {
	return EtherCatFieldbus::getCycleProgramTime_seconds() - playbackStartTime_seconds;
}

float Manoeuvre::getPlaybackProgress() {
	float timeInManoeuvre = getPlaybackTime_seconds();
	float manoeuvreLength = getLength_seconds();
	float progress = timeInManoeuvre / manoeuvreLength;
	if (progress < 0.0) return 0.0;
	else if (progress > 1.0) return 1.0;
	return progress;
}











void Manoeuvre::setType(ManoeuvreType::Type t) {
	type = t;
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