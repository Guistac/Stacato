#pragma once


class Action{
public:
	virtual void undo() = 0;
	virtual void redo() = 0;
	std::string name;
};

namespace ActionHistory{

	void push(std::shared_ptr<Action> action);
	void undo();
	void redo();
	bool canUndo();
	bool canRedo();
	std::vector<std::shared_ptr<Action>>& get();
	size_t getTopIndex();
	
};



void testUndoHistory();
