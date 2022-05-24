#include <pch.h>

#include "Playback.h"

#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Machine/Machine.h"
#include "Environnement/Environnement.h"

namespace PlaybackManager {

	std::vector<std::shared_ptr<Manoeuvre>> activeManoeuvres;

	void push(std::shared_ptr<Manoeuvre> manoeuvre){
		activeManoeuvres.push_back(manoeuvre);
	}

	void pop(std::shared_ptr<Manoeuvre> manoeuvre){
		for(int i = 0; i < activeManoeuvres.size(); i++){
			if(activeManoeuvres[i] == manoeuvre){
				activeManoeuvres.erase(activeManoeuvres.begin() + i);
				break;
			}
		}
	}

	std::vector<std::shared_ptr<Manoeuvre>>& getActiveManoeuvres(){
		return activeManoeuvres;
	}

	bool isAnyManoeuvreActive(){
		return !activeManoeuvres.empty();
	}

	void stopAllManoeuvres() {
		for(int i = activeManoeuvres.size() - 1; i >= 0; i--) activeManoeuvres[i]->stop();
	}

	long long getTime_microseconds(){
		return Environnement::getTime_nanoseconds() / 1000;
	}

	void incrementPlaybackPosition() {
		long long time_micros = Environnement::getTime_nanoseconds() / 1000;
		for (auto& manoeuvre : activeManoeuvres) {
			if (manoeuvre->isPlaying()) manoeuvre->incrementPlaybackPosition(time_micros);
		}
	}

	void updateActiveManoeuvreState() {
		//needs to call onPlaybackEnd in machines
		std::vector<std::shared_ptr<Manoeuvre>> finishedManoeuvres;
		for(auto& manoeuvre : activeManoeuvres){
			manoeuvre->updatePlaybackStatus();
			if(manoeuvre->isFinished()) {
				finishedManoeuvres.push_back(manoeuvre);
			}
		}
		for(auto& manoeuvre : finishedManoeuvres) pop(manoeuvre);
	}

}
