#include <pch.h>

#include "Manoeuvre.h"
#include "Machine/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Plot/Plot.h"

#include <tinyxml2.h>


void Manoeuvre::setType(ManoeuvreType type_){
	std::vector<std::shared_ptr<ParameterTrack>> oldTracks = tracks;
	
	//change all parameter tracks to new types
	for(int i = 0; i < tracks.size(); i++){
		std::shared_ptr<ParameterTrack>& track = tracks[i];
		track = ParameterTrack::create(track->getParameter(), type_);
	}
}


//Copy Constructor
/*
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
 */

void Manoeuvre::addTrack(std::shared_ptr<MachineParameter>& parameter) {
	auto newTrack = ParameterTrack::create(parameter, getType());
	tracks.push_back(newTrack);
}

void Manoeuvre::removeTrack(std::shared_ptr<MachineParameter>& parameter) {
	for (int i = 0; i < tracks.size(); i++) {
		if (tracks[i]->getParameter() == parameter) {
			tracks.erase(tracks.begin() + i);
			break;
		}
	}
}

void Manoeuvre::removeTrack(int removedIndex) {
	if(removedIndex < 0 || removedIndex >= tracks.size()) return;
	tracks.erase(tracks.begin() + removedIndex);
}

bool Manoeuvre::hasTrack(std::shared_ptr<MachineParameter>& parameter) {
	for (auto& track : tracks) {
		if (track->getParameter() == parameter) return true;
	}
	return false;
}


void Manoeuvre::moveTrack(int oldIndex, int newIndex){
	if(oldIndex < 0) return;
	if(oldIndex >= tracks.size()) return;
	if(newIndex < 0) return;
	if(newIndex >= tracks.size()) return;
	if(oldIndex == newIndex) return;
		
	auto tmp = tracks[oldIndex];
	tracks.erase(tracks.begin() + oldIndex);
	tracks.insert(tracks.begin() + newIndex, tmp);
}


float Manoeuvre::getPlaybackProgress() {
	/*
	float timeInManoeuvre = playbackPosition_seconds;
	float manoeuvreLength = getLength_seconds();
	float progress = timeInManoeuvre / manoeuvreLength;
	if (progress < 0.0) return 0.0;
	else if (progress > 1.0) return 1.0;
	return progress;
	 */
	return 0.0;
}





double Manoeuvre::getLength_seconds() {
	/*
	double longestTrack = 0.0;
	for (auto& track : tracks) {
		double trackTime = track->getLength_seconds();
		if (trackTime > longestTrack) {
			longestTrack = trackTime;
		}
	}
	return longestTrack;
	 */
	return 0.0;
}
