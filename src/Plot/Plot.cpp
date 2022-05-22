#include <pch.h>

#include "Plot.h"

#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

void Plot::selectManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre) {
	selectedManoeuvre = manoeuvre;
}

std::shared_ptr<Manoeuvre> Plot::getSelectedManoeuvre() {
	return selectedManoeuvre;
}


//————————————————————————————————
//			Add Manoeuvre
//————————————————————————————————

class AddManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<Plot> plot;
	std::shared_ptr<Manoeuvre> addedManoeuvre;
	int addedIndex;
	ManoeuvreType type;
	
	AddManoeuvreCommand(std::string& name, std::shared_ptr<Plot> plot_, ManoeuvreType type_) : Command(name){
		plot = plot_;
		type = type_;
	}
	
	virtual void execute(){
		addedManoeuvre = std::make_shared<Manoeuvre>();
		addedManoeuvre->overwriteType(type);
		addedManoeuvre->setDescription("");
		std::string manoeuvreName = "M-" + std::to_string(plot->getManoeuvres().size());
		addedManoeuvre->setName(manoeuvreName);
		
		if(plot->getSelectedManoeuvre()){
			auto selected = plot->getSelectedManoeuvre();
			auto& manoeuvres = plot->getManoeuvres();
			for (int i = 0; i < manoeuvres.size(); i++) {
				if (manoeuvres[i] == selected) {
					addedIndex = i + 1;
					manoeuvres.insert(manoeuvres.begin() + i + 1, addedManoeuvre);
					break;
				}
			}
		}else {
			addedIndex = plot->getManoeuvres().size();
			plot->getManoeuvres().push_back(addedManoeuvre);
		}
		plot->selectManoeuvre(addedManoeuvre);
		addedManoeuvre->subscribeAllTracksToMachineParameter();
	}
	
	virtual void undo(){
		auto& manoeuvres = plot->getManoeuvres();
		for(int i = 0; i < manoeuvres.size(); i++){
			if(manoeuvres[i] == addedManoeuvre){
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
		addedManoeuvre->unsubscribeAllTracksFromMachineParameter();
	}
	virtual void redo(){
		auto& manoeuvres = plot->getManoeuvres();
		manoeuvres.insert(manoeuvres.begin() + addedIndex, addedManoeuvre);
		plot->selectManoeuvre(addedManoeuvre);
		addedManoeuvre->subscribeAllTracksToMachineParameter();
	}
};

void Plot::addManoeuvre() {
	std::string name = "Add Manoeuvre";
	auto thisPlot = shared_from_this();
	auto command = std::make_shared<AddManoeuvreCommand>(name, thisPlot, ManoeuvreType::KEY);
	CommandHistory::pushAndExecute(command);
}




//————————————————————————————————
//			Delete Manoeuvre
//————————————————————————————————


class DeleteManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<Plot> plot;
	std::shared_ptr<Manoeuvre> deletedManoeuvre;
	int deletedIndex;
	
	DeleteManoeuvreCommand(std::string name, std::shared_ptr<Plot> plot_, std::shared_ptr<Manoeuvre> deleted_) : Command(name){
		plot = plot_;
		deletedManoeuvre = deleted_;
	}
	
	virtual void execute(){
		auto& manoeuvres = plot->getManoeuvres();
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == deletedManoeuvre) {
				deletedIndex = i;
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
		deletedManoeuvre->unsubscribeAllTracksFromMachineParameter();
	}
	virtual void undo(){
		auto& manoeuvres = plot->getManoeuvres();
		manoeuvres.insert(manoeuvres.begin() + deletedIndex, deletedManoeuvre);
		plot->selectManoeuvre(deletedManoeuvre);
		deletedManoeuvre->subscribeAllTracksToMachineParameter();
	}
};

void Plot::deleteSelectedManoeuvre() {
	if (getSelectedManoeuvre() != nullptr) {
		std::string name = "Delete Manoeuvre " + std::string(selectedManoeuvre->getName());
		auto command = std::make_shared<DeleteManoeuvreCommand>(name, shared_from_this(), getSelectedManoeuvre());
		CommandHistory::pushAndExecute(command);
	}
}




//————————————————————————————————
//		Duplicate Manoeuvre
//————————————————————————————————

class DuplicateManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<Plot> plot;
	std::shared_ptr<Manoeuvre> original;
	std::shared_ptr<Manoeuvre> copy;
	int insertIndex;
	
	DuplicateManoeuvreCommand(std::string& name, std::shared_ptr<Manoeuvre> original_, std::shared_ptr<Plot> plot_) : Command(name){
		original = original_;
		plot = plot_;
	}
	
	virtual void execute(){
		copy = original->copy();
		copy->validateAllParameterTracks();
		auto& manoeuvres = plot->getManoeuvres();
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == original) {
				manoeuvres.insert(manoeuvres.begin() + i + 1, copy);
				insertIndex = i + 1;
				break;
			}
		}
	}
	virtual void undo(){
		auto& manoeuvres = plot->getManoeuvres();
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == copy) {
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
	}
	virtual void redo(){
		auto& manoeuvres = plot->getManoeuvres();
		manoeuvres.insert(manoeuvres.begin() + insertIndex, copy);
	}
};


void Plot::duplicateSelectedManoeuvre() {
	if (getSelectedManoeuvre() != nullptr) {
		std::string name = "Duplicate Manoeuvre " + std::string(getSelectedManoeuvre()->getName());
		auto command = std::make_shared<DuplicateManoeuvreCommand>(name, getSelectedManoeuvre(), shared_from_this());
		CommandHistory::pushAndExecute(command);
	}
}




//————————————————————————————————
//			Move Manoeuvre
//————————————————————————————————

class ReorderManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<Plot> plot;
	int oldIndex;
	int newIndex;
	
	ReorderManoeuvreCommand(std::string& name, int oldIndex_, int newIndex_, std::shared_ptr<Plot> plot_) : Command(name){
		plot = plot_;
		oldIndex = oldIndex_;
		newIndex = newIndex_;
	}
	
	virtual void execute(){
		auto& manoeuvres = plot->getManoeuvres();
		auto temp = manoeuvres[oldIndex];
		manoeuvres.erase(manoeuvres.begin() + oldIndex);
		manoeuvres.insert(manoeuvres.begin() + newIndex, temp);
	}
	
	virtual void undo(){
		auto& manoeuvres = plot->getManoeuvres();
		auto temp = manoeuvres[newIndex];
		manoeuvres.erase(manoeuvres.begin() + newIndex);
		manoeuvres.insert(manoeuvres.begin() + oldIndex, temp);
	}
	
};

void Plot::reorderManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre, int newIndex) {
	std::string name = "Move Manoeuvre " + std::string(manoeuvre->getName());
	int oldIndex = getManoeuvreIndex(manoeuvre);
	auto command = std::make_shared<ReorderManoeuvreCommand>(name, oldIndex, newIndex, shared_from_this());
	CommandHistory::pushAndExecute(command);
}








int Plot::getManoeuvreIndex(std::shared_ptr<Manoeuvre> manoeuvre){
	for(int i = 0; i < getManoeuvres().size(); i++){
		if(manoeuvre == getManoeuvres()[i]) return i;
	}
	return -1;
}

void Plot::selectNextManoeuvre(){
	auto selectedManoeuvre = getSelectedManoeuvre();
	if(selectedManoeuvre){
		int nextManoeuvreIndex = getManoeuvreIndex(selectedManoeuvre) + 1;
		if(nextManoeuvreIndex >= getManoeuvres().size()) nextManoeuvreIndex = 0;
		selectManoeuvre(getManoeuvres()[nextManoeuvreIndex]);
	}else if(getManoeuvres().empty()) return;
	else selectManoeuvre(getManoeuvres().front());
}

void Plot::selectPreviousManoeuvre(){
	auto selectedManoeuvre = getSelectedManoeuvre();
	if(selectedManoeuvre){
		int previousManoeuvreIndex = getManoeuvreIndex(selectedManoeuvre) - 1;
		if(previousManoeuvreIndex <= -1) previousManoeuvreIndex = getManoeuvres().size() - 1;
		selectManoeuvre(getManoeuvres()[previousManoeuvreIndex]);
	}else if(getManoeuvres().empty()) return;
	else selectManoeuvre(getManoeuvres().back());
}




void Plot::refreshPlotAfterMachineLimitChanged(std::shared_ptr<Machine> m) {
	//find all manoeuvres that have a parametertrack for the edited machine and revalidate the curves
}


void Plot::refreshAll() {
	/*
	refreshChainingDependencies();
	for (auto& manoeuvre : manoeuvres) {
		for (auto& track : manoeuvre->tracks) {
			track->refreshAfterChainedDependenciesRefresh();
		}
	}
	 */
}
