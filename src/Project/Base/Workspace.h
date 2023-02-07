#pragma once

namespace Workspace{

	std::vector<std::shared_ptr<Project>>& getProjects(){
		static std::vector<std::shared_ptr<Project>> projects;
		return projects;
	}

	void openFile(std::filesystem::path file){
		//check if extension matches any accepted file type
		//load project and add to project list
	}

}
