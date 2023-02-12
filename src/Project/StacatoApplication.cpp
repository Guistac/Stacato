#include <pch.h>

#include "Stacato.h"

#include "config.h"

#include "Nodes/NodeFactory.h"
#include "Environnement/Environnement.h"
#include "Console/ConsoleHandler.h"

#include "StacatoEditor.h"

#include "Workspace/Workspace.h"

#include "StacatoProject.h"
#include "StacatoEditor.h"

namespace Stacato::Application{



bool initialize(){
	
	#ifdef STACATO_MACOS
	//we delete this file on each startup since we don't use it and it sometimes causes loading problems with imguinodeeditor
	std::filesystem::path nodeEditorJsonFile = "NodeEditor.json";
	std::filesystem::remove(nodeEditorJsonFile);
	#endif
	
	//initialize node factory modules
	NodeFactory::load();
	
	//load network interfaces, initialize networking, open ethercat network interface
	Environnement::initialize();
	
	//start looking for consoles, and load profile for previously connected ones
	ConsoleHandler::initialize();
	
	//for debug builds, always try to load the debug project in the debug directory
	#ifdef STACATO_DEBUG
	std::filesystem::path debugFilePath = "DebugProject.stacato";
	Workspace::openFile(debugFilePath);
	#endif
	
}



bool terminate(){
	
	//terminate serial communications
	ConsoleHandler::terminate();
	
	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();
	
}


std::shared_ptr<File> openFile(std::filesystem::path path){

	if(!path.has_filename()){
		Logger::error("[Stacato Workspace] Could not open file : Path has no File Name");
		return nullptr;
	}
	
	if(!path.has_extension()){
		Logger::error("[Stacato Workspace] Could not open file : File has no extension");
		return nullptr;
	}
	
	std::string fileName = path.filename().string();
	std::string fileExtension = path.extension().string();
	
	if(fileExtension == ".stacato"){
		auto loadedProject = StacatoProject::createInstance();
		loadedProject->setFilePath(path);
		if(!loadedProject->readFile()){
			Logger::error("[Stacato Workspace] Failed to open stacato project {}", fileName);
			return nullptr;
		}
		StacatoEditor::openProject(loadedProject);
		return loadedProject;
	}
	
	Logger::error("[Stacato Workspace] Could not open file '{}' : Unsupported file type", fileName);
	return nullptr;
}


};

