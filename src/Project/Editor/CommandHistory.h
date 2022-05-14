#pragma once

class Command{
public:
	
	
	virtual void execute() = 0;
	virtual void redo(){
		//TODO: implement extra redo method for commands that require it (for example for commands with side effects)
		execute();
	}
	virtual void undo() = 0;
	std::string name;
	
	std::vector<std::shared_ptr<Command>> sideEffects;
};

namespace CommandHistory{

	std::vector<std::shared_ptr<Command>>& get();
	size_t getUndoableCommandCount();
	void pushAndExecute(std::shared_ptr<Command> command);
	bool canUndo();
	bool canRedo();
	void undo();
	void redo();
	std::shared_ptr<Command> getUndoableCommand();
	std::shared_ptr<Command> getRedoableCommand();

};
