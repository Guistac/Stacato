#pragma once

#include "Project.h"

/*
 
 DESCRIPTION
 
 Workspace if the layer above the editor, it owns all projects
 the workspace handles loading files and opening projects
 it can have a single active project and switch between multiple projects
 There should be only a single workspace
 
 IMPLEMENTATION EXAMPLE
 
 class WorkspaceImplementation{
 
 protected:
 
	virtual void onOpenFile(std::filesystem::path file) override {
		//check if is file path
		//check if file extension matches project file name
		//load the project file
		//load other file types if needed
	};
 
 };
 
 */

class Workspace{
public:
	
	const std::vector<std::shared_ptr<NewProject>>& getProjects(){
		return projects;
	}
	
	bool hasOpenProject(){ return currentOpenProject != nullptr; }
	
	std::shared_ptr<NewProject>& getOpenProject(){ return currentOpenProject; }
	
	bool openProject(std::shared_ptr<NewProject> project){
		if(currentOpenProject && currentOpenProject->canClose()){
			currentOpenProject->close();
			currentOpenProject = project;
			project->open();
			return true;
		}
		return false;
	}
	
	virtual void openFile(std::filesystem::path file){
		onOpenFile(file);
	}
	
protected:
	
	virtual void onOpenFile(std::filesystem::path file) = 0;
	
private:
	
	std::vector<std::shared_ptr<NewProject>> projects;
	std::shared_ptr<NewProject> currentOpenProject = nullptr;
	
};
