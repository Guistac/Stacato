#include <pch.h>

#include "Playback.h"

#include "Plot/Plot.h"
#include "Animation/Manoeuvre.h"
#include "Animation/Animation.h"
#include "Machine/Machine.h"
#include "Environnement/Environnement.h"

namespace PlaybackManager {

	std::mutex mutex;

	std::vector<std::shared_ptr<Animation>> activeAnimations;
	std::vector<std::shared_ptr<Manoeuvre>> activeManoeuvres;

	double playbackSpeedMultiplier = 1.0;
	double minPlaybackSpeedMultiplier = 0.5;
	double maxPlaybackSpeedMultiplier = 1.5;
	void setPlaybackSpeedMultiplier(double multiplier){ playbackSpeedMultiplier = std::clamp(multiplier, minPlaybackSpeedMultiplier, maxPlaybackSpeedMultiplier); }
	void resetPlaybackSpeedMultiplier(){ playbackSpeedMultiplier = 1.0; }
	double getPlaybackSpeedMultiplier(){ return playbackSpeedMultiplier; }
	double getMinPlaybackSpeedMultiplier(){ return minPlaybackSpeedMultiplier; }
	double getMaxPlaybackSpeedMutliplier(){ return maxPlaybackSpeedMultiplier; }

	void push(std::shared_ptr<Animation> animation){
		const std::lock_guard<std::mutex> lock(mutex);
		for(auto& activeAnimation : activeAnimations) if(activeAnimation == animation) return;
		activeAnimations.push_back(animation);
	}

	void push(std::shared_ptr<Manoeuvre> manoeuvre){
		const std::lock_guard<std::mutex> lock(mutex);
		for(auto& activeManoeuvre : activeManoeuvres) if(manoeuvre == activeManoeuvre) return;
		activeManoeuvres.push_back(manoeuvre);
	}

	std::vector<std::shared_ptr<Animation>> getActiveAnimations(){
		const std::lock_guard<std::mutex> lock(mutex);
		return activeAnimations;
	}

	std::vector<std::shared_ptr<Manoeuvre>> getActiveManoeuvres(){
		const std::lock_guard<std::mutex> lock(mutex);
		return activeManoeuvres;
	}

	bool isAnyAnimationActive(){
		return !activeAnimations.empty();
	}

	void stopAllAnimations(){
		for(auto& animation : activeAnimations) animation->stop();
	}


	long long getTime_microseconds(){
		return Environnement::getTime_nanoseconds() / 1000;
	}


	
	void removeAnimation(std::shared_ptr<Animation> animation){
		for(int i = 0; i < activeAnimations.size(); i++){
		   if(activeAnimations[i] == animation){
			   if(animation->getAnimatable()->currentAnimation == animation) animation->getAnimatable()->currentAnimation = nullptr;
			   activeAnimations.erase(activeAnimations.begin() + i);
			   break;
		   }
		}
	}

	void removeManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre){
		for(int i = 0; i < activeManoeuvres.size(); i++){
		   if(activeManoeuvres[i] == manoeuvre){
			   activeManoeuvres.erase(activeManoeuvres.begin() + i);
			   break;
		   }
		}
	}

	void update() {
		const std::lock_guard<std::mutex> lock(mutex);
		
		//check playback status of animations on previous cycle
		//remove them if they are finished
		std::vector<std::shared_ptr<Animation>> finishedAnimations;
		for(auto& animation : activeAnimations){
			animation->updatePlaybackState();
			if(!animation->isPlaying() && !animation->isPaused() && !animation->isInRapid()){
				finishedAnimations.push_back(animation);
			}
		}
		for(auto& animation : finishedAnimations) removeAnimation(animation);
		
		//check playback status of manoeuvres on previous cycle
		//remove them if they are finished
		std::vector<std::shared_ptr<Manoeuvre>> finishedManoeuvres;
		for(auto& manoeuvre : activeManoeuvres){
			manoeuvre->updatePlaybackState();
			if(!manoeuvre->hasActiveAnimations()) finishedManoeuvres.push_back(manoeuvre);
		}
		for(auto& manoeuvre : finishedManoeuvres) removeManoeuvre(manoeuvre);
		
		
		//increment playback position of current manoeuvres
		long long time_micros = Environnement::getTime_nanoseconds() / 1000;
		
		//we should increment the playaback position instead of setting a hard time
		long long timeIncrement_nanos = Environnement::getDeltaTime_nanoseconds();
		double adjustedTimeIncrement_seconds = playbackSpeedMultiplier * double(timeIncrement_nanos) / 1000000000.0;
		
		for (auto& animation : activeAnimations) {
			if(animation->isPlaying()){
				animation->incrementPlaybackPosition(adjustedTimeIncrement_seconds);
			}
		}
		
	}

}
