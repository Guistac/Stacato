#pragma once

#include "File.h"
#include "Component.h"

/*
 DESCRIPTION
 
 Project is a top level Component, it contains all data that is associated with a save file
 It tracks all user actions on the save file and allows undo redo operations
 it also knows if there are unsaved modifications
 
 IMPLEMENTATION EXAMPLE
 
class ProjectImplementation : public NewProject{
 
 DECLARE_PROTOTYPE_IMPLENTATION_METHODS(ProjectImplementation)
 
protected:
	 
	virtual void onConstruction() override {
		Component::onConstruction();
		//initialize object
	}
	 
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		Component::onCopyFrom(source);
		//copy project content
	}
 
	virtual bool onWriteFile() override {
		return true;
	};
 
	virtual bool onReadFile() override {
		return true;
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


//TODO: components can report loading and saving errors to the project


class UndoableAction;

class Project : public Legato::Component, public File{
	
	DECLARE_PROTOTYPE_INTERFACE_METHODS(Project)
	
public:
	
	virtual bool canClose() = 0;
	
	void close(){
		Logger::info("Closing Project");
		onClose();
	}
	void open(){
		Logger::info("Opening Project");
		onOpen();
	}
	
	void registerAction(std::shared_ptr<UndoableAction> action);
	
	bool canUndo();
	void undo();
	
	bool canRedo();
	void redo();
	
	std::shared_ptr<UndoableAction> getLastAction();
	std::vector<std::shared_ptr<UndoableAction>> getActionList();
	
	bool hasUnsavedModifications(){ return false; }
	
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
	
	virtual bool onSerialization() override { return throwSerializationError(); }
	virtual bool onDeserialization() override { return throwSerializationError(); }
	virtual bool serializeIntoParent(Serializable& parent) override { return throwSerializationError(); }
	virtual bool serializeIntoParent(Serializable* parent) override { return throwSerializationError(); }
	virtual bool serializeIntoParent(std::shared_ptr<Serializable> parent) override { return throwSerializationError(); }
	virtual bool deserializeFromParent(Serializable& parent) override { return throwSerializationError(); }
	virtual bool deserializeFromParent(Serializable* parent) override { return throwSerializationError(); }
	virtual bool deserializeFromParent(std::shared_ptr<Serializable> parent) override { return throwSerializationError(); }
	
	bool throwSerializationError(){
		Logger::warn("[Project] cannot serialize or deserialize a project, use readFile() and writeFile()");
		return false;
	}
	
};

