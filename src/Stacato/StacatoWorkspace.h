#pragma once

class File;
class StacatoProject;

namespace Stacato::Workspace{

	std::shared_ptr<File> openFile(std::filesystem::path path);
	bool onQuitRequest();

	bool hasCurrentProject();
	std::shared_ptr<StacatoProject> getCurrentProject();
	void createNewProject();
	void openProject(std::shared_ptr<StacatoProject> project);
	void closeCurrentProject();

};
