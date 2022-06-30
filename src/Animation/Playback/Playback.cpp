#include <pch.h>

#include "Playback.h"

#include "Project/Project.h"
#include "Plot/Plot.h"
#include "Animation/Manoeuvre.h"
#include "Animation/Animation.h"
#include "Machine/Machine.h"
#include "Environnement/Environnement.h"

namespace PlaybackManager {

	std::mutex mutex;

	std::vector<std::shared_ptr<Animation>> activeAnimations;

	void removeAnimation(std::shared_ptr<Animation> animation){
		for(int i = 0; i < activeAnimations.size(); i++){
			if(activeAnimations[i] == animation){
				activeAnimations.erase(activeAnimations.begin() + i);
				break;
			}
		}
	}

	void push(std::shared_ptr<Animation> animation){
		const std::lock_guard<std::mutex> lock(mutex);
		activeAnimations.push_back(animation);
	}

	void pop(std::shared_ptr<Animation> animation){
		const std::lock_guard<std::mutex> lock(mutex);
		removeAnimation(animation);
	}


	std::vector<std::shared_ptr<Animation>> getActiveAnimations(){
		const std::lock_guard<std::mutex> lock(mutex);
		return activeAnimations;
	}

	bool isAnyAnimationActive(){
		return activeAnimations.empty();
	}

	void stopAllAnimations(){
		for(auto& animation : activeAnimations) animation->stop();
	}


	long long getTime_microseconds(){
		return Environnement::getTime_nanoseconds() / 1000;
	}

	void update() {
		const std::lock_guard<std::mutex> lock(mutex);
		
		//check playback status of animations on previous cycle
		//remove them if they are finished
		std::vector<std::shared_ptr<Animation>> finishedAnimations;
		for(auto& animation : activeAnimations){
			animation->updatePlaybackState();
			if(animation->getPlaybackState() == Animation::PlaybackState::NOT_PLAYING){
				finishedAnimations.push_back(animation);
			}
		}
		for(auto& animation : finishedAnimations) removeAnimation(animation);
		
		//increment playback position of current manoeuvres
		long long time_micros = Environnement::getTime_nanoseconds() / 1000;
		for (auto& animation : activeAnimations) {
			if(animation->getPlaybackState() == Animation::PlaybackState::PLAYING){
				animation->incrementPlaybackPosition(time_micros);
			}
		}
	}

}
