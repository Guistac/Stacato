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

#include "Fieldbus/EtherCatFieldbus.h"


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
		Stacato::Editor::unlock();
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
	
	
	
	
	std::thread environnementAutoStart = std::thread([](){
		//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		Timing::Timer watchdog;
		watchdog.setExpirationSeconds(2.0);
		while(!Environnement::isReady()){
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if(watchdog.isExpired()){
				Logger::warn("Environnement autosetup timed out");
				return;
			}
		}
		Logger::info("Autostarting Environnement");
		Environnement::start(false); //start without opening the popup

		if(!EtherCatFieldbus::b_autoEnable) return;
		
		watchdog.setExpirationSeconds(10.0);
		while(!Environnement::isRunning()){
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if(watchdog.isExpired()){
				Logger::warn("Environnement autostart timed out");
				return;
			}
		}
		Logger::info("Environnement autostart fnished");

		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		watchdog.setExpirationSeconds(10.0);
		while(!Environnement::areAllMachinesEnabled()){
			Environnement::enableAllMachines();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if(watchdog.isExpired()){
				Logger::warn("Machine autoenable timed out");
				return;
			}
		}

		Logger::info("Machine autoenable finished");
	});
	environnementAutoStart.detach();

	 
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

