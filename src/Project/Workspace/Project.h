#pragma once

#include "Component.h"

/*
 DESCRIPTION
 
 Project is a top level Component, it contains all data that is associated with a save file
 It tracks all user actions on the save file and allows undo redo operations
 it also knows if there are unsaved modifications
 
 IMPLEMENTATION EXAMPLE
 
class ProjectImplementation : public Project{
	DECLARE_PROTOTYPE_INTERFACE_METHODS(ProjectImplementation)
 
protected:
	 
	virtual bool onSerialization() override {
		bool success = true;
		success &= Component::onSerialization();
		//save project content
		return success;
	}
	 
	virtual bool onDeserialization() override {
		bool success = true;
		success &= Component::onDeserialization();
		//load project content
		return success;
	}
	 
	virtual void onConstruction() override {
		Component::onConstruction();
		//initialize object
	}
	 
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Component::onCopyFrom(source);
		//copy project content
	}
 
	virtual bool canClose() override {
		return true;
	}
 
	virtual void onOpen() override {
		//initialize some project variables?
	}
 
	virtual void onClose() override {
		//deinitialize some project variables?
	};
 
 };
 
 */

class UndoableAction;

class NewProject : public Component{
public:
	
	virtual bool canClose() = 0;
	
	void close(){
		onOpen();
	}
	void open(){
		onClose();
	}
	
	void registerAction(std::shared_ptr<UndoableAction> action);
	
	bool canUndo();
	void undo();
	
	bool canRedo();
	void redo();
	
	std::shared_ptr<UndoableAction> getLastAction();
	std::vector<std::shared_ptr<UndoableAction>> getActionList();
	
	bool hasUnsavedModifications();
	
protected:
	
	virtual void onOpen() = 0;
	virtual void onClose() = 0;
	
	virtual void setParent(std::shared_ptr<Component> parent_) override {
		Logger::error("Cannot set parent object of Project {}", getName());
	}
	
	virtual void setProject(std::shared_ptr<ProjectComponent> project_) override {
		Logger::error("Cannot set project of Project {}", getName());
	}
	
private:
	
	std::shared_ptr<UndoableAction> actionHistory;
	
};
