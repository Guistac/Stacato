#include <pch.h>
#include "StacatoProject.h"

#include "Plot/Plot.h"
#include "Environnement/Environnement.h"
#include "Legato/Gui/Layout.h"
#include "Visualizer/Visualizer.h"
#include "Environnement/EnvironnementScript.h"

void StacatoProject::onConstruction() {
	Project::onConstruction();
	
	addChild(layouts);
	
	auto defaultPlot = Plot::create();
	defaultPlot->setName("Default Plot");
	setCurrentPlot(defaultPlot);
	
	Environnement::createNew();
}

void StacatoProject::copyFrom(Ptr<Legato::Component> source) {
	Legato::Project::copyFrom(source);
	auto original = source->cast<StacatoProject>();
	
	plots.clear();
	for(auto plot : original->getPlots()){
		auto copy = plot->duplicate();
		plots.push_back(copy);
	}
}

bool StacatoProject::onSerialization() {

	std::string projectFolderPath = path.string();
	
	std::string environnementFilePath = projectFolderPath + "/Environnement.stacatoEnvironnement";
	if (!Environnement::save(environnementFilePath.c_str())) return false;

	std::string scriptsFolderPath = projectFolderPath + "/Scripts";
	if(!std::filesystem::exists(std::filesystem::path(scriptsFolderPath))) std::filesystem::create_directory(std::filesystem::path(scriptsFolderPath));

	Environnement::StageVisualizer::saveScript(scriptsFolderPath + "/VisualizerScript");
	Environnement::Script::save(scriptsFolderPath + "/EnvironnementScript");
	
	std::string plotsFolder = projectFolderPath + "/Plots";
	if (!std::filesystem::exists(std::filesystem::path(plotsFolder))) std::filesystem::create_directory(std::filesystem::path(plotsFolder));
	
	tinyxml2::XMLDocument plotListXML;
	tinyxml2::XMLElement* plotsXML = plotListXML.NewElement("Plots");
	plotListXML.InsertEndChild(plotsXML);
	
	for (int i = 0; i < getPlots().size(); i++) {
		std::shared_ptr<Plot> plot = getPlots()[i];
		std::string fileName = "Plot-" + std::to_string(i) + " " + plot->getName() + ".stacatoPlot";
		std::string plotFilePath = plotsFolder + "/" + fileName;
		plot->save(plotFilePath);
		tinyxml2::XMLElement* plotXML = plotsXML->InsertNewChildElement("Plot");
		plotXML->SetAttribute("FileName", fileName.c_str());
	}
	
	std::string plotListFilePath = plotsFolder + "/PlotList.stacatoPlotList";
	plotListXML.SaveFile(plotListFilePath.c_str());
	
	return true;
};

bool StacatoProject::onDeserialization() {
	
	std::string projectFolderPath = path.string();
	if (!std::filesystem::is_directory(projectFolderPath)) return false;

	//look for the environnement file
	bool b_loadedEnvironnementFile = false;
	for (const auto& entry : std::filesystem::directory_iterator(projectFolderPath)) {
		if(entry.path().filename() == "Environnement.stacatoEnvironnement"){
			std::string entryPath = entry.path().string();
			b_loadedEnvironnementFile = Environnement::load(entryPath.c_str());
			break;
		}
	}
	if(!b_loadedEnvironnementFile) {
		Logger::warn("Could not load environnement file in project {}", path.string());
		return false;
	}
	
	//look for the stage folder
	std::string scriptFolderPath = projectFolderPath + "/Scripts";
	if (!std::filesystem::exists(std::filesystem::path(scriptFolderPath))) {
		Logger::warn("Could not find Scripts Folder in project {}", path.string());
		return false;
	}
	
	Environnement::StageVisualizer::loadScript(scriptFolderPath + "/VisualizerScript");
	Environnement::Script::load(scriptFolderPath + "/EnvironnementScript");
	
	//look for the plot folder
	std::string plotsFolderPath = projectFolderPath + "/Plots";
	if (!std::filesystem::exists(std::filesystem::path(plotsFolderPath))) {
		Logger::warn("Could not find Plot Folder in project {}", path.string());
		return false;
	}
	
	
	std::string plotListFilePath = plotsFolderPath + "/PlotList.stacatoPlotList";
	tinyxml2::XMLDocument plotListXML;
	tinyxml2::XMLError loadResult = plotListXML.LoadFile(plotListFilePath.c_str());
	if (loadResult != tinyxml2::XML_SUCCESS) {
		Logger::warn("Could not open plot list file : {}", tinyxml2::XMLDocument::ErrorIDToName(loadResult));
		return false;
	}
	
	tinyxml2::XMLElement* plotsXML = plotListXML.FirstChildElement("Plots");
	if(plotsXML == nullptr){
		Logger::warn("could not find plot list xml element");
		return false;
	}
	
	std::vector<std::string> plotFileNames;
	
	tinyxml2::XMLElement* plotFileXML = plotsXML->FirstChildElement("Plot");
	while(plotFileXML){
		const char* plotFileName;
		if(plotFileXML->QueryAttribute("FileName", &plotFileName) != tinyxml2::XML_SUCCESS) {
			Logger::warn("could not find plot filename attribute");
			return false;
		}
		plotFileNames.push_back(std::string(plotFileName));
		plotFileXML = plotFileXML->NextSiblingElement("Plot");
	}
	
	for(auto& plotFileName : plotFileNames){
		std::string plotFilePath = plotsFolderPath + "/" + plotFileName;
		auto plot = Plot::load(plotFilePath);
		if(plot == nullptr){
			Logger::warn("failed to load plot");
			return false;
		}
		addPlot(plot);
	}
	
	if (getPlots().empty()) {
		auto defaultPlot = createNewPlot();
		defaultPlot->setName("Default Plot");
		setCurrentPlot(defaultPlot);
	}else setCurrentPlot(getPlots().front());

	return true;
};

bool StacatoProject::onPostLoad(){ return true; }

//bool StacatoProject::canClose() { return !hasUnsavedModifications() && !Environnement::isRunning(); }




void StacatoProject::setCurrentPlot(std::shared_ptr<Plot> plot){
	if(plot == nullptr) return;
	currentPlot = plot;
	plot->b_scrollToSelectedManoeuvre = true;
}

std::shared_ptr<Plot> StacatoProject::createNewPlot(){
	auto newPlot = Plot::create();
	plots.push_back(newPlot);
	currentPlot = newPlot;
	return newPlot;
}

void StacatoProject::deletePlot(std::shared_ptr<Plot> plot){
	if(plots.size() <= 1) return;
	for(int i = 0; i < plots.size(); i++){
		if(plots[i] == plot){
			plots.erase(plots.begin() + i);
			break;
		}
	}
	if(plot == currentPlot) currentPlot = plots.front();
}

void StacatoProject::deleteAllPlots(){
	plots.clear();
	currentPlot = nullptr;
}

void StacatoProject::addPlot(std::shared_ptr<Plot> plot){
	plots.push_back(plot);
}

void StacatoProject::duplicatePlot(std::shared_ptr<Plot> plot){
	auto copy = plot->duplicate();
	plots.push_back(copy);
	currentPlot = copy;
}
