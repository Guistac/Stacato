#include <pch.h>

#include "Playback.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Motion/Machine/Machine.h"
#include "Environnement/Environnement.h"

namespace Playback {

	std::vector<std::shared_ptr<Manoeuvre>> rapidManoeuvres;
	std::vector<std::shared_ptr<Manoeuvre>> playingManoeuvres;

	void rapidToStart(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!isPlaying(manoeuvre)) {
			manoeuvre->playbackPosition_seconds = 0.0;
			for (auto& track : manoeuvre->tracks) {
				track->playbackPosition_seconds = 0.0;
				track->rapidToStart();
			}
			rapidManoeuvres.push_back(manoeuvre);
		}
	}

	void rapidToEnd(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!isPlaying(manoeuvre)) {
			manoeuvre->playbackPosition_seconds = 0.0;
			for (auto& track : manoeuvre->tracks) {
				track->playbackPosition_seconds = 0.0;
				track->rapidToEnd();
			}
			rapidManoeuvres.push_back(manoeuvre);
		}
	}

	void rapidToPlaybackPosition(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!isPlaying(manoeuvre) || isPaused(manoeuvre)) {
			for (auto& track : manoeuvre->tracks) {
				track->playbackPosition_seconds = manoeuvre->playbackPosition_seconds;
				track->rapidToPlaybackPosition();
			}
			rapidManoeuvres.push_back(manoeuvre);
		}
	}
	
	void stopRapid(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (isInRapid(manoeuvre)) {
			for (auto& track : manoeuvre->tracks) track->cancelRapid();
			for (int i = 0; i < rapidManoeuvres.size(); i++) {
				if (rapidManoeuvres[i] == manoeuvre) {
					rapidManoeuvres.erase(rapidManoeuvres.begin() + i);
					break;
				}
			}
		}
	}
	
	bool isInRapid(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		for (auto& m : rapidManoeuvres) {
			if (m == manoeuvre) return true;
		}
		return false;
	}

	float getRapidProgress(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		float lowestPrimingProgress = 1.0;
		for (auto& track : manoeuvre->tracks) {
			float progress = track->getRapidProgress();
			if (progress < lowestPrimingProgress) lowestPrimingProgress = progress;
		}
		return lowestPrimingProgress;
	}




	bool isPrimedToStart(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (manoeuvre->tracks.empty()) return false;
		for (auto& track : manoeuvre->tracks) {
			if (!track->isPrimedToStart()) return false;
		}
		return true;
	}

	bool isPrimedToEnd(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (manoeuvre->tracks.empty()) return false;
		for (auto& track : manoeuvre->tracks) {
			if (!track->isPrimedToEnd()) return false;
		}
		return true;
	}

	bool isPrimedToPlaybackPosition(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (manoeuvre->tracks.empty()) return false;
		for (auto& track : manoeuvre->tracks) {
			if (!track->isPrimedToPlaybackPosition()) return false;
		}
		return true;
	}






	void startPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!isPlaying(manoeuvre)) {
			double time = Environnement::getTime_seconds();
			manoeuvre->playbackStartTime_seconds = time - manoeuvre->playbackPosition_seconds;
			switch (manoeuvre->type) {
				case Manoeuvre::Type::KEY_POSITION:
					break;
				case Manoeuvre::Type::TIMED_MOVEMENT:
					for (auto& track : manoeuvre->tracks) {
						//track->parameter->machine->getTimedMovementTo(parameter, parameterValue, curves);
					}
					break;
				case Manoeuvre::Type::MOVEMENT_SEQUENCE:
					for (auto& track : manoeuvre->tracks) {
						track->playbackPosition_seconds = manoeuvre->playbackPosition_seconds;
						track->parameter->machine->startParameterPlayback(track);
					}
					break;
			}
			playingManoeuvres.push_back(manoeuvre);
		}
	}

	void pausePlayback(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (!manoeuvre->b_isPaused) {
			manoeuvre->b_isPaused = true;
			for (auto& track : manoeuvre->tracks) {
				//this stops the machine track but doesn't remove it from the playback list
				track->parameter->machine->interruptParameterPlayback(track->parameter);
			}
		}
	}

	void resumePlayback(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (manoeuvre->b_isPaused) {
			manoeuvre->b_isPaused = false;
			switch (manoeuvre->type) {
				case Manoeuvre::Type::KEY_POSITION:
					break;
				case Manoeuvre::Type::TIMED_MOVEMENT:
					for (auto& track : manoeuvre->tracks) {
						track->parameter->actualParameterTrack = track;
						//track->parameter->machine->getTimedMovementTo(parameter, parameterValue, curves);
					}
					break;
				case Manoeuvre::Type::MOVEMENT_SEQUENCE:
					if (isPrimedToPlaybackPosition(manoeuvre)) {
						double time = Environnement::getTime_seconds();
						manoeuvre->playbackStartTime_seconds = time - manoeuvre->playbackPosition_seconds;
						for (auto& track : manoeuvre->tracks) {
							track->playbackPosition_seconds = manoeuvre->playbackPosition_seconds;
							track->parameter->machine->startParameterPlayback(track);
						}
					}
					break;
			}
		}
	}
	
	void stopPlayback(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		if (isPlaying(manoeuvre)) {
			for (auto& track : manoeuvre->tracks) {
				track->parameter->machine->interruptParameterPlayback(track->parameter);
			}
			for (int i = 0; i < playingManoeuvres.size(); i++) {
				if (playingManoeuvres[i] == manoeuvre) {
					playingManoeuvres[i]->playbackPosition_seconds = 0.0;
					playingManoeuvres[i]->b_isPaused = false;
					for (auto& track : playingManoeuvres[i]->tracks) {
						track->playbackPosition_seconds = 0.0;
					}
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

	bool isPaused(const std::shared_ptr<Manoeuvre>& manoeuvre) {
		for (auto& m : playingManoeuvres) {
			if (m->b_isPaused && m == manoeuvre) return true;
		}
		return false;
	}

	void stopAllManoeuvres() {
		for (auto& manoeuvre : playingManoeuvres) {
			for (auto& track : manoeuvre->tracks) {
				track->parameter->machine->interruptParameterPlayback(track->parameter);
			}
		}
		playingManoeuvres.clear();
	}

	void incrementPlaybackPosition() {
		//update the playback position of playing manoeuvres and tracks
		double time = Environnement::getTime_seconds();
		for (auto& playingManoeuvre : playingManoeuvres) {
			if (!playingManoeuvre->b_isPaused) {
				//get playback position, but limit it to the length of the manoeuvre
				double playbackPosition = time - playingManoeuvre->playbackStartTime_seconds;
				double manoeuvreLength = playingManoeuvre->getLength_seconds();
				playbackPosition = std::min(playbackPosition, manoeuvreLength);
				playingManoeuvre->playbackPosition_seconds = playbackPosition;
				for (auto& track : playingManoeuvre->tracks) {
					track->playbackPosition_seconds = playbackPosition;
				}
			}
		}
	}

	void updateActiveManoeuvreState() {
		//check if manoeuvre rapids are finished
		for (int i = rapidManoeuvres.size() - 1; i >= 0; i--) {
			if (getRapidProgress(rapidManoeuvres[i]) >= 1.0) {
				Logger::debug("Manoeuvre {} Finished Priming", rapidManoeuvres[i]->name);
				rapidManoeuvres.erase(rapidManoeuvres.begin() + i);
			}
		}
		//terminate playback of manoeuvres that are finished
		for (int i = playingManoeuvres.size() - 1; i >= 0; i--) {
			if (playingManoeuvres[i]->getPlaybackProgress() >= 1.0) {
				playingManoeuvres[i]->playbackPosition_seconds = 0.0;
				for (auto& track : playingManoeuvres[i]->tracks) {
					//stop playback and reset playhead
					//track->parameter->machine->interruptParameterPlayback(track->parameter);
					track->parameter->machine->endParameterPlayback(track->parameter);
					track->playbackPosition_seconds = 0.0;
				}
				playingManoeuvres.erase(playingManoeuvres.begin() + i);
			}
		}
	}

}
