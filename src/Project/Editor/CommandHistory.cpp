#include <pch.h>
#include "CommandHistory.h"

#include "Project/Project.h"

namespace CommandHistory{

	std::vector<std::shared_ptr<Command>> history;
	size_t undoableCommandCount = 0;
	std::shared_ptr<Command> currentCommand = nullptr;

	std::vector<std::shared_ptr<Command>>& get(){
		return history;
	}

	size_t getUndoableCommandCount(){
		return undoableCommandCount;
	}

	void pushAndExecute(std::shared_ptr<Command> command){
		if(currentCommand == nullptr) {
			currentCommand = command;
			command->execute(); //if other commands get execute by this one, they will get added to the original commands side effects
			if(undoableCommandCount < history.size()) history.erase(history.begin() + undoableCommandCount, history.end());
			history.push_back(command);
			undoableCommandCount++;
			currentCommand = nullptr;
		}
		else{
			currentCommand->sideEffects.push_back(command);
			command->execute();
		}
		Project::setModified();
	}

	bool canUndo(){
		return undoableCommandCount > 0;
	}

	bool canRedo(){
		return undoableCommandCount < history.size();
	}

	void undo(){
		if(!canUndo()) return;
		undoableCommandCount--;
		std::shared_ptr<Command> undoneCommand = history[undoableCommandCount];
		std::vector<std::shared_ptr<Command>>& sideEffects = undoneCommand->sideEffects;
		for(int i = sideEffects.size() - 1; i >= 0; i--) sideEffects[i]->undo();
		undoneCommand->undo();
		Project::setModified();
	}

	void redo(){
		if(!canRedo()) return;
		std::shared_ptr<Command> redoneCommand = history[undoableCommandCount];
		currentCommand = redoneCommand;
		redoneCommand->redo();
		std::vector<std::shared_ptr<Command>>& sideEffects = redoneCommand->sideEffects;
		for(int i = 0; i < sideEffects.size(); i++) sideEffects[i]->redo();
		currentCommand = nullptr;
		undoableCommandCount++;
		Project::setModified();
	}

	std::shared_ptr<Command> getUndoableCommand(){
		if(canUndo()) return history[undoableCommandCount - 1];
		return nullptr;
	}

	std::shared_ptr<Command> getRedoableCommand(){
		if(canRedo()) return history[undoableCommandCount];
		return nullptr;
	}

}




