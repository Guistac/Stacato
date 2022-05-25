#pragma once

class Command{
public:
	
	Command(std::string name_) : name(name_){}
	
	const char* getName(){ return name.c_str(); }
	
	virtual void execute() = 0;
	virtual void undo() = 0;
	virtual void redo(){ execute(); }
	
	std::vector<std::shared_ptr<Command>>& getSideEffects(){ return sideEffects; }
	void addSideEffect(std::shared_ptr<Command> sideEffect){ sideEffects.push_back(sideEffect); }
	
private:
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
