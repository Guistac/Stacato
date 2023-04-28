#include <pch.h>

#include "StacatoApplication.h"

#include "config.h"

#include "Nodes/NodeFactory.h"
#include "Environnement/Environnement.h"
#include "Console/ConsoleHandler.h"

#include "Legato/Workspace.h"

#include "Project/StacatoProject.h"

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
	
	Stacato::Editor::initialize();
	
	#if defined(STACATO_DEBUG)
	Stacato::Editor::unlock();
	#else
	Stacato::Editor::lock();
	//Stacato::Editor::unlock();
	
	#endif
	
	Stacato::Editor::createNewProject();
	
	//for debug builds, always try to load the debug project in the debug directory
	#ifdef STACATO_DEBUG
	std::filesystem::path debugFilePath = "DebugProject.stacato";
	::Workspace::openFile(debugFilePath);
	#endif
	
}



bool terminate(){
	
	Stacato::Editor::terminate();
	
	//terminate serial communications
	ConsoleHandler::terminate();
	
	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();
	
}


bool requestQuit(){
	if(!Stacato::Editor::hasCurrentProject()) return true;
	return Stacato::Editor::getCurrentProject()->canClose();
}

};

