#include <pch.h>

#include "Manoeuvre.h"
#include "Machine/Machine.h"
#include "Machine/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Plot/ManoeuvreList.h"

#include "Plot/Plot.h"

class SetManoeuvreTypeCommand : public Command{
public:
	
	std::shared_ptr<Manoeuvre> manoeuvre;
	ManoeuvreType newType;
	std::vector<std::shared_ptr<ParameterTrack>> oldTracks;
	std::vector<std::shared_ptr<ParameterTrack>> newTracks;
	
	SetManoeuvreTypeCommand(std::string& name, std::shared_ptr<Manoeuvre> manoeuvre_, ManoeuvreType newType_) : Command(name){
		manoeuvre = manoeuvre_;
		newType = newType_;
		oldTracks = manoeuvre->getTracks();
	}
	
	virtual void execute(){
		for(int i = 0; i < manoeuvre->getTracks().size(); i++){
			std::shared_ptr<ParameterTrack>& track = manoeuvre->getTracks()[i];
			track = ParameterTrack::create(track->getParameter(), newType);
			track->setManoeuvre(manoeuvre);
		}
		newTracks = manoeuvre->getTracks();
		manoeuvre->updateTrackSummary();
	}
	virtual void undo(){
		manoeuvre->getTracks() = oldTracks;
		manoeuvre->updateTrackSummary();
	}
	virtual void redo(){
		manoeuvre->getTracks() = newTracks;
		manoeuvre->updateTrackSummary();
	}

};

Manoeuvre::Manoeuvre(){
	type->setEditCallback([this](std::shared_ptr<Parameter> parameter){
		auto typeParameter = std::dynamic_pointer_cast<EnumeratorParameter<ManoeuvreType>>(parameter);
		std::string commandName = "Set Manoeuvre type to " + std::string(Enumerator::getDisplayString(this->getType()));
		auto setManoeuvreTypeCommand = std::make_shared<SetManoeuvreTypeCommand>(commandName, shared_from_this(), typeParameter->value);
		CommandHistory::pushAndExecute(setManoeuvreTypeCommand);
	});
}

std::shared_ptr<Manoeuvre> Manoeuvre::copy(){
	auto copy = std::make_shared<Manoeuvre>();
	
	std::string copyName = std::string(getName()) + " (copy)";
	copy->name->overwrite(copyName);
	copy->description->overwrite(getDescription());
	copy->type->overwrite(getType());
	copy->manoeuvreList = manoeuvreList;
	
	for(auto track : getTracks()){
		auto trackCopy = ParameterTrack::copy(track);
		trackCopy->setManoeuvre(copy);
		copy->tracks.push_back(trackCopy);
	}
	
	return copy;
}






class AddParameterTrackCommand : public Command{
public:
	
	std::shared_ptr<MachineParameter> machineParameter;
	std::shared_ptr<ParameterTrack> addedTrack;
	std::shared_ptr<Manoeuvre> manoeuvre;
	
	AddParameterTrackCommand(std::string& name, std::shared_ptr<MachineParameter> machineParameter_, std::shared_ptr<Manoeuvre> manoeuvre_) : Command(name){
		machineParameter = machineParameter_;
		manoeuvre = manoeuvre_;
	}
	
	virtual void execute(){
		addedTrack = machineParameter->createTrack(manoeuvre->getType());
		manoeuvre->getTracks().push_back(addedTrack);
		addedTrack->setManoeuvre(manoeuvre);
		addedTrack->validate();
	}
	
	virtual void undo(){
		auto& tracks = manoeuvre->getTracks();
		for(int i = 0; i < tracks.size(); i++){
			if(tracks[i] == addedTrack){
				tracks.erase(tracks.begin() + i);
				break;
			}
		}
		addedTrack->unsubscribeFromMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
	virtual void redo(){
		manoeuvre->getTracks().push_back(addedTrack);
		addedTrack->subscribeToMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
};

void Manoeuvre::addTrack(std::shared_ptr<MachineParameter>& parameter) {
	std::string name = "Add Track " + std::string(parameter->getMachine()->getName()) + " : " + std::string(parameter->getName());
	auto command = std::make_shared<AddParameterTrackCommand>(name, parameter, shared_from_this());
	CommandHistory::pushAndExecute(command);
}









class RemoveParameterTrackCommand : public Command{
public:
	
	std::shared_ptr<MachineParameter> removedParameter;
	std::shared_ptr<ParameterTrack> removedTrack;
	std::shared_ptr<Manoeuvre> manoeuvre;
	int removeIndex;
	
	RemoveParameterTrackCommand(std::string& name, std::shared_ptr<MachineParameter> removedParameter_, std::shared_ptr<Manoeuvre> manoeuvre_) : Command(name){
		removedParameter = removedParameter_;
		manoeuvre = manoeuvre_;
	}
	
	virtual void execute(){
		auto& tracks = manoeuvre->getTracks();
		for (int i = 0; i < tracks.size(); i++) {
			if (tracks[i]->getParameter() == removedParameter) {
				removedTrack = tracks[i];
				tracks.erase(tracks.begin() + i);
				removeIndex = i;
				break;
			}
		}
		removedTrack->unsubscribeFromMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
	virtual void undo(){
		auto& tracks = manoeuvre->getTracks();
		tracks.insert(tracks.begin() + removeIndex, removedTrack);
		removedTrack->subscribeToMachineParameter();
		manoeuvre->updateTrackSummary();
	}
	
};

void Manoeuvre::removeTrack(std::shared_ptr<MachineParameter> parameter) {
	if(hasTrack(parameter)){
		std::string name = "Remove Track " + std::string(parameter->getMachine()->getName()) + " : " + std::string(parameter->getName());
		auto command = std::make_shared<RemoveParameterTrackCommand>(name, parameter, shared_from_this());
		CommandHistory::pushAndExecute(command);
	}
}






class MoveParameterTrackCommand : public Command{
public:
	
	int newIndex, oldIndex;
	std::shared_ptr<Manoeuvre> manoeuvre;
	
	MoveParameterTrackCommand(std::string& name, int oldIndex_, int newIndex_, std::shared_ptr<Manoeuvre> manoeuvre_) : Command(name){
		manoeuvre = manoeuvre_;
		oldIndex = oldIndex_;
		newIndex = newIndex_;
	}
	
	virtual void execute(){
		auto& tracks = manoeuvre->getTracks();
		auto temp = tracks[oldIndex];
		tracks.erase(tracks.begin() + oldIndex);
		tracks.insert(tracks.begin() + newIndex, temp);
	}
	
	virtual void undo(){
		auto& tracks = manoeuvre->getTracks();
		auto temp = tracks[newIndex];
		tracks.erase(tracks.begin() + newIndex);
		tracks.insert(tracks.begin() + oldIndex, temp);
	}
	
};

void Manoeuvre::moveTrack(int oldIndex, int newIndex){
	if(oldIndex < 0) return;
	if(oldIndex >= tracks.size()) return;
	if(newIndex < 0) return;
	if(newIndex >= tracks.size()) return;
	if(oldIndex == newIndex) return;
	std::string name = "Move Track " + std::string(tracks[oldIndex]->getParameter()->getName());
	auto command = std::make_shared<MoveParameterTrackCommand>(name, oldIndex, newIndex, shared_from_this());
	CommandHistory::pushAndExecute(command);
}






void Manoeuvre::subscribeAllTracksToMachineParameter(){
	for(auto track : tracks) track->subscribeToMachineParameter();
}
void Manoeuvre::unsubscribeAllTracksFromMachineParameter(){
	for(auto track : tracks) track->unsubscribeFromMachineParameter();
}


void Manoeuvre::validateAllParameterTracks(){
	for(auto& track : tracks) track->validate();
}

void Manoeuvre::updateTrackSummary(){
	double longestTrackDuration_seconds = 0.0;
	bool allTracksValid = true;
	for(auto& track : tracks){
		if(!track->isValid()) allTracksValid = false;
		longestTrackDuration_seconds = std::max(longestTrackDuration_seconds, track->getDuration());
	}
	b_valid = allTracksValid;
	duration_seconds = longestTrackDuration_seconds;
}


bool Manoeuvre::hasTrack(std::shared_ptr<MachineParameter>& parameter) {
	for (auto& track : tracks) {
		if (track->getParameter() == parameter) return true;
	}
	return false;
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
	for(auto& track : tracks){
		if(!track->isMachineEnabled()) return false;
	}
	return true;
}

//OK
bool Manoeuvre::areNoMachinesEnabled(){
	for(auto& track : tracks){
		if(track->isMachineEnabled()) return false;
	}
	return true;
}

//OK
bool Manoeuvre::canRapidToStart(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE: return !areNoMachinesEnabled();
	}
}

//OK
bool Manoeuvre::isAtStart(){
	for(auto& track : tracks){
		if(!track->isAtStart()) return false;
	}
	return true;
}

//OK
bool Manoeuvre::canRapidToTarget(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return !areNoMachinesEnabled();
	}
}

//OK
bool Manoeuvre::isAtTarget(){
	for(auto& track : tracks){
		if(!track->isAtTarget()) return false;
	}
	return true;
}

//OK
bool Manoeuvre::canRapidToPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE: return !areNoMachinesEnabled();
	}
}

//OK but needs work on parameter track side
bool Manoeuvre::isAtPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY:
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE:
			for(auto& track : tracks){
				if(!track->isAtPlaybackPosition()) return false;
			}
			return true;
	}
}

//OK
bool Manoeuvre::canSetPlaybackPosition(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET: return false;
		case ManoeuvreType::SEQUENCE: return isFinished() || isPaused();
	}
}

//OK
bool Manoeuvre::canStartPlayback(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET: return !areNoMachinesEnabled();
		case ManoeuvreType::SEQUENCE:
			for(auto& track : tracks){
				if(!track->isReadyToStartPlayback()) return false;
			}
			return true;
	}
}

//OK
bool Manoeuvre::canPausePlayback(){
	switch(getType()){
		case ManoeuvreType::KEY: return false;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return isPlaying();
	}
}

//OK
bool Manoeuvre::canStop(){
	return !isFinished();
}

//commands



#include "Motion/Playback/Playback.h"

//OK
void Manoeuvre::rapidToStart(){
	if(!canRapidToStart()) return;
	b_inRapid = true;
	b_playing = false;
	b_paused = false;
	for(auto& track : tracks) track->rapidToStart();
	PlaybackManager::push(shared_from_this());
	playbackPosition_seconds = 0.0;
	for(auto& track : tracks) track->setPlaybackPosition(0.0);
}

//OK
void Manoeuvre::rapidToTarget(){
	if(!canRapidToTarget()) return;
	b_inRapid = true;
	b_playing = false;
	b_paused = false;
	for(auto& track : tracks) track->rapidToTarget();
	PlaybackManager::push(shared_from_this());
	playbackPosition_seconds = duration_seconds;
	for(auto& track : tracks) track->setPlaybackPosition(duration_seconds);
}


void Manoeuvre::rapidToPlaybackPosition(){
	if(!canRapidToPlaybackPosition()) return;
	b_inRapid = true;
	b_playing = false;
	b_paused = false;
	for(auto& track : tracks) track->rapidToPlaybackPosition();
	PlaybackManager::push(shared_from_this());
}


void Manoeuvre::startPlayback(){
	if(!canStartPlayback()) return;
	//the offset is to account for starting playback when the playback position is not zero
	playbackStartTime_microseconds = PlaybackManager::getTime_microseconds() - playbackPosition_seconds * 1000000;
	b_inRapid = false;
	b_playing = true;
	b_paused = false;
	for(auto& track : tracks) track->startPlayback();
	PlaybackManager::push(shared_from_this());
}

void Manoeuvre::pausePlayback(){
	if(!canPausePlayback()) return;
	b_inRapid = false;
	b_playing = false;
	b_paused = true;
	for(auto& track : tracks) track->stop();
}

void Manoeuvre::setPlaybackPosition(double seconds){
	if(!canSetPlaybackPosition()) return;
	playbackPosition_seconds = seconds;
	for(auto& track : tracks) track->setPlaybackPosition(seconds);
}

//OK needs for for actual playback
void Manoeuvre::stop(){
	if(isFinished()) return;
	b_inRapid = false;
	b_playing = false;
	b_paused = false;
	for(auto& track : tracks) track->stop();
	PlaybackManager::pop(shared_from_this());
	playbackPosition_seconds = 0.0;
	for(auto& track : tracks) track->setPlaybackPosition(0.0);
}


//OK
float Manoeuvre::getRapidProgress(){
	float smallestProgress = 1.0;
	for(auto& track : tracks){
		float progress = track->getRapidProgress();
		smallestProgress = std::min(smallestProgress, progress);
	}
	return smallestProgress;
}

//OK
bool Manoeuvre::isRapidFinished(){
	return getRapidProgress() >= 1.0;
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

double Manoeuvre::getDuration(){
	switch(getType()){
		case ManoeuvreType::KEY: return 0.0;
		case ManoeuvreType::TARGET:
		case ManoeuvreType::SEQUENCE: return duration_seconds;
	}
}




float Manoeuvre::getPlaybackProgress(){ return playbackPosition_seconds / duration_seconds; }
bool Manoeuvre::isPlaybackFinished(){ return playbackPosition_seconds >= duration_seconds;  }

void Manoeuvre::incrementPlaybackPosition(long long playbackTime_microseconds){
	if(isPaused()) return;
	playbackPosition_seconds = (playbackTime_microseconds - playbackStartTime_microseconds) / 1000000.0;
	for(auto& track : tracks) track->setPlaybackPosition(playbackPosition_seconds);
}

void Manoeuvre::updatePlaybackStatus(){
	for(auto& track : tracks) track->updatePlaybackStatus();
	
	if(isPlaying() && isPlaybackFinished()) {
		//needs to call onPlaybackEnd in machines
		b_playing = false;
		playbackPosition_seconds = 0.0;
		for(auto& track : tracks) track->setPlaybackPosition(0.0);
	}
	else if(isInRapid() && isRapidFinished()) b_inRapid = false;
}







