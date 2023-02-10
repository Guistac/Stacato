#pragma once

#include "Project.h"

/*
 
 DESCRIPTION
 
 Workspace if the layer above the editor, it owns all projects
 the workspace handles loading files and opening projects
 it can have a single active project and switch between multiple projects
 There should be only a single workspace

 };
*/

namespace Workspace{

	const std::vector<std::shared_ptr<NewProject>>& getProjects();
	bool hasProject(std::shared_ptr<NewProject> project);
	void addProject(std::shared_ptr<NewProject> project);
	void removeProject(std::shared_ptr<NewProject> project);

	bool hasCurrentProject();
	std::shared_ptr<NewProject> getCurrentProject();

	bool openProject(std::shared_ptr<NewProject> project);
	
	bool openFile(std::filesystem::path file);
	void setFileOpenCallback(std::function<bool(std::filesystem::path)> callback);

};

