#pragma once

#include "LegatoFile.h"

namespace Legato{

	class Action;

	class Project : public File{
		COMPONENT_INTERFACE(Project)
	public:
		virtual bool serialize() override;
		virtual bool deserialize() override;
		
		bool canUndo();
		bool canRedo();
		void undo();
		void redo();

		size_t getUndoableCount(){ return undoableActionCount; }
		std::vector<std::shared_ptr<Action>>& getActionHistory(){ return actionHistory; }
		std::shared_ptr<Action> getUndoableAction();
		std::shared_ptr<Action> getRedoableAction();
		void execute(std::shared_ptr<Action> newAction);
		
	private:
		std::vector<std::shared_ptr<Action>> actionHistory;
		size_t undoableActionCount = 0;
		std::shared_ptr<Action> currentAction = nullptr;
		
	};

}
