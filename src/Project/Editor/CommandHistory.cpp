#include <pch.h>
#include "CommandHistory.h"

#include "Project/Project.h"


namespace CommandHistory{
	
	std::vector<std::shared_ptr<UndoableCommand>> history;
	size_t undoableCommandCount = 0;

	std::shared_ptr<UndoableCommand> currentCommand = nullptr;

	void push(std::shared_ptr<UndoableCommand> command){
		if(currentCommand){
			Logger::trace("Executing Undoable SideEffect: {}", command->getName());
			currentCommand->addSideEffect(command);
			command->onExecute();
		}else{
			currentCommand = command;
			Logger::trace("Executing Undoable Command: {}", command->getName());
			command->onExecute();
			if(undoableCommandCount < history.size()) history.erase(history.begin() + undoableCommandCount, history.end());
			history.push_back(command);
			undoableCommandCount++;
			currentCommand = nullptr;
		}
	}

	bool canUndo(){ return undoableCommandCount > 0; }

	bool canRedo(){ return undoableCommandCount < history.size(); }

	void undo(){
		if(!canUndo()) return;
		undoableCommandCount--;
		
		//undo all side effects in reverse order, the undo command
		auto command = history[undoableCommandCount];
		for(int i = command->getSideEffects().size() - 1; i >= 0; i--) {
			auto sideEffect = command->getSideEffects()[i];
			Logger::trace("Undoing SideEffect: {}", sideEffect->getName());
			sideEffect->onUndo();
		}
		Logger::trace("Undoing Command: {}", command->getName());
		command->onUndo();
		
		Project::setModified();
	}

	void redo(){
		if(!canRedo()) return;
		
		//redo command, the redo all side effects in order
		auto command = history[undoableCommandCount];
		Logger::trace("Redoing Command: {}", command->getName());
		command->onRedo();
		for(int i = 0; i < command->getSideEffects().size(); i++) {
			auto sideEffect = command->getSideEffects()[i];
			Logger::trace("Redoing SideEffect: {}", sideEffect->getName());
			sideEffect->onRedo();
		}
		
		
		undoableCommandCount++;
		Project::setModified();
	}


	std::shared_ptr<UndoableCommand> getUndoableCommand(){
		if(canUndo()) return history[undoableCommandCount - 1];
		return nullptr;
	}

	std::shared_ptr<UndoableCommand> getRedoableCommand(){
		if(canRedo()) return history[undoableCommandCount];
		return nullptr;
	}

	std::vector<std::shared_ptr<UndoableCommand>>& get(){ return history; }

	size_t getUndoableCommandCount(){ return undoableCommandCount; }

}



void UndoableCommand::execute(){
	auto thisCommand = shared_from_this();
	CommandHistory::push(thisCommand);
}
