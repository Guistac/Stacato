#include <pch.h>

#include "Manoeuvre.h"
#include "Machine/Machine.h"
#include "Animation/Animatable.h"
#include "Animation/Animation.h"
#include "Plot/ManoeuvreList.h"
#include "Animation/Playback/Playback.h"

#include "Plot/Plot.h"

#include <tinyxml2.h>


bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	
	using namespace tinyxml2;
	
	name->save(manoeuvreXML);
	description->save(manoeuvreXML);
	type->save(manoeuvreXML);
	
	for (auto& animation : animations) {
		//tracks which have a group parent are listed in the manoeuvres track vector
		//but they don't get saved in the main manoeuvre
		//instead they are saved by their parent parameter track
		if (animation->hasParentComposite()) continue;
		XMLElement* animationXML = manoeuvreXML->InsertNewChildElement("Animation");
		animation->save(animationXML);
	}
	 
	return true;
}

std::shared_ptr<Manoeuvre> Manoeuvre::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	auto manoeuvre = Manoeuvre::make(ManoeuvreType::KEY);
	
	manoeuvre->name->load(xml);
	manoeuvre->description->load(xml);
	manoeuvre->type->load(xml);
	
	XMLElement* animationXML = xml->FirstChildElement("Animation");
	while (animationXML != nullptr) {
		auto newAnimation = Animation::load(animationXML);
		if(newAnimation == nullptr) return nullptr;
		newAnimation->setManoeuvre(manoeuvre);
		manoeuvre->animations.push_back(newAnimation);
		animationXML = animationXML->NextSiblingElement("Animation");
	}
	
	manoeuvre->validateAllParameterTracks();
	
	return manoeuvre;
}


class SetManoeuvreTypeCommand : public UndoableCommand{
public:
	
	std::shared_ptr<Manoeuvre> manoeuvre;
	ManoeuvreType newType;
	std::vector<std::shared_ptr<Animation>> oldTracks;
	std::vector<std::shared_ptr<Animation>> newTracks;
	
	SetManoeuvreTypeCommand(std::string& name, std::shared_ptr<Manoeuvre> manoeuvre_, ManoeuvreType newType_) : UndoableCommand(name){
		manoeuvre = manoeuvre_;
		newType = newType_;
		oldTracks = manoeuvre->getAnimations();
	}
	
	virtual void onExecute(){
		for(int i = 0; i < manoeuvre->getAnimations().size(); i++){
			std::shared_ptr<Animation>& animation = manoeuvre->getAnimations()[i];
			animation = Animation::create(animation->getAnimatable(), newType);
			animation->setManoeuvre(manoeuvre);
			animation->validate();
		}
		newTracks = manoeuvre->getAnimations();
		manoeuvre->updateTrackSummary();
	}
	virtual void onUndo(){
		manoeuvre->getAnimations() = oldTracks;
		manoeuvre->updateTrackSummary();
	}
	virtual void onRedo(){
		manoeuvre->getAnimations() = newTracks;
		manoeuvre->updateTrackSummary();
	}

};



std::shared_ptr<Manoeuvre> Manoeuvre::make(ManoeuvreType type){
	auto manoeuvre = std::make_shared<Manoeuvre>();
	manoeuvre->type->overwrite(type);
	manoeuvre->type->setEditCallback([manoeuvre](std::shared_ptr<Parameter> parameter){
		auto typeParameter = std::dynamic_pointer_cast<EnumeratorParameter<ManoeuvreType>>(parameter);
		std::string commandName = "Set Manoeuvre type to " + std::string(Enumerator::getDisplayString(manoeuvre->getType()));
		std::make_shared<SetManoeuvreTypeCommand>(commandName, manoeuvre, typeParameter->value)->execute();
	});
	return manoeuvre;
}

std::shared_ptr<Manoeuvre> Manoeuvre::copy(){
	auto copy = make();
	std::string copyName = std::string(getName()) + " (copy)";
	copy->name->overwrite(copyName);
	copy->description->overwrite(getDescription());
	copy->type->overwrite(getType());
	copy->manoeuvreList = manoeuvreList;
	for(auto animation : getAnimations()){
		auto animationCopy = animation->copy();
		animationCopy->setManoeuvre(copy);
		copy->animations.push_back(animationCopy);
	}
	return copy;
}






class AddAnimationCommand : public UndoableCommand{
public:
	
	std::shared_ptr<Animatable> animatable;
	std::shared_ptr<Animation> addedAnimation;
	std::shared_ptr<Manoeuvre> manoeuvre;
	
	AddAnimationCommand(std::string& name, std::shared_ptr<Animatable> animatable_, std::shared_ptr<Manoeuvre> manoeuvre_) : UndoableCommand(name){
		animatable = animatable_;
		manoeuvre = manoeuvre_;
	}
	
	virtual void onExecute(){
		addedAnimation = animatable->makeAnimation(manoeuvre->getType());
		manoeuvre->getAnimations().push_back(addedAnimation);
		addedAnimation->setManoeuvre(manoeuvre);
		addedAnimation->fillDefaults();
		addedAnimation->validate();
	}
	
	virtual void onUndo(){
		auto& animations = manoeuvre->getAnimations();
		for(int i = 0; i < animations.size(); i++){
			if(animations[i] == addedAnimation){
				animations.erase(animations.begin() + i);
				break;
			}
		}
		addedAnimation->unsubscribeFromMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
	virtual void onRedo(){
		manoeuvre->getAnimations().push_back(addedAnimation);
		addedAnimation->subscribeToMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
};

void Manoeuvre::addAnimation(std::shared_ptr<Animatable> animatable) {
	std::string name = "Add Animation " + /*std::string(animatable->getMachine()->getName()) + " : " +*/ std::string(animatable->getName());
	std::make_shared<AddAnimationCommand>(name, animatable, shared_from_this())->execute();
}









class RemoveAnimationCommand : public UndoableCommand{
public:
	
	std::shared_ptr<Animatable> removedAnimatable;
	std::shared_ptr<Animation> removedAnimation;
	std::shared_ptr<Manoeuvre> manoeuvre;
	int removeIndex;
	
	RemoveAnimationCommand(std::string& name, std::shared_ptr<Animatable> removedParameter_, std::shared_ptr<Manoeuvre> manoeuvre_) : UndoableCommand(name){
		removedAnimatable = removedParameter_;
		manoeuvre = manoeuvre_;
	}
	
	virtual void onExecute(){
		auto& animations = manoeuvre->getAnimations();
		for (int i = 0; i < animations.size(); i++) {
			if (animations[i]->getAnimatable() == removedAnimatable) {
				removedAnimation = animations[i];
				animations.erase(animations.begin() + i);
				removeIndex = i;
				break;
			}
		}
		removedAnimation->unsubscribeFromMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
	virtual void onUndo(){
		auto& animations = manoeuvre->getAnimations();
		animations.insert(animations.begin() + removeIndex, removedAnimation);
		removedAnimation->subscribeToMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
};

void Manoeuvre::removeAnimation(std::shared_ptr<Animatable> animatable) {
	if(hasAnimation(animatable)){
		std::string name = "Remove Animation " + /*std::string(animatable->getMachine()->getName()) + " : " +*/ std::string(animatable->getName());
		std::make_shared<RemoveAnimationCommand>(name, animatable, shared_from_this())->execute();
	}
}






class MoveAnimationCommand : public UndoableCommand{
public:
	
	int newIndex, oldIndex;
	std::shared_ptr<Manoeuvre> manoeuvre;
	
	MoveAnimationCommand(std::string& name, int oldIndex_, int newIndex_, std::shared_ptr<Manoeuvre> manoeuvre_) : UndoableCommand(name){
		manoeuvre = manoeuvre_;
		oldIndex = oldIndex_;
		newIndex = newIndex_;
	}
	
	virtual void onExecute(){
		auto& animations = manoeuvre->getAnimations();
		auto temp = animations[oldIndex];
		animations.erase(animations.begin() + oldIndex);
		animations.insert(animations.begin() + newIndex, temp);
	}
	
	virtual void onUndo(){
		auto& animations = manoeuvre->getAnimations();
		auto temp = animations[newIndex];
		animations.erase(animations.begin() + newIndex);
		animations.insert(animations.begin() + oldIndex, temp);
	}
	
};

void Manoeuvre::moveAnimation(int oldIndex, int newIndex){
	if(oldIndex < 0) return;
	if(oldIndex >= animations.size()) return;
	if(newIndex < 0) return;
	if(newIndex >= animations.size()) return;
	if(oldIndex == newIndex) return;
	std::string name = "Move Animation " + std::string(animations[oldIndex]->getAnimatable()->getName());
	std::make_shared<MoveAnimationCommand>(name, oldIndex, newIndex, shared_from_this())->execute();
}


bool Manoeuvre::hasAnimation(std::shared_ptr<Animatable> animatable) {
	for (auto& animation : animations) {
		if (animation->getAnimatable() == animatable) return true;
	}
	return false;
}








void Manoeuvre::subscribeAllTracksToMachineParameter(){
	for(auto animation : animations) animation->subscribeToMachineParameter();
}
void Manoeuvre::unsubscribeAllTracksFromMachineParameter(){
	for(auto animation : animations) animation->unsubscribeFromMachineParameter();
}


void Manoeuvre::validateAllParameterTracks(){
	for(auto& animation : animations) animation->validate();
}

void Manoeuvre::updateTrackSummary(){
	double longestAnimationDuration_seconds = 0.0;
	bool allAnimationsValid = true;
	for(auto& animation : animations){
		if(!animation->isValid()) allAnimationsValid = false;
		longestAnimationDuration_seconds = std::max(longestAnimationDuration_seconds, animation->getDuration());
	}
	b_valid = allAnimationsValid;
	duration_seconds = longestAnimationDuration_seconds;
}


void Manoeuvre::getCurveRange(double& minX, double& maxX, double& minY, double& maxY){
	double maX = DBL_MIN;
	double miY = DBL_MAX;
	double maY = DBL_MIN;
	for(auto& animation : animations){
		maX = std::max(maX, animation->getDuration());
		double animMinY, animMaxY;
		animation->getCurvePositionRange(animMinY, animMaxY);
		miY = std::min(miY, animMinY);
		maY = std::max(maY, animMaxY);
	}
	minX = 0;
	maxX = maX;
	minY = miY;
	maxY = maY;
}






bool Manoeuvre::isSelected(){
	if(!isInManoeuvreList()) return false;
	return manoeuvreList->getPlot()->getSelectedManoeuvre() == shared_from_this();
}

void Manoeuvre::select(){
	if(isInManoeuvreList()) manoeuvreList->getPlot()->selectManoeuvre(shared_from_this());
}

void Manoeuvre::deselect(){
	if(!isSelected()) return;
	if(isInManoeuvreList()) manoeuvreList->getPlot()->selectManoeuvre(nullptr);
}










//OK
bool Manoeuvre::areAllMachinesEnabled(){
	for(auto& animation : animations){
		if(!animation->isMachineEnabled()) return false;
	}
	return true;
}

//OK
bool Manoeuvre::areNoMachinesEnabled(){
	for(auto& animation : animations){
		if(animation->isMachineEnabled()) return false;
	}
	return true;
}






bool Manoeuvre::canRapidToStart(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE: return !areNoMachinesEnabled();
	}
}

bool Manoeuvre::isAtStart(){
	for(auto& animation : animations){
		if(!animation->isAtStart()) return false;
	}
	return true;
}

void Manoeuvre::rapidToStart(){
	if(!canRapidToStart()) return;
	b_inRapid = true;
	b_playing = false;
	b_paused = false;
	for(auto& animation : animations) animation->rapidToStart();
	PlaybackManager::push(shared_from_this());
	playbackPosition_seconds = 0.0;
	for(auto& animation : animations) animation->setPlaybackPosition(0.0);
}







bool Manoeuvre::canRapidToTarget(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return !areNoMachinesEnabled();
	}
}

bool Manoeuvre::isAtTarget(){
	for(auto& animation : animations){
		if(!animation->isAtTarget()) return false;
	}
	return true;
}

void Manoeuvre::rapidToTarget(){
	if(!canRapidToTarget()) return;
	b_inRapid = true;
	b_playing = false;
	b_paused = false;
	for(auto& animation : animations) animation->rapidToTarget();
	PlaybackManager::push(shared_from_this());
	playbackPosition_seconds = duration_seconds;
	for(auto& animation : animations) animation->setPlaybackPosition(duration_seconds);
}






bool Manoeuvre::canRapidToPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE: return !areNoMachinesEnabled();
	}
}

bool Manoeuvre::isAtPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE:
			if(areNoMachinesEnabled()) return false;
			for(auto& animation : animations){
				if(animation->isMachineEnabled() && !animation->isAtPlaybackPosition()) return false;
			}
			return true;
	}
}

void Manoeuvre::rapidToPlaybackPosition(){
	if(!canRapidToPlaybackPosition()) return;
	b_inRapid = true;
	b_playing = false;
	b_paused = false;
	for(auto& animation : animations) animation->rapidToPlaybackPosition();
	PlaybackManager::push(shared_from_this());
}





float Manoeuvre::getRapidProgress(){
	float smallestProgress = 1.0;
	for(auto& animation : animations){
		float progress = animation->getRapidProgress();
		smallestProgress = std::min(smallestProgress, progress);
	}
	return smallestProgress;
}

bool Manoeuvre::isRapidFinished(){
	return getRapidProgress() >= 1.0;
}








bool Manoeuvre::canStartPlayback(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET: return !areNoMachinesEnabled();
		case ManoeuvreType::SEQUENCE:
			if(areNoMachinesEnabled()) return false;
			for(auto& animation : animations){
				if(animation->isMachineEnabled() && !animation->isReadyToStartPlayback()) return false;
			}
			return true;
	}
}

bool Manoeuvre::canPausePlayback(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return isPlaying();
	}
}

void Manoeuvre::startPlayback(){
	if(!canStartPlayback()) return;
	//the offset is to account for starting playback when the playback position is not zero
	playbackStartTime_microseconds = PlaybackManager::getTime_microseconds() - playbackPosition_seconds * 1000000;
	b_inRapid = false;
	b_playing = true;
	b_paused = false;
	for(auto& animation : animations) animation->startPlayback();
	if(getType() == ManoeuvreType::TARGET){
		double largestTime = 0.0;
		for(auto& animation : animations){
			if(animation->getDuration() > largestTime) largestTime = animation->getDuration();
		}
		duration_seconds = largestTime;
	}
	PlaybackManager::push(shared_from_this());
}

void Manoeuvre::pausePlayback(){
	if(!canPausePlayback()) return;
	b_inRapid = false;
	b_playing = false;
	b_paused = true;
	for(auto& animation : animations) animation->interruptPlayback();
}






bool Manoeuvre::canSetPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE: return isFinished() || isPaused();
	}
}

void Manoeuvre::setPlaybackPosition(double seconds){
	if(!canSetPlaybackPosition()) return;
	playbackPosition_seconds = seconds;
	for(auto& animation : animations) animation->setPlaybackPosition(seconds);
}










void Manoeuvre::stop(){
	if(isFinished()) return;
	b_inRapid = false;
	b_playing = false;
	b_paused = false;
	for(auto& animation : animations) animation->stop();
	PlaybackManager::pop(shared_from_this());
	playbackPosition_seconds = 0.0;
	for(auto& animation : animations) animation->setPlaybackPosition(0.0);
}









bool Manoeuvre::hasDuration(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET:	return isPlaying();
		case ManoeuvreType::SEQUENCE: return true;
	}
}

double Manoeuvre::getDuration(){
	switch(getType()){
		case ManoeuvreType::KEY: return 0.0;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return duration_seconds;
	}
}

double Manoeuvre::getPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY: return 0.0;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return playbackPosition_seconds;
	}
}

double Manoeuvre::getRemainingPlaybackTime(){
	switch(getType()){
		case ManoeuvreType::KEY: return 0.0;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return playbackPosition_seconds - duration_seconds;
	}
}







void Manoeuvre::incrementPlaybackPosition(long long playbackTime_microseconds){
	if(isPaused()) return;
	playbackPosition_seconds = (playbackTime_microseconds - playbackStartTime_microseconds) / 1000000.0;
	for(auto& animation : animations) animation->setPlaybackPosition(playbackPosition_seconds);
}

void Manoeuvre::updatePlaybackStatus(){
	for(auto& animation : animations) animation->updatePlaybackStatus();
	
	if(isPlaying() && isPlaybackFinished()) {
		//needs to call onPlaybackEnd in machines
		b_playing = false;
		playbackPosition_seconds = 0.0;
		for(auto& animation : animations) animation->setPlaybackPosition(0.0);
	}
	else if(isInRapid() && isRapidFinished()) {
		b_inRapid = false;
	}
}

void Manoeuvre::onTrackPlaybackStop(){
	bool anyTracksStillPlaying = false;
	for(auto& animation : animations){
		if(animation->isPlaying()) {
			anyTracksStillPlaying = true;
			break;
		}
	}
	if(!anyTracksStillPlaying){
		b_playing = false;
		playbackPosition_seconds = 0.0;
		for(auto& animation : animations) animation->setPlaybackPosition(0.0);
		PlaybackManager::pop(shared_from_this());
	}
}
