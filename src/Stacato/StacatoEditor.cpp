#include <pch.h>

#include "StacatoEditor.h"

#include "Gui/Project/ProjectGui.h"
#include "Environnement/Environnement.h"
#include "Gui/Plot/SequencerGui.h"
#include "Gui/Environnement/EnvironnementGui.h"
#include "Gui/Plot/PlotGui.h"
#include "Gui/Environnement/Dashboard/Managers.h"
#include "Gui/Plot/PlaybackGui.h"
#include "Gui/Fieldbus/EtherCatGui.h"

#include "Legato/Gui/Gui.h"

#include "Project/StacatoProject.h"
#include "Legato/Workspace.h"

#include "Animation/ManualControlChannel.h"

class Window;

namespace Stacato::Editor{

std::vector<std::shared_ptr<Window>> userWindows;
std::vector<std::shared_ptr<Window>> administratorWindows;
std::vector<std::shared_ptr<Window>>& getUserWindows(){ return userWindows; }
std::vector<std::shared_ptr<Window>>& getAdministratorWindows(){ return administratorWindows; }

bool b_isLocked = false;

bool isLocked(){ return b_isLocked; }
void unlock(){
	b_isLocked = false;
	for(auto adminWindow : administratorWindows){
		Legato::Gui::WindowManager::registerWindow(adminWindow);
	}
}
void lock(){
	b_isLocked = true;
	for(auto adminWindow : administratorWindows){
		Legato::Gui::WindowManager::unregisterWindow(adminWindow);
		adminWindow->close();
	}
}
bool checkEditorPassword(std::string& password){ return password == "StacatoCompact"; }

void initialize(){
	//admin windows
	administratorWindows.push_back(Environnement::Gui::NodeEditorWindow::get());
	administratorWindows.push_back(Environnement::Gui::NodeManagerWindow::get());
	administratorWindows.push_back(Environnement::Gui::VisualizerScriptWindow::get());
	administratorWindows.push_back(Environnement::Gui::EtherCATWindow::get());
	administratorWindows.push_back(Environnement::Gui::ScriptEditorWindow::get());
	
	//user windows
	userWindows.push_back(Environnement::Gui::SetupWindow::get());
	userWindows.push_back(Environnement::Gui::VisualizerWindow::get());
	userWindows.push_back(PlotGui::ManoeuvreListWindow::get());
	userWindows.push_back(PlotGui::TrackSheetEditorWindow::get());
	userWindows.push_back(PlotGui::CurveEditorWindow::get());
	userWindows.push_back(DashboardWindow::get());
	userWindows.push_back(Playback::Gui::PlaybackManagerWindow::get());
	userWindows.push_back(EtherCatNetworkWindow::get());
	userWindows.push_back(EtherCatDevicesWindow::get());
	userWindows.push_back(Environnement::Gui::LogWindow::get());
	userWindows.push_back(ManualControlsWindow::get());
	
	for(auto userWindow : userWindows){
		Legato::Gui::WindowManager::registerWindow(userWindow);
	}
	unlock();
}

void terminate(){
	
}


std::shared_ptr<StacatoProject> currentProject;

std::shared_ptr<File> openFile(std::filesystem::path path){

	if(!path.has_filename()){
		Logger::error("[Stacato] Could not open file : Path has no File Name");
		return nullptr;
	}
	
	if(!path.has_extension()){
		Logger::error("[Stacato] Could not open file : File has no extension");
		return nullptr;
	}
	
	std::string fileName = path.filename().string();
	std::string fileExtension = path.extension().string();
	
	if(fileExtension == ".stacatoEnvironnement"){
		Logger::info("[Stacato] user requested reading of environnement file");
		path = path.parent_path();
		fileName = path.filename().string();
		fileExtension = path.extension().string();
		Logger::info("Project folder is {}", path.string());
	}

	if(fileExtension == ".stacato"){
		auto loadedProject = StacatoProject::createInstance();
		loadedProject->setFilePath(path);
		if(!loadedProject->readFile()){
			Logger::error("[Stacato] Failed to open stacato project {}", fileName);
			return nullptr;
		}
		
		openProject(loadedProject);
		return loadedProject;
	}
	
	Logger::error("[Stacato] Could not open file '{}' : Unsupported file type", fileName);
	return nullptr;
}

bool hasCurrentProject(){ return currentProject != nullptr; }

std::shared_ptr<StacatoProject> getCurrentProject(){ return currentProject; }

void closeCurrentProject(){
	currentProject = nullptr;
}

void openProject(std::shared_ptr<StacatoProject> project){
	currentProject = project;
}

void createNewProject(){
	if(hasCurrentProject() && !currentProject->canClose()) return;
	
	closeCurrentProject();
	auto newProject = StacatoProject::createInstance();
	openProject(newProject);
	::Workspace::addFile(newProject);
}



};
