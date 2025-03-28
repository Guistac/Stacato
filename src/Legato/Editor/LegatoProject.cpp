#include "LegatoProject.h"
#include "LegatoAction.h"

bool Legato::Project::serialize(){
	
	if(fileName.empty()){
		Logger::error("[Project] cannot serialize, filename is empty");
		return false;
	}
	
	std::filesystem::path path = getPath();
	if(!std::filesystem::exists(path)){
		Logger::debug("[Project] creating {}", path.string());
		try{
			std::filesystem::create_directory(path);
		}
		catch(std::filesystem::filesystem_error e){
			Logger::error("[Project] Could not create project folder : {}", e.code().message());
			return false;
		}
	}
	
	for(auto child : getChildren()){
		child->serialize();
	}
	
	Logger::info("[Project] Successfully serialized project {}", path.string());
	
	return true;
}

bool Legato::Project::deserialize(){
	
	if(fileName.empty()){
		Logger::error("[Project] cannot deserialize, filename is empty");
		return false;
	}
	
	std::filesystem::path path = getPath();
	if(!std::filesystem::exists(path)){
		Logger::error("[Project] Could not load, directory does not exists: {}", path.string());
		return false;
	}
	
	for(auto child : getChildren()){
		child->deserialize();
	}
	
	Logger::info("[Project] Successfully deserialized project {}", path.string());
	
}



 
 void Legato::Project::pushAction(std::shared_ptr<Action> action){
	 if(currentAction){
		 Logger::trace("Executing Undoable SideEffect: {}", action->getName());
		 currentAction->addSideEffect(action);
		 action->onExecute();
	 }else{
		 currentAction = action;
		 Logger::trace("Executing Undoable Command: {}", action->getName());
		 action->onExecute();
		 if(undoableActionCount < actionHistory.size()) actionHistory.erase(actionHistory.begin() + undoableActionCount, actionHistory.end());
		 actionHistory.push_back(action);
		 undoableActionCount++;
		 currentAction = nullptr;
	 }
 }

 bool Legato::Project::canUndo(){ return undoableActionCount > 0; }

 bool Legato::Project::canRedo(){ return undoableActionCount < actionHistory.size(); }

 void Legato::Project::undo(){
	 if(!canUndo()) return;
	 undoableActionCount--;
	 
	 //undo all side effects in reverse order, the undo command
	 auto action = actionHistory[undoableActionCount];
	 for(size_t i = action->getSideEffects().size() - 1; i >= 0; i--) {
		 auto sideEffect = action->getSideEffects()[i];
		 Logger::trace("Undoing SideEffect: {}", sideEffect->getName());
		 sideEffect->onUndo();
	 }
	 Logger::trace("Undoing : {}", action->getName());
	 action->onUndo();
	 
	 //Project::setModified();
 }

 void Legato::Project::redo(){
	 if(!canRedo()) return;
	 
	 //redo command, the redo all side effects in order
	 auto action = actionHistory[undoableActionCount];
	 Logger::trace("Redoing : {}", action->getName());
	 action->onRedo();
	 for(int i = 0; i < action->getSideEffects().size(); i++) {
		 auto sideEffect = action->getSideEffects()[i];
		 Logger::trace("Redoing SideEffect: {}", sideEffect->getName());
		 sideEffect->onRedo();
	 }
		 
	 undoableActionCount++;
	 //Project::setModified();
 }


 std::shared_ptr<Legato::Action> Legato::Project::getUndoableAction(){
	 if(canUndo()) return actionHistory[undoableActionCount - 1];
	 return nullptr;
 }

 std::shared_ptr<Legato::Action> Legato::Project::getRedoableAction(){
	 if(canRedo()) return actionHistory[undoableActionCount];
	 return nullptr;
 }




 /*
 void Legato::Command::execute(){
	 auto thisCommand = shared_from_this();
	 CommandHistory::push(thisCommand);
 }
 */

 
 
 
