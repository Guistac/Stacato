#include <pch.h>
#include "ManoeuvreList.h"

#include "Project/Editor/CommandHistory.h"
#include "Plot/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Motion/Manoeuvre/ParameterTrack.h"

//————————————————————————————————
//			Add Manoeuvre
//————————————————————————————————

class AddManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<ManoeuvreList> manoeuvreList;
	std::shared_ptr<Manoeuvre> addedManoeuvre;
	int addedIndex;
	ManoeuvreType type;
	
	AddManoeuvreCommand(std::string& name, std::shared_ptr<ManoeuvreList> manoeuvreList_, ManoeuvreType type_) : Command(name){
		manoeuvreList = manoeuvreList_;
		type = type_;
	}
	
	virtual void execute(){
		addedManoeuvre = std::make_shared<Manoeuvre>();
		addedManoeuvre->overwriteType(type);
		addedManoeuvre->setDescription("");
		std::string manoeuvreName = "M-" + std::to_string(manoeuvreList->getManoeuvres().size());
		addedManoeuvre->setName(manoeuvreName);
		addedManoeuvre->setManoeuvreList(manoeuvreList);
		auto plot = manoeuvreList->getPlot();
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		auto selectedManoeuvre = plot->getSelectedManoeuvre();
		int insertionIndex = manoeuvres.size();
		if(selectedManoeuvre) insertionIndex = manoeuvreList->getManoeuvreIndex(selectedManoeuvre) + 1;
		manoeuvres.insert(manoeuvres.begin() + insertionIndex, addedManoeuvre);
		addedManoeuvre->subscribeAllTracksToMachineParameter();
		addedManoeuvre->updateTrackSummary();
		addedManoeuvre->select();
	}
	
	virtual void undo(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		for(int i = 0; i < manoeuvres.size(); i++){
			if(manoeuvres[i] == addedManoeuvre){
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
		addedManoeuvre->unsubscribeAllTracksFromMachineParameter();
		addedManoeuvre->deselect();
	}
	virtual void redo(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		manoeuvres.insert(manoeuvres.begin() + addedIndex, addedManoeuvre);
		addedManoeuvre->subscribeAllTracksToMachineParameter();
		addedManoeuvre->select();
	}
};

void ManoeuvreList::addManoeuvre() {
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
	
	std::shared_ptr<ManoeuvreList> manoeuvreList;
	std::shared_ptr<Manoeuvre> deletedManoeuvre;
	int deletedIndex;
	
	DeleteManoeuvreCommand(std::string name, std::shared_ptr<ManoeuvreList> manoeuvreList_, std::shared_ptr<Manoeuvre> deleted_) : Command(name){
		manoeuvreList = manoeuvreList_;
		deletedManoeuvre = deleted_;
	}
	
	virtual void execute(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		for (int i = 0; i < manoeuvres.size(); i++) {
			if (manoeuvres[i] == deletedManoeuvre) {
				deletedIndex = i;
				manoeuvres.erase(manoeuvres.begin() + i);
				break;
			}
		}
		deletedManoeuvre->unsubscribeAllTracksFromMachineParameter();
		deletedManoeuvre->deselect();
	}
	virtual void undo(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		manoeuvres.insert(manoeuvres.begin() + deletedIndex, deletedManoeuvre);
		deletedManoeuvre->subscribeAllTracksToMachineParameter();
		deletedManoeuvre->select();
	}
};

void ManoeuvreList::deleteSelectedManoeuvre() {
	auto selectedManoeuvre = getPlot()->getSelectedManoeuvre();
	if (selectedManoeuvre && selectedManoeuvre->isInManoeuvreList()) {
		std::string name = "Delete Manoeuvre " + std::string(selectedManoeuvre->getName());
		auto command = std::make_shared<DeleteManoeuvreCommand>(name, shared_from_this(), selectedManoeuvre);
		CommandHistory::pushAndExecute(command);
	}
}




//————————————————————————————————
//		Duplicate Manoeuvre
//————————————————————————————————

class DuplicateManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<ManoeuvreList> manoeuvreList;
	std::shared_ptr<Manoeuvre> original;
	std::shared_ptr<Manoeuvre> copy;
	int insertIndex;
	
	DuplicateManoeuvreCommand(std::string& name, std::shared_ptr<Manoeuvre> original_, std::shared_ptr<ManoeuvreList> manoeuvreList_) : Command(name){
		original = original_;
		manoeuvreList = manoeuvreList_;
	}
	
	virtual void execute(){
		copy = original->copy();
		copy->validateAllParameterTracks();
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		insertIndex = manoeuvreList->getManoeuvreIndex(original) + 1;
		manoeuvres.insert(manoeuvres.begin() + insertIndex, copy);
		copy->updateTrackSummary();
		copy->select();
	}
	virtual void undo(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		manoeuvres.erase(manoeuvres.begin() + insertIndex);
		copy->deselect();
	}
	virtual void redo(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		manoeuvres.insert(manoeuvres.begin() + insertIndex, copy);
		copy->select();
	}
};


void ManoeuvreList::duplicateSelectedManoeuvre() {
	auto selectedManoeuvre = getPlot()->getSelectedManoeuvre();
	if (selectedManoeuvre && selectedManoeuvre->isInManoeuvreList()) {
		std::string name = "Duplicate Manoeuvre " + std::string(selectedManoeuvre->getName());
		auto command = std::make_shared<DuplicateManoeuvreCommand>(name, selectedManoeuvre, shared_from_this());
		CommandHistory::pushAndExecute(command);
	}
}




//————————————————————————————————
//			Move Manoeuvre
//————————————————————————————————

class ReorderManoeuvreCommand : public Command{
public:
	
	std::shared_ptr<ManoeuvreList> manoeuvreList;
	int oldIndex;
	int newIndex;
	
	ReorderManoeuvreCommand(std::string& name, int oldIndex_, int newIndex_, std::shared_ptr<ManoeuvreList> manoeuvreList_) : Command(name){
		manoeuvreList = manoeuvreList_;
		oldIndex = oldIndex_;
		newIndex = newIndex_;
	}
	
	virtual void execute(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		auto temp = manoeuvres[oldIndex];
		manoeuvres.erase(manoeuvres.begin() + oldIndex);
		manoeuvres.insert(manoeuvres.begin() + newIndex, temp);
	}
	
	virtual void undo(){
		auto& manoeuvres = manoeuvreList->getManoeuvres();
		auto temp = manoeuvres[newIndex];
		manoeuvres.erase(manoeuvres.begin() + newIndex);
		manoeuvres.insert(manoeuvres.begin() + oldIndex, temp);
	}
	
};

void ManoeuvreList::moveManoeuvre(std::shared_ptr<Manoeuvre> manoeuvre, int newIndex) {
	std::string name = "Move Manoeuvre " + std::string(manoeuvre->getName());
	int oldIndex = getManoeuvreIndex(manoeuvre);
	auto command = std::make_shared<ReorderManoeuvreCommand>(name, oldIndex, newIndex, shared_from_this());
	CommandHistory::pushAndExecute(command);
}




int ManoeuvreList::getManoeuvreIndex(std::shared_ptr<Manoeuvre> manoeuvre){
	if(!manoeuvre->isInManoeuvreList()) return -1;
	for(int i = 0; i < getManoeuvres().size(); i++){
		if(manoeuvre == getManoeuvres()[i]) return i;
	}
	return -1;
}

void ManoeuvreList::selectNextManoeuvre(){
	auto selectedManoeuvre = getPlot()->getSelectedManoeuvre();
	if(selectedManoeuvre && selectedManoeuvre->isInManoeuvreList()){
		int nextManoeuvreIndex = getManoeuvreIndex(selectedManoeuvre) + 1;
		if(nextManoeuvreIndex >= getManoeuvres().size()) nextManoeuvreIndex = 0; //
		getPlot()->selectManoeuvre(manoeuvres[nextManoeuvreIndex]);
	}else if(!getManoeuvres().empty()) getPlot()->selectManoeuvre(getManoeuvres().back());
}

void ManoeuvreList::selectPreviousManoeuvre(){
	auto selectedManoeuvre = getPlot()->getSelectedManoeuvre();
	if(selectedManoeuvre && selectedManoeuvre->isInManoeuvreList()){
		int previousManoeuvreIndex = getManoeuvreIndex(selectedManoeuvre) - 1;
		if(previousManoeuvreIndex <= -1) previousManoeuvreIndex = getManoeuvres().size() - 1;
		getPlot()->selectManoeuvre(manoeuvres[previousManoeuvreIndex]);
	}else if(!getManoeuvres().empty()) getPlot()->selectManoeuvre(getManoeuvres().front());
}

