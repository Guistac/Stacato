#pragma once


namespace Stacato::Application{

	bool initialize();
	bool terminate();

};



namespace Stacato::Gui{

	void initialize();
	void terminate();
	void gui();

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


//Stacato::Workspace
//	Stacato::Editor
//		contains unique windows that display gui to modify the project
//		there is only one editor and only one project can be edited at one time
