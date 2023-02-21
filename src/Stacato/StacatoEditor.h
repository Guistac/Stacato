#pragma once

class Window;
class File;
class StacatoProject;


namespace Stacato::Editor{

enum class Mode{
	LOCKED,
	PERFORMANCE,
	DESIGN,
	EXPERT
};

void initialize();
void terminate();

bool isLocked();
void unlock();
void lock();
bool checkEditorPassword(std::string& password);

std::vector<std::shared_ptr<Window>>& getUserWindows();
std::vector<std::shared_ptr<Window>>& getAdministratorWindows();



std::shared_ptr<File> openFile(std::filesystem::path path);

bool hasCurrentProject();
std::shared_ptr<StacatoProject> getCurrentProject();
void createNewProject();
void openProject(std::shared_ptr<StacatoProject> project);
void closeCurrentProject();



};
