#include <pch.h>

#include "Playback.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"

namespace Playback {

	std::vector<std::shared_ptr<Manoeuvre>> primingManoeuvres;
	std::vector<std::shared_ptr<Manoeuvre>> playingManoeuvres;


	void startPriming(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!isPriming(manoeuvre)) {
			for (auto& track : manoeuvre->tracks) track->prime();
			primingManoeuvres.push_back(manoeuvre);
		}
	}
	
	void stopPriming(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (isPriming(manoeuvre)) {
			for (auto& track : manoeuvre->tracks) track->cancelPriming();
			for (int i = 0; i < primingManoeuvres.size(); i++) {
				if (primingManoeuvres[i] == manoeuvre) {
					primingManoeuvres.erase(primingManoeuvres.begin() + i);
					break;
				}
			}
		}
	}
	
	bool isPriming(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		for (auto& m : primingManoeuvres) {
			if (m == manoeuvre) return true;
		}
		return false;
	}

	bool isPrimed(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (manoeuvre->tracks.empty()) return false;
		for (auto& track : manoeuvre->tracks) {
			if (!track->isPrimed()) return false;
		}
		return true;
	}

	float getPrimingProgress(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		float lowestPrimingProgress = 1.0;
		for (auto& track : manoeuvre->tracks) {
			float progress = track->getPrimingProgress();
			if (progress < lowestPrimingProgress) lowestPrimingProgress = progress;
		}
		return lowestPrimingProgress;
	}






	void startPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!isPlaying(manoeuvre)) {
			double time = EtherCatFieldbus::getCycleProgramTime_seconds();
			manoeuvre->playbackStartTime_seconds = time;
			for (auto& track : manoeuvre->tracks) {
				track->parameter->actualParameterTrack = track;
				track->playbackStartTime_seconds = time;
			}
			playingManoeuvres.push_back(manoeuvre);
		}
	}
	
	void stopPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (isPlaying(manoeuvre)) {
			for (auto& track : manoeuvre->tracks) track->parameter->actualParameterTrack = nullptr;
			for (int i = 0; i < playingManoeuvres.size(); i++) {
				if (playingManoeuvres[i] == manoeuvre) {
					playingManoeuvres.erase(playingManoeuvres.begin() + i);
					break;
				}
			}
		}
	}

	bool isPlaying(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		for (auto& m : playingManoeuvres) {
			if (m == manoeuvre) return true;
		}
		return false;
	}

	void stopAllManoeuvres() {
		for (auto& manoeuvre : playingManoeuvres) {
			for (auto& track : manoeuvre->tracks) track->parameter->actualParameterTrack = nullptr;
		}
		playingManoeuvres.clear();
	}


	void updateActiveManoeuvreState() {
		for (int i = primingManoeuvres.size() - 1; i >= 0; i--) {
			if (!isPriming(primingManoeuvres[i])) {
				Logger::warn("Manoeuvre {} Finished Priming", primingManoeuvres[i]->name);
				primingManoeuvres.erase(primingManoeuvres.begin() + i);
			}
		}
		for (int i = playingManoeuvres.size() - 1; i >= 0; i--) {
			if (playingManoeuvres[i]->getPlaybackProgress() >= 1.0) {
				for (auto& track : playingManoeuvres[i]->tracks) track->parameter->actualParameterTrack = nullptr;
				playingManoeuvres.erase(playingManoeuvres.begin() + i);
			}
		}
	}

}