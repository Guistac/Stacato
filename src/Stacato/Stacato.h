#pragma once


namespace Stacato::Application{
	bool initialize();
	bool terminate();
};



class Window;

namespace Stacato::Gui{
	void initialize();
	void terminate();
	void gui();

	std::vector<std::shared_ptr<Window>>& getUserWindows();
	std::vector<std::shared_ptr<Window>>& getAdministratorWindows();

};


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
