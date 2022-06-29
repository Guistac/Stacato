#pragma once


class UndoableCommand : public std::enable_shared_from_this<UndoableCommand> {
public:
	
	UndoableCommand(std::string name_) : name(name_){}
	std::string& getName(){ return name; }
		
	std::vector<std::shared_ptr<UndoableCommand>>& getSideEffects(){ return sideEffects; }
	void addSideEffect(std::shared_ptr<UndoableCommand> sideEffect){ sideEffects.push_back(sideEffect); }
	
	void execute();
	
	virtual void onExecute() = 0;
	virtual void onUndo() = 0;
	virtual void onRedo(){ onExecute(); }
	
private:
	std::string name;
	std::vector<std::shared_ptr<UndoableCommand>> sideEffects;
};



namespace CommandHistory{

	bool canUndo();
	bool canRedo();
	void undo();
	void redo();

	size_t getUndoableCommandCount();
	std::vector<std::shared_ptr<UndoableCommand>>& get();
	std::shared_ptr<UndoableCommand> getUndoableCommand();
	std::shared_ptr<UndoableCommand> getRedoableCommand();

};

