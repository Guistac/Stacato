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
	for(auto& animation : animations){
		if(animation->canRapidToStart()) return true;
	}
	return false;
}

bool Manoeuvre::canRapidToTarget(){
	for(auto& animation : animations){
		if(animation->canRapidToTarget()) return true;
	}
	return false;
}


bool Manoeuvre::canRapidToPlaybackPosition(){
	for(auto& animation : animations){
		if(animation->canRapidToPlaybackPosition()) return true;
	}
	return false;
}



bool Manoeuvre::isAtStart(){
	for(auto& animation : animations){
		if(!animation->isAtStart()) return false;
	}
	return true;
}


bool Manoeuvre::isAtTarget(){
	for(auto& animation : animations){
		if(!animation->isAtTarget()) return false;
	}
	return true;
}

bool Manoeuvre::isAtPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::KEY:
		case ManoeuvreType::SEQUENCE:
			if(areNoMachinesEnabled()) return false;
			for(auto& animation : animations){
				if(animation->isMachineEnabled() && !animation->isAtPlaybackPosition()) return false;
			}
			return true;
	}
}



void Manoeuvre::rapidToStart(){
	if(!canRapidToStart()) return;
	for(auto& animation : animations) animation->rapidToStart();
}

void Manoeuvre::rapidToTarget(){
	if(!canRapidToTarget()) return;
	for(auto& animation : animations) animation->rapidToTarget();
}

void Manoeuvre::rapidToPlaybackPosition(){
	if(!canRapidToPlaybackPosition()) return;
	for(auto& animation : animations) animation->rapidToPlaybackPosition();
}










void Manoeuvre::updatePlaybackState(){
	for(auto& animation : getAnimations()){
		if(animation->getPlaybackState() != Animation::PlaybackState::NOT_PLAYING){
			b_hasActiveAnimations = true;
			return;
		}
	}
	b_hasActiveAnimations = false;
}

std::vector<std::shared_ptr<Animation>> Manoeuvre::getActiveAnimations(){
	std::vector<std::shared_ptr<Animation>> activeAnimations;
	for(auto& animation : getAnimations()){
		if(animation->getPlaybackState() != Animation::PlaybackState::NOT_PLAYING){
			activeAnimations.push_back(animation);
		}
	}
	return activeAnimations;
}



bool Manoeuvre::canStartPlayback(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE:
			for(auto& animation : animations){
				if(animation->isReadyToStartPlayback()) return true;
			}
			return false;
	}
}

bool Manoeuvre::canPausePlayback(){
	for(auto& animation : getAnimations()){
		if(animation->getPlaybackState() == Animation::PlaybackState::PLAYING) return true;
	}
	return false;
}

void Manoeuvre::startPlayback(){
	if(!canStartPlayback()) return;
	for(auto& animation : animations) animation->startPlayback();
	updateDuration();
}

void Manoeuvre::pausePlayback(){
	if(!canPausePlayback()) return;
	for(auto& animation : animations) animation->pausePlayback();
}

void Manoeuvre::updateDuration(){
	double largestTime = 0.0;
	for(auto& animation : animations){
		if(animation->getDuration() > largestTime) largestTime = animation->getDuration();
	}
	duration_seconds = largestTime;
}




bool Manoeuvre::canSetPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY:
			return false;
		case ManoeuvreType::TARGET:
			for(auto& animation : getAnimations()){
				if(animation->getPlaybackState() != Animation::PlaybackState::PAUSED) return false;
			}
			return true;
		case ManoeuvreType::SEQUENCE:
			for(auto& animation : getAnimations()){
				Animation::PlaybackState state = animation->getPlaybackState();
				if(state == Animation::PlaybackState::PLAYING || state == Animation::PlaybackState::IN_RAPID) return false;
			}
			return true;
	}
}

void Manoeuvre::setPlaybackPosition(double seconds){
	if(!canSetPlaybackPosition()) return;
	for(auto& animation : animations) animation->setPlaybackPosition(seconds);
}








bool Manoeuvre::canStop(){
	for(auto& animation : getAnimations()){
		if(animation->getPlaybackState() != Animation::PlaybackState::NOT_PLAYING) return true;
	}
	return false;
}

void Manoeuvre::stop(){
	if(!canStop()) return;
	for(auto& animation : animations) animation->stop();
}









bool Manoeuvre::hasDuration(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET:
			for(auto& animation : getAnimations()){
				switch(animation->getPlaybackState()){
					case Animation::PlaybackState::PLAYING:
					case Animation::PlaybackState::PAUSED: return true;
					default: break;
				}
			}
			return false;
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
		case ManoeuvreType::SEQUENCE: return 0.0;
	}
}

double Manoeuvre::getRemainingPlaybackTime(){
	switch(getType()){
		case ManoeuvreType::KEY: return 0.0;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return duration_seconds;
	}
}







void Manoeuvre::updatePlaybackStatus(){
	bool active = false;
	for(auto& animation : getAnimations()){
		if(animation->getPlaybackState() != Animation::PlaybackState::NOT_PLAYING) {
			active = true;
			break;
		}
	}
	b_hasActiveAnimations = active;
}
