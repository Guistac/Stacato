#pragma once

class Command{
public:
	virtual void execute() = 0;
	virtual void undo() = 0;
	std::string name;
};

namespace CommandHistory{

	inline std::vector<std::shared_ptr<Command>>& get(){
		static std::vector<std::shared_ptr<Command>> history;
		return history;
	}

	inline size_t& getUndoableCommandCount(){
		static size_t undoableCommandCount = 0;
		return undoableCommandCount;
	}

	inline void push(std::shared_ptr<Command> command){
		if(getUndoableCommandCount() < get().size()) get().erase(get().begin() + getUndoableCommandCount(), get().end());
		get().push_back(command);
		getUndoableCommandCount()++;
	}

	inline void undo(){
		getUndoableCommandCount()--;
		get()[getUndoableCommandCount()]->undo();
	}

	inline void redo(){
		get()[getUndoableCommandCount()]->execute();
		getUndoableCommandCount()++;
	}

	inline bool canUndo(){
		return getUndoableCommandCount() > 0;
	}

	inline bool canRedo(){
		return getUndoableCommandCount() < get().size();
	}

};
