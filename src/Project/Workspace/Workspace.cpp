#include <pch.h>

#include "Workspace.h"

namespace Workspace{

	std::vector<std::shared_ptr<NewProject>> projects;
	std::shared_ptr<NewProject> currentProject = nullptr;

	const std::vector<std::shared_ptr<NewProject>>& getProjects(){ return projects; }

	void addProject(std::shared_ptr<NewProject> project){
		projects.push_back(project);
	}

	void removeProject(std::shared_ptr<NewProject> project){
		if(project == currentProject){
			Logger::warn("[Workspace] Cannot remove current project");
			return;
		}
		for(size_t i = projects.size() - 1; i >= 0; i--){
			if(project == projects[i]){
				projects.erase(projects.begin() + i);
				break;
			}
		}
	}

	bool hasProject(std::shared_ptr<NewProject> project){
		std::filesystem::path queriedPath = project->getFilePath();
		for(auto listedProject : projects){
			if(listedProject->getFilePath() == queriedPath) return true;
		}
		return false;
	}

	std::shared_ptr<NewProject> getCurrentProject(){ return currentProject; }
   
	bool hasCurrentProject(){ return getCurrentProject() != nullptr; }
   
	bool openProject(std::shared_ptr<NewProject> project){
		//add the project to the list if it isn't in there yet
		if(!hasProject(project)) addProject(project);
		//if the project can be opened
		if(hasCurrentProject() && !getCurrentProject()->canClose()){
			Logger::warn("[Workspace] Could not open project : current project must be closed first");
			return false;
		}else{
			//close current project
			if(hasCurrentProject()) getCurrentProject()->close();
			//make new project current
			currentProject = project;
			project->open();
			return true;
		}
	}

	std::function<bool(std::filesystem::path)> openFileCallback;

	bool openFile(std::filesystem::path path){
		return openFileCallback(path);
	}

	void setFileOpenCallback(std::function<bool(std::filesystem::path)> callback){
		openFileCallback = callback;
	}
   

};
