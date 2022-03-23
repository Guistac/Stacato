#include <pch.h>

#include "Manoeuvre.h"
#include "Motion/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Plot/Plot.h"

#include <tinyxml2.h>

//Copy Constructor
Manoeuvre::Manoeuvre(const Manoeuvre& original) {
	sprintf(name, "%s*", original.name);
	sprintf(description, "(copy) %s", original.description);
	type = original.type;
	parentPlot = original.parentPlot;
	for (auto& track : original.tracks) {
		if (track->hasParentParameterTrack()) continue;
		std::shared_ptr<ParameterTrack> copiedTrack = std::make_shared<ParameterTrack>(*track);
		if (copiedTrack->hasChildParameterTracks()) {
			for (auto& childParameterTrack : copiedTrack->childParameterTracks) {
				childParameterTrack->parentParameterTrack = copiedTrack;
				tracks.push_back(childParameterTrack);
			}
		}
		tracks.push_back(copiedTrack);
	}
}

void Manoeuvre::addTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	std::shared_ptr<Manoeuvre> thisManoeuvre = shared_from_this();
	std::shared_ptr<ParameterTrack> newTrack = std::make_shared<ParameterTrack>(parameter, thisManoeuvre);
	tracks.push_back(newTrack);
	if (newTrack->hasChildParameterTracks()) {
		for (auto& childParameterTrack : newTrack->childParameterTracks) {
			childParameterTrack->parentParameterTrack = newTrack;
			tracks.push_back(childParameterTrack);
		}
	}
	switch (type) {
		case Type::KEY_POSITION:
				newTrack->setSequenceType(SequenceType::Type::CONSTANT);
				break;
			case Type::TIMED_MOVEMENT:
			case Type::MOVEMENT_SEQUENCE:
				newTrack->originIsPreviousTarget = true;
				newTrack->setSequenceType(SequenceType::Type::TIMED_MOVE);
	}
	parentPlot->refreshChainingDependencies();
	newTrack->refreshAfterParameterEdit();
}

void Manoeuvre::removeTrack(std::shared_ptr<AnimatableParameter>& parameter) {
	for (int i = 0; i < tracks.size(); i++) {
		if (tracks[i]->parameter == parameter) {
			std::shared_ptr<ParameterTrack> deletedTrack = tracks[i];
			tracks.erase(tracks.begin() + i);
			for (auto childParameterTrack : deletedTrack->childParameterTracks) {
				removeTrack(childParameterTrack->parameter);
			}
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











void Manoeuvre::setType(Manoeuvre::Type t) {
	type = t;
	switch (type) {
		case Manoeuvre::Type::KEY_POSITION:
			for (auto& track : tracks) {
				track->setSequenceType(SequenceType::Type::CONSTANT);
			}
			break;
		case Manoeuvre::Type::TIMED_MOVEMENT:
			for (auto& track : tracks) {
				track->originIsPreviousTarget = true;
				track->setSequenceType(SequenceType::Type::TIMED_MOVE);
			}
			break;
		case Manoeuvre::Type::MOVEMENT_SEQUENCE:
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
