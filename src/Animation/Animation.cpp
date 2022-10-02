#include <pch.h>

#include "Animation.h"

#include "Machine/Machine.h"
#include "Environnement/Environnement.h"
#include "Animation/Manoeuvre.h"

std::shared_ptr<Animation> Animation::create(std::shared_ptr<Animatable> animatable, ManoeuvreType manoeuvreType){
	std::shared_ptr<Animation> newAnimation;
	if(animatable->isComposite()) {
		newAnimation = std::make_shared<AnimationComposite>(animatable->toComposite(), manoeuvreType);
	}
	else{
		switch(manoeuvreType){
			case ManoeuvreType::KEY:
				newAnimation = std::make_shared<AnimationKey>(animatable);
				break;
			case ManoeuvreType::TARGET:
				newAnimation = std::make_shared<TargetAnimation>(animatable);
				break;
			case ManoeuvreType::SEQUENCE:
				newAnimation = std::make_shared<SequenceAnimation>(animatable);
				break;
		}
	}
	return newAnimation;
}

std::shared_ptr<Animation> Animation::copy(){
	std::shared_ptr<Animation> copy;
	if(isComposite()) copy = toComposite()->copy();
	else{
		switch(getType()){
			case ManoeuvreType::KEY:		copy = toKey()->copy(); break;
			case ManoeuvreType::TARGET: 	copy = toTarget()->copy(); break;
			case ManoeuvreType::SEQUENCE: 	copy = toSequence()->copy(); break;
		}
	}
	copy->subscribeToMachineParameter();
	return copy;
}

bool Animation::save(tinyxml2::XMLElement* xml){
	xml->SetAttribute("Machine", animatable->getMachine()->getName());
	xml->SetAttribute("MachineUniqueID", animatable->getMachine()->getUniqueID());
	xml->SetAttribute("Parameter", animatable->getName());
	xml->SetAttribute("IsComposite", isComposite());
	if(!isComposite()) xml->SetAttribute("Type", Enumerator::getSaveString(getType()));
	return onSave(xml);
}

std::shared_ptr<Animation> Animation::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	const char* machineName;
	int machineUniqueID;
	const char* animatableName;
	
	//we need to find the exact parameter object linked with the parameter track
	//to do this we match the machine name, machine unique id and then parse the machines parameters and match the parameter name
	
	if(xml->QueryStringAttribute("Machine", &machineName) != XML_SUCCESS) {
		Logger::warn("could not load machine name");
		return nullptr;
	}
	
	if(xml->QueryIntAttribute("MachineUniqueID", &machineUniqueID) != XML_SUCCESS){
		Logger::warn("Could not load machine unique id");
		return nullptr;
	}
	
	if(xml->QueryStringAttribute("Parameter", &animatableName) != XML_SUCCESS){
		Logger::warn("Could not load parameter name");
		return nullptr;
	}
	
	
	std::shared_ptr<Machine> machine = nullptr;
	for (auto& m : Environnement::getMachines()) {
		if (strcmp(machineName, m->getName()) == 0 && machineUniqueID == m->getUniqueID()) {
			machine = m;
			break;
		}
	}
	if (machine == nullptr) {
		Logger::warn("Could not identify machine {} with unique ID {}", machineName, machineUniqueID);
		return nullptr;
	}

	std::shared_ptr<Animatable> animatable = nullptr;
	for (auto& a : machine->getAnimatables()) {
		if (strcmp(animatableName, a->getName()) == 0) {
			animatable = a;
			break;
		}
	}
	if (animatable == nullptr) {
		Logger::warn("Could not find animatable named {} in machine {}", animatableName, machine->getName());
		return nullptr;
	}
	
	//once we have the parameter object, we can create and load the parameter track object:
	auto loadedTrack = load(xml, animatable);
	if(loadedTrack == nullptr) {
		Logger::warn("Failed to load parameter track {} of machine {}", animatableName, machine->getName());
		return nullptr;
	}
	loadedTrack->subscribeToMachineParameter();
	
	return loadedTrack;
}



std::shared_ptr<Animation> Animation::load(tinyxml2::XMLElement* xml, std::shared_ptr<Animatable> animatable){
	using namespace tinyxml2;
	
	bool isGroup = false;
	ManoeuvreType manoeuvreType;
	
	xml->QueryBoolAttribute("IsGroup", &isGroup);
	
	if(!isGroup){
		const char* typeSaveString;
		if(xml->QueryStringAttribute("Type", &typeSaveString) != XML_SUCCESS){
			Logger::warn("could not load type save string");
			return nullptr;
		}
		if(!Enumerator::getEnumeratorFromSaveString(typeSaveString, manoeuvreType)){
			Logger::warn("could not identify track type save string");
			return nullptr;
		}
	}
	
	std::shared_ptr<Animation> animation = nullptr;
	if(isGroup) AnimationComposite::load(xml, animatable->toComposite());
	else{
		switch(manoeuvreType){
			case ManoeuvreType::KEY: animation = AnimationKey::load(xml, animatable); break;
			case ManoeuvreType::TARGET: animation = TargetAnimation::load(xml, animatable); break;
			case ManoeuvreType::SEQUENCE: animation = SequenceAnimation::load(xml, animatable); break;
		}
	}
	if(animation == nullptr){
		Logger::warn("Could not load parameter track");
		return nullptr;
	}
	return animation;
}






//————————— Validation & Filling ——————————

void Animation::fillDefaults(){
	animatable->getMachine()->fillAnimationDefaults(shared_from_this());
}

void Animation::validate(){
	validationErrorString = "";
	b_valid = getAnimatable()->validateAnimation(shared_from_this());
	if(hasManoeuvre()) manoeuvre->updateAnimationSummary();
}

void Animation::subscribeToMachineParameter(){
	animatable->subscribeAnimation(shared_from_this());
}

void Animation::unsubscribeFromMachineParameter(){
	animatable->unsubscribeAnimation(shared_from_this());
}






//—————————— Rapids ——————————

bool Animation::canRapid(){
	return animatable->isReadyToMove();
}

void Animation::rapidToStart(){
	if(!canRapidToStart()) return;
	animatable->stopAnimation();
	if(onRapidToStart()){
		setPlaybackPosition(0.0);
		
		b_isPlaying = false;
		b_isInRapid = true;
		
		auto thisAnimation = shared_from_this();
		animatable->currentAnimation = thisAnimation;
		
		PlaybackManager::push(thisAnimation);
		if(hasManoeuvre()) PlaybackManager::push(getManoeuvre());
	}
}

void Animation::rapidToTarget(){
	if(!canRapidToTarget()) return;
	animatable->stopAnimation();
	if(onRapidToTarget()){
		setPlaybackPosition(duration_seconds);
		
		b_isPlaying = false;
		b_isInRapid = true;
		
		auto thisAnimation = shared_from_this();
		animatable->currentAnimation = thisAnimation;
		
		PlaybackManager::push(thisAnimation);
		if(hasManoeuvre()) PlaybackManager::push(getManoeuvre());
	}
}

void Animation::rapidToPlaybackPosition(){
	if(!canRapidToPlaybackPosition()) return;
	animatable->stopAnimation();
	if(onRapidToPlaybackPosition()){
		
		b_isPlaying = false;
		b_isInRapid = true;
		if(getType() == ManoeuvreType::KEY) b_isPaused = false;
		else b_isPaused = true;
		
		auto thisAnimation = shared_from_this();
		animatable->currentAnimation = thisAnimation;
		
		PlaybackManager::push(thisAnimation);
		if(hasManoeuvre()) PlaybackManager::push(getManoeuvre());
	}
}

float Animation::getRapidProgress(){
	return animatable->getRapidProgress();
}

float Animation::getPlaybackProgress(){
    if(duration_seconds == 0.0) return 0.0;
	return playbackPosition_seconds / duration_seconds;
}




//—————————— Playback ——————————

void Animation::startPlayback(){
	if(!b_isPaused) animatable->stopAnimation(); //TODO: verify if this interferes with the next line
	//animatable->stopAnimation();
	if(onStartPlayback()){
		updateDuration();
		
		//the offset is to account for starting playback when the playback position is not zero
		playbackStartTime_microseconds = PlaybackManager::getTime_microseconds() - playbackPosition_seconds * 1000000;
		b_isPlaying = true;
		b_isPaused = false;
		b_isInRapid = false;
		
		auto thisAnimation = shared_from_this();
		animatable->onPlaybackStart(thisAnimation);
		//animatable->currentAnimation = thisAnimation;
		//animatable->onPlaybackStart();
		
		PlaybackManager::push(thisAnimation);
		if(manoeuvre) PlaybackManager::push(manoeuvre);
		
		requestCurveRefocus();
	}
}

void Animation::pausePlayback(){
	if(!b_isPlaying) return;
	b_isPlaying = false;
	b_isPaused = true;
	animatable->onPlaybackPause();
}

void Animation::stopPlayback(){
	if(!b_isPlaying && !b_isPaused) return;
	b_isPlaying = false;
	b_isPaused = false;
	if(animatable->currentAnimation == shared_from_this()) animatable->onPlaybackStop();
	setPlaybackPosition(0.0);
}





void Animation::stopRapid(){
	animatable->cancelRapid();
	animatable->currentAnimation = nullptr;
	b_isInRapid = false;
	//setPlaybackPosition(0.0);
}



bool Animation::isMachineEnabled(){ return animatable->getMachine()->isEnabled(); }

std::shared_ptr<AnimationValue> Animation::getValueAtPlaybackTime(){
	return animatable->getValueAtAnimationTime(shared_from_this(), playbackPosition_seconds);
}

void Animation::stop(){
	if(isPlaying() || isPaused()) {
		stopPlayback();
	}
	else if(isInRapid()) {
		stopRapid();
	}
}

void Animation::updateDuration(){
	double longestCurveDuration = 0.0;
	for(auto& curve : getCurves()) {
		longestCurveDuration = std::max(longestCurveDuration, curve->getLength());
	}
	duration_seconds = longestCurveDuration;
}

void Animation::updatePlaybackState(){
	if(isPlaying()){
		if(playbackPosition_seconds >= duration_seconds && isAtTarget() && !animatable->isMoving()) {
			b_isPlaying = false;
			b_isPaused = false;
			animatable->currentAnimation = nullptr;
			animatable->onPlaybackEnd();
			setPlaybackPosition(0.0);
		}
	}
	else if(isInRapid()){
		if(!animatable->isInRapid()) b_isInRapid = false;
	}
}

void Animation::incrementPlaybackPosition(long long playbackTime_microseconds){
	playbackPosition_seconds = (playbackTime_microseconds - playbackStartTime_microseconds) / 1000000.0;
	playbackPosition_seconds = std::min(playbackPosition_seconds, duration_seconds);
}
