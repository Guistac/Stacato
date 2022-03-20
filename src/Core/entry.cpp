#include <pch.h>

#include "config.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Project/Project.h"
#include "Project/Environnement.h"

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
    //initializes application window and sets working directory
	ApplicationWindow::init();
    
	//Logger is initialized after working directory is defined to have log file access
	Logger::init();
	Logger::critical("Stacato Version {}.{} {} ({})", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME, STACATO_BUILD_TYPE);
	Logger::debug("Application Working Directory: {}", std::filesystem::current_path().string());
	
	//initialize node factory librairies
	EtherCatDeviceFactory::loadDevices();
	NodeFactory::loadNodes();
	
	//load environnement and plots, configure ethercat network interfaces
	Project::loadStartup();
	
	//load network interfaces, initialize networking, open ethercat network interface
	Environnement::initialize();

	//application gui runtime, function returns when application is quit
	ApplicationWindow::open(3500,2000);

	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();

	//shut down logger
	Logger::terminate();

	//terminate application
	ApplicationWindow::terminate();
}
