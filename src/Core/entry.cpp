#include <pch.h>
#include "config.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Project/Project.h"

#include <iostream>

#ifdef MACOS
int main() {
#endif

#ifdef WIN32
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
	//set working directory to "Resources" folder located next to executable
	std::string defaultWorkingDirectory = std::filesystem::current_path().string();
	std::filesystem::current_path(defaultWorkingDirectory + "/Resources");
    
#endif

	ApplicationWindow::init(); //sets working directory to macos application bundle resources folder
    
    std::cout << std::filesystem::current_path() << std::endl;
    
	Logger::init();
	Logger::info("Stacato Version {}.{}", VERSION_MAJOR, VERSION_MINOR);
	
	EtherCatDeviceFactory::loadDevices();
	NodeFactory::loadNodes();
	Project::load("Project");
	
	EtherCatFieldbus::updateNetworkInterfaceCardList();
	EtherCatFieldbus::init();

	ApplicationWindow::open(3000,2000);

	EtherCatFieldbus::terminate();

	Logger::terminate();

	ApplicationWindow::terminate();
}
