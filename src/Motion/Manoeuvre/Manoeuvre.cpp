#include <pch.h>

#include "Manoeuvre.h"
#include "Machine/Machine.h"
#include "Machine/AnimatableParameter.h"
#include "Motion/Manoeuvre/ParameterTrack.h"
#include "Fieldbus/EtherCatFieldbus.h"

#include "Plot/Plot.h"



std::shared_ptr<Manoeuvre> Manoeuvre::copy(){
	auto copy = std::make_shared<Manoeuvre>();
	
	std::string copyName = std::string(getName()) + " (copy)";
	copy->name->overwrite(copyName);
	copy->description->overwrite(getDescription());
	copy->type->overwrite(getType());
	
	for(auto track : getTracks()){
		auto trackCopy = ParameterTrack::copy(track);
		copy->tracks.push_back(trackCopy);
	}
	
	return copy;
}



class SetManoeuvreTypeCommand : public Command{
public:
	
	Manoeuvre* manoeuvre;
	ManoeuvreType newType;
	std::vector<std::shared_ptr<ParameterTrack>> oldTracks;
	std::vector<std::shared_ptr<ParameterTrack>> newTracks;
	
	SetManoeuvreTypeCommand(std::string& name, Manoeuvre* manoeuvre_, ManoeuvreType newType_) : Command(name){
		manoeuvre = manoeuvre_;
		newType = newType_;
		oldTracks = manoeuvre->getTracks();
	}
	
	virtual void execute(){
		for(int i = 0; i < manoeuvre->getTracks().size(); i++){
			std::shared_ptr<ParameterTrack>& track = manoeuvre->getTracks()[i];
			track = ParameterTrack::create(track->getParameter(), newType);
		}
		newTracks = manoeuvre->getTracks();
	}
	virtual void undo(){
		manoeuvre->getTracks() = oldTracks;
	}
	virtual void redo(){
		manoeuvre->getTracks() = newTracks;
	}

};


void Manoeuvre::init(){
	type->setEditCallback([this](std::shared_ptr<Parameter> parameter){
		auto typeParameter = std::dynamic_pointer_cast<EnumeratorParameter<ManoeuvreType>>(parameter);
		std::string commandName = "Set Manoeuvre type to " + std::string(Enumerator::getDisplayString(this->getType()));
		auto setManoeuvreTypeCommand = std::make_shared<SetManoeuvreTypeCommand>(commandName, this, typeParameter->value);
		CommandHistory::pushAndExecute(setManoeuvreTypeCommand);
	});
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
		addedTrack = ParameterTrack::create(machineParameter, manoeuvre->getType());
		manoeuvre->getTracks().push_back(addedTrack);
	}
	
	virtual void undo(){
		auto& tracks = manoeuvre->getTracks();
		for(int i = 0; i < tracks.size(); i++){
			if(tracks[i] == addedTrack){
				tracks.erase(tracks.begin() + i);
				break;
			}
		}
	}
	
	virtual void redo(){
		manoeuvre->getTracks().push_back(addedTrack);
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
	}
	
	virtual void undo(){
		auto& tracks = manoeuvre->getTracks();
		tracks.insert(tracks.begin() + removeIndex, removedTrack);
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










bool Manoeuvre::hasTrack(std::shared_ptr<MachineParameter>& parameter) {
	for (auto& track : tracks) {
		if (track->getParameter() == parameter) return true;
	}
	return false;
}


float Manoeuvre::getPlaybackProgress() {
	/*
	float timeInManoeuvre = playbackPosition_seconds;
	float manoeuvreLength = getLength_seconds();
	float progress = timeInManoeuvre / manoeuvreLength;
	if (progress < 0.0) return 0.0;
	else if (progress > 1.0) return 1.0;
	return progress;
	 */
	return 0.0;
}





double Manoeuvre::getLength_seconds() {
	/*
	double longestTrack = 0.0;
	for (auto& track : tracks) {
		double trackTime = track->getLength_seconds();
		if (trackTime > longestTrack) {
			longestTrack = trackTime;
		}
	}
	return longestTrack;
	 */
	return 0.0;
}
