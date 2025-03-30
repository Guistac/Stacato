#include "LegatoProject.h"
#include "LegatoAction.h"

void Legato::Project::setPath(std::filesystem::path projectFilePath){
	if(projectFilePath.empty()){
		Logger::warn("[Project] path is empty");
		return;
	}
	else if(!projectFilePath.has_filename()){
		Logger::warn("[Project] '{}' path does not contain a file name", projectFilePath.string());
		return;
	}
	else path = projectFilePath;
}

bool Legato::Project::serialize(){
	
	if(path.empty()){
		Logger::error("[Project] cannot serialize, filename is empty");
		return false;
	}
	
	std::filesystem::path path = getCompletePath();
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
	
	bool b_success = true;
	b_success &= onSerialization();
	for(auto child : getChildren()){
		b_success &= child->serialize();
	}
	
	if(b_success) Logger::info("[Project] {} Saved Successfully", path.string());
	else Logger::warn("[Project] {} Saved with errors", path.string());
	
	return b_success;
}

bool Legato::Project::deserialize(){
	
	if(path.empty()){
		Logger::error("[Project] cannot deserialize, filename is empty");
		return false;
	}
	
	std::filesystem::path path = getCompletePath();
	if(!std::filesystem::exists(path)){
		Logger::error("[Project] Could not load, directory does not exists: {}", path.string());
		return false;
	}
	
	bool b_success = true;
	
	b_success &= onDeserialization();
	for(auto child : getChildren()){
		b_success &= child->deserialize();
	}
	b_success &= onPostLoad();
	
	if(b_success) Logger::info("[Project] {} loaded successfully", path.string());
	else Logger::warn("[Project] {} loaded with errors", path.string());
	
	return b_success;
}



 
 void Legato::Project::execute(std::shared_ptr<Action> action){
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
	 if(action->hasSideEffects()){
		 for(size_t i = action->getSideEffects().size() - 1; i >= 0; i--) {
			 auto sideEffect = action->getSideEffects()[i];
			 Logger::trace("Undoing SideEffect: {}", sideEffect->getName());
			 sideEffect->onUndo();
		 }
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

 
 
 
