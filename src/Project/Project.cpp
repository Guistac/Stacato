#include <pch.h>

#include "Project.h"

#include "Project/Environnement.h"
#include "Project/Plot.h"
#include "Motion/Manoeuvre/Manoeuvre.h"
#include "Fieldbus/EtherCatFieldbus.h"

namespace Project {

	std::vector<std::shared_ptr<Plot>> plots;
	std::shared_ptr<Plot> currentPlot;

	char projectDirectory[512] = "";

	bool hasDefaultSavePath() {
		return strcmp(projectDirectory, "") != 0;
	}

	bool load(const char* dir) {

		strcpy(projectDirectory, dir);

#ifndef MACOS
		std::string projectFolderPath = std::string(dir) + "/";
		if (!std::filesystem::is_directory(projectFolderPath)) return false;

		for (const auto& entry : std::filesystem::directory_iterator(projectFolderPath)) {
			if (entry.path().extension() == ".stacato") {
				Environnement::load(entry.path().generic_u8string().c_str());
				break;
			}
		}
        
		std::string stageFolderPath = projectFolderPath + "Stage/";
		if (!std::filesystem::exists(std::filesystem::path(stageFolderPath))) {}

		std::string plotsFolderPath = projectFolderPath + "Plots/";
        if (std::filesystem::exists(std::filesystem::path(plotsFolderPath))) {
            for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(plotsFolderPath))) {
                if (entry.path().extension() == ".plot") {
                    std::shared_ptr<Plot> plot = std::make_shared<Plot>();
                    if (plot->load(entry.path().u8string().c_str())) {
                        plots.push_back(plot);
                    }
                }
            }
        }
#else
      
        std::string projectFolderPath = std::string(dir) + "/";
        std::string environnementFilePath = projectFolderPath + "Environnement.stacato";
        if(!Environnement::load(environnementFilePath.c_str())) Logger::warn("Could not load Environnement.stacato");
        else Logger::info("Loaded Environnement.stacato");
        
        std::string plotsFolderPath = projectFolderPath + "Plots/";
        std::string plotFilePath = plotsFolderPath + "Default_Plot.plot";
        
        std::shared_ptr<Plot> plot = std::make_shared<Plot>();
        if(plot->load(plotFilePath.c_str())){
            plots.push_back(plot);
            Logger::info("Loaded Default Plot");
        }else{
            Logger::warn("Failed to load Default Plot");
        }
        
#endif
		if (plots.empty()) {
			plots.push_back(std::make_shared<Plot>());
			strcpy(plots.back()->name, "Default Plot");
		}
		currentPlot = plots.back();

		return true;
	}

	bool reload() {
		if(hasDefaultSavePath()) return load(projectDirectory);
		return false;
	}


	bool saveAs(const char* dir) {

		strcpy(projectDirectory, dir);
#ifndef MACOS
		std::filesystem::path directory = dir;
		if (!std::filesystem::exists(directory)) std::filesystem::create_directory(directory);

		std::string projectFolderPath = directory.generic_u8string() + "/";

		std::string environnementFilePath = projectFolderPath + "Environnement.stacato";
		if (!Environnement::save(environnementFilePath.c_str())) return false;

		std::string stageFolder = projectFolderPath + "Stage/";
		if(!std::filesystem::exists(std::filesystem::path(stageFolder))) std::filesystem::create_directory(std::filesystem::path(stageFolder));

		std::string plotsFolder = projectFolderPath + "Plots/";
		if (!std::filesystem::exists(std::filesystem::path(plotsFolder))) std::filesystem::create_directory(std::filesystem::path(plotsFolder));

		for (auto plot : plots) {
			std::string plotFilePath = plotsFolder + plot->name  + "_" + std::to_string(plot->saveTime) + ".plot";
			plot->save(plotFilePath.c_str());
		}
#else
      
        if(Environnement::save("TestFile.HAHAHAHAHAHA")) Logger::critical("TEST OK");
        
        std::string projectFolderPath = "Project/";
        std::string environnementFilePath = projectFolderPath + "Environnement.stacato";
        if(!Environnement::save(environnementFilePath.c_str())) Logger::warn("Could not save Environnement.stacato");
        else Logger::info("Saved Environnement.stacato");
        
        std::string plotFolderPath = projectFolderPath + "Plots/";
        std::string plotFilePath = plotFolderPath + "Default_Plot.plot";
        if(plots.front()->save(plotFilePath.c_str())){
            Logger::info("Saved Plot File");
        }else{
            Logger::warn("Failed To Save Plot File");
        }
        
#endif
		return true;
	}

	bool save() {
		if(hasDefaultSavePath()) return saveAs(projectDirectory);
		return false;
	}

	void createNew() {
		Environnement::createNew();
		plots.clear();
		currentPlot = nullptr;
		currentPlot = std::make_shared<Plot>();
		strcpy(currentPlot->name, "Default Plot");
		plots.push_back(currentPlot);
		strcpy(projectDirectory, "");
	}

	bool isEditingAllowed() {
		return !EtherCatFieldbus::isCyclicExchangeActive();
	}

};
