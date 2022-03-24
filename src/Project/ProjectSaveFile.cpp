#include <pch.h>

#include "Project.h"

#include "Environnement/Environnement.h"
#include "Plot/Plot.h"
#include "Gui/ApplicationWindow/ApplicationWindow.h"

namespace Project{

	char saveFilePath[512];
	bool b_hasFilePath;

	bool b_hasUnsavedModifications;

	bool b_newProjectRequested = false;
	char newProjectFilePath[512];

	bool isNewProjectRequested(){ return b_newProjectRequested; }

	void confirmNewProjectRequest(){
		if(!b_newProjectRequested) return;
		Environnement::stop();
		b_hasUnsavedModifications = false;
		if(newProjectFilePath[0] == 0) createNew();
		else load(newProjectFilePath);
		cancelNewProjectRequest();
	}

	void cancelNewProjectRequest(){
		b_newProjectRequested = false;
		newProjectFilePath[0] = 0;
	}

	void createNew() {
		if(!canCloseImmediately()){
			newProjectFilePath[0] = 0;
			b_newProjectRequested = true;
			Logger::info("Trying to creat new project while current one has modifications or is running");
			return false;
		}
		Logger::info("Creating new project");
		Environnement::createNew();
		plots.clear();
		currentPlot = nullptr;
		currentPlot = std::make_shared<Plot>();
		strcpy(currentPlot->name, "Default Plot");
		plots.push_back(currentPlot);
		saveFilePath[0] = 0;
		b_hasFilePath = false;
		ApplicationWindow::hideUnsavedModifications();
		ApplicationWindow::setWindowName("New Project");
		b_hasUnsavedModifications = false;
	}

	void loadStartup(){
		//check if something the app was launched by double clicking a save file
		//else check if the last loaded project exists
		//else just create an empty project
		if(ApplicationWindow::wasLaunchedByOpeningFile()){
			const char* path = ApplicationWindow::getOpenedFilePath();
			Logger::info("App launched by opening file : {}", path);
			if(load(path)) return;
			else createNew();
		}else{
			createNew();
		}
	}

	bool reloadSaved() {
		if(b_hasFilePath) return load(saveFilePath);
		return false;
	}

	bool load(const char* dir) {

		std::filesystem::path filePath(dir);
		
		if(filePath.extension().string() != ".stacato"){
			Logger::warn("File {} could not be loaded, wrong file extension", filePath.string());
			return false;
		}
		
		if(!canCloseImmediately()){
			strcpy(newProjectFilePath, dir);
			b_newProjectRequested = true;
			Logger::info("Trying to load project while current one has modifications or is running");
			return false;
		}
		
		Logger::info("Loading Project File {}", filePath.filename().string());
		
		std::string projectFolderPath = std::string(dir) + "/";
		if (!std::filesystem::is_directory(projectFolderPath)) return false;

		//look for the environnement file
		bool b_loadedEnvironnementFile = false;
		for (const auto& entry : std::filesystem::directory_iterator(projectFolderPath)) {
			if(entry.path().filename() == "Environnement.stacatoEnvironnement"){
				b_loadedEnvironnementFile = Environnement::load(entry.path().generic_u8string().c_str());
				break;
			}
		}
		if(!b_loadedEnvironnementFile) {
			Logger::warn("Could not load environnement file in project {}", filePath.filename().string());
			return false;
		}
		
		//look for the stage folder
		std::string stageFolderPath = projectFolderPath + "Stage/";
		if (!std::filesystem::exists(std::filesystem::path(stageFolderPath))) {
			Logger::warn("Could not find Stage Folder in project {}", filePath.filename().string());
			return false;
		}

		//look for the plot folder
		std::string plotsFolderPath = projectFolderPath + "Plots/";
		if (!std::filesystem::exists(std::filesystem::path(plotsFolderPath))) {
			Logger::warn("Could not find Plot Folder in project {}", filePath.filename().string());
			return false;
		}
		
		//load plot files
		for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(plotsFolderPath))) {
			if (entry.path().extension() == ".stacatoPlot") {
				std::shared_ptr<Plot> plot = std::make_shared<Plot>();
				if (plot->load(entry.path().u8string().c_str())) {
					plots.push_back(plot);
				}
			}
		}
		if (plots.empty()) {
			plots.push_back(std::make_shared<Plot>());
			strcpy(plots.back()->name, "Default Plot");
		}
		currentPlot = plots.back();

		strcpy(saveFilePath, dir);
		b_hasFilePath = true;
		b_hasUnsavedModifications = false;
		
		ApplicationWindow::hideUnsavedModifications();
		ApplicationWindow::setWindowName(filePath.filename().replace_extension("").string().c_str());
		
		Logger::info("Loaded File {}", filePath.filename().string());
		
		return true;
	}





	bool save() {
		if(b_hasFilePath) {
			bool b_saved = saveAs(saveFilePath);
			if(b_saved) ApplicationWindow::hideUnsavedModifications();
			return b_saved;
		}
		return false;
	}

	bool hasFilePath(){ return b_hasFilePath; }

	bool saveAs(const char* dir) {

		std::filesystem::path filePath = dir;
		if(filePath.extension() != ".stacato") filePath.replace_extension(".stacato");
		
		if (!std::filesystem::exists(filePath)) std::filesystem::create_directory(filePath);
		std::string projectFolderPath = filePath.string() + "/";

		std::string environnementFilePath = projectFolderPath + "Environnement.stacatoEnvironnement";
		if (!Environnement::save(environnementFilePath.c_str())) return false;

		std::string stageFolder = projectFolderPath + "Stage/";
		if(!std::filesystem::exists(std::filesystem::path(stageFolder))) std::filesystem::create_directory(std::filesystem::path(stageFolder));

		std::string plotsFolder = projectFolderPath + "Plots/";
		if (!std::filesystem::exists(std::filesystem::path(plotsFolder))) std::filesystem::create_directory(std::filesystem::path(plotsFolder));

		for (int i = 0; i < plots.size(); i++) {
			std::shared_ptr<Plot> plot = plots[i];
			std::string plotFilePath = plotsFolder + plot->name + "_" + std::to_string(i) + ".stacatoPlot";
			plot->save(plotFilePath.c_str());
		}
		
		strcpy(saveFilePath, dir);
		b_hasFilePath = true;
		b_hasUnsavedModifications = false;
		
		ApplicationWindow::hideUnsavedModifications();
		ApplicationWindow::setWindowName(filePath.filename().replace_extension("").string().c_str());
		
		return true;
	}



	void setModified(){
		b_hasUnsavedModifications = true;
		ApplicationWindow::showUnsavedModifications();
	}

	bool hasUnsavedModifications(){
		return b_hasUnsavedModifications;
	}

}







