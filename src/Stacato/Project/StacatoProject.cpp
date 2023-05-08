#include <pch.h>
#include "StacatoProject.h"

#include "Plot/Plot.h"
#include "Environnement/Environnement.h"
#include "Legato/Gui/Layout.h"
#include "Visualizer/Visualizer.h"
#include "Environnement/EnvironnementScript.h"
#include "Plot/Plot.h"

void StacatoProject::onConstruction() {
	Project::onConstruction();
	
	environnement = EnvironnementObject::createInstance();
	
	plot = Plot::createInstance();
	plot->setSaveString("Plot");
	plot->setAnimatableRegistry(environnement->getAnimatableRegistry());
	
	layouts = LayoutList::createInstance();
}

void StacatoProject::onCopyFrom(std::shared_ptr<Prototype> source) {
	Project::onCopyFrom(source);
	auto original = downcasted_shared_from_this<StacatoProject>();
}

bool StacatoProject::onWriteFile() {
	
	auto path = getFilePath();
	if(path.extension() != ".stacato") path.replace_extension(".stacato");
	
	if (!std::filesystem::exists(path)) std::filesystem::create_directory(path);
	std::string projectFolderPath = filePath.string();

	std::string environnementFilePath = projectFolderPath + "/Environnement.stacatoEnvironnement";
	environnement->setFilePath(environnementFilePath);
	if(!environnement->writeFile()) return false;
	
	layouts->setFilePath(projectFolderPath + "/Layouts.stacatoLayout");
	if(!layouts->writeFile()) return false;
	
	std::string scriptsFolderPath = projectFolderPath + "/Scripts";
	if(!std::filesystem::exists(std::filesystem::path(scriptsFolderPath))) std::filesystem::create_directory(std::filesystem::path(scriptsFolderPath));

	Environnement::StageVisualizer::saveScript(scriptsFolderPath + "/VisualizerScript");
	Environnement::Script::save(scriptsFolderPath + "/EnvironnementScript");
	
	std::string plotFilePath = projectFolderPath + "/Plot.stacatoPlot";
	plot->setFilePath(plotFilePath);
	plot->writeFile();
	
	return false;
};

bool StacatoProject::onReadFile() {
	
	auto path = getFilePath();
	
	if(path.extension() != ".stacato"){
		Logger::warn("File {} could not be loaded, wrong file extension", filePath.string());
		return false;
	}
	
	std::string projectFolderPath = std::string(path);
	if (!std::filesystem::is_directory(projectFolderPath)) return false;

	//load the environnement file
	std::string environnementFilePath = projectFolderPath + "/Environnement.stacatoEnvironnement";
	environnement->setFilePath(environnementFilePath);
	if(!environnement->readFile()){
		Logger::warn("Could not load environnement file in project {}", filePath.filename().string());
		return false;
	}
	
	layouts->setFilePath(projectFolderPath + "/Layouts.stacatoLayout");
	if(!layouts->readFile()) return false;
	
	//look for the stage folder
	std::string scriptFolderPath = projectFolderPath + "/Scripts";
	if (!std::filesystem::exists(std::filesystem::path(scriptFolderPath))) {
		Logger::warn("Could not find Scripts Folder in project {}", filePath.filename().string());
		return false;
	}
	
	Environnement::StageVisualizer::loadScript(scriptFolderPath + "/VisualizerScript");
	Environnement::Script::load(scriptFolderPath + "/EnvironnementScript");
	
	std::string plotFilePath = projectFolderPath + "/Plot.stacatoPlot";
	plot->setFilePath(plotFilePath);
	plot->readFile();
	
	return true;
};

bool StacatoProject::canClose() {
	return false;
	//return !hasUnsavedModifications() && !Environnement::isRunning();
}

void StacatoProject::onOpen() {
	//initialize some project variables?
	//load window layout
}

void StacatoProject::onClose() {
	//deinitialize some project variables?
};
