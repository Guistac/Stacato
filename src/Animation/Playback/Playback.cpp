#include <pch.h>

#include "Playback.h"

#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Animation/Manoeuvre.h"
#include "Animation/Animation.h"
#include "Machine/Machine.h"
#include "Environnement/Environnement.h"

namespace PlaybackManager {

	std::recursive_mutex mutex;

	std::vector<std::shared_ptr<Manoeuvre>> activeManoeuvres;

	void push(std::shared_ptr<Manoeuvre> manoeuvre){
		mutex.lock();
		activeManoeuvres.push_back(manoeuvre);
		mutex.unlock();
	}

	void pop(std::shared_ptr<Manoeuvre> manoeuvre){
		mutex.lock();
		for(int i = 0; i < activeManoeuvres.size(); i++){
			if(activeManoeuvres[i] == manoeuvre){
				activeManoeuvres.erase(activeManoeuvres.begin() + i);
				break;
			}
		}
		mutex.unlock();
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

	void update() {
		mutex.lock();
		long long time_micros = Environnement::getTime_nanoseconds() / 1000;
		for (auto& manoeuvre : activeManoeuvres) {
			if (manoeuvre->isPlaying()) manoeuvre->incrementPlaybackPosition(time_micros);
		}
		
		//needs to call onPlaybackEnd in machines
		std::vector<std::shared_ptr<Manoeuvre>> finishedManoeuvres;
		for(auto& manoeuvre : activeManoeuvres){
			manoeuvre->updatePlaybackStatus();
			if(manoeuvre->isFinished()) {
				finishedManoeuvres.push_back(manoeuvre);
			}
		}
		for(auto& manoeuvre : finishedManoeuvres) pop(manoeuvre);
		mutex.unlock();
	}

}
