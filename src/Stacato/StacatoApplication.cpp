#include <pch.h>

#include "StacatoApplication.h"

#include "config.h"

#include "Nodes/NodeFactory.h"
#include "Environnement/Environnement.h"
#include "Console/ConsoleHandler.h"

#include "Legato/Workspace.h"

#include "Project/StacatoProject.h"

#include "StacatoEditor.h"
#include "Utilities/Timing.h"


namespace Stacato::Application{



bool initialize(std::filesystem::path launchPath){
	
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
	
	Stacato::Editor::initialize();
	
	#if defined(STACATO_DEBUG)
		Stacato::Editor::unlock();
	#else
		Stacato::Editor::lock();
	#endif

	bool b_launchPathLoaded;

	if(launchPath.empty()) b_launchPathLoaded = false;
	else b_launchPathLoaded = ::Workspace::openFile(launchPath);
	
	if(!b_launchPathLoaded){
		#ifdef STACATO_DEBUG
			Logger::info("Loading debug project");
			std::filesystem::path debugFilePath = "DebugProject.stacato";
			if(!::Workspace::openFile(debugFilePath)) {
				Logger::info("Could not load debug project, loading empty project");
				Stacato::Editor::createNewProject();
			}
		#else
			Logger::info("Loading empty project");
			Stacato::Editor::createNewProject();
		#endif
	}
	 
	return true;
}



bool terminate(){
	
	Stacato::Editor::terminate();
	
	//terminate serial communications
	ConsoleHandler::terminate();
	
	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();
	
	return true;
}


bool requestQuit(){
	if(!Stacato::Editor::hasCurrentProject()) return true;
	return Stacato::Editor::getCurrentProject()->canClose();
}

};

