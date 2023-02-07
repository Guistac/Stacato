#pragma once

#include "Component.h"

/*
 
 
 
 */

class UndoableAction;

class Project : public Component{
public:
		
	virtual void setParent(std::shared_ptr<Component> parent_) override {
		Logger::error("Cannot set parent object of Project {}", getName());
	}
	
	virtual void setProject(std::shared_ptr<ProjectComponent> project_) override {
		Logger::error("Cannot set project of Project {}", getName());
	}
	
private:
	
	std::shared_ptr<UndoableAction> actionHistory;
	
};
