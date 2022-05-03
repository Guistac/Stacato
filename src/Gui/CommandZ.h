#pragma once


class Command{
public:
	virtual void execute() = 0;
	virtual void undo() = 0;
	std::string name;
};

namespace CommandHistory{

	void push(std::shared_ptr<Command> command);
	void undo();
	void redo();
	bool canUndo();
	bool canRedo();

	std::vector<std::shared_ptr<Command>>& get();
	size_t getTopIndex();
};





void testUndoHistory();
