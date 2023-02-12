#include <pch.h>

#include "StacatoEditor.h"
#include "Workspace/Workspace.h"



namespace StacatoEditor{

std::shared_ptr<StacatoProject> currentProject;

bool hasCurrentProject(){ return currentProject != nullptr; }

std::shared_ptr<StacatoProject> getCurrentProject(){ return currentProject; }

void openProject(std::shared_ptr<StacatoProject> project){
	currentProject = project;
}

void closeCurrentProject(){
	currentProject = nullptr;
}

void createNewProject(){
	if(currentProject->canClose()){
		closeCurrentProject();
		auto newProject = StacatoProject::createInstance();
		openProject(newProject);
		Workspace::addFile(newProject);
	}
}

};
