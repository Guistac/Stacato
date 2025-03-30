#pragma once

#include "LegatoDirectory.h"

namespace Legato{

	class Action;

	class Project : public Directory{
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
		
		virtual void setPath(std::filesystem::path projectFilePath) override;
		
	private:
		std::vector<std::shared_ptr<Action>> actionHistory;
		size_t undoableActionCount = 0;
		std::shared_ptr<Action> currentAction = nullptr;
		
	};

}
