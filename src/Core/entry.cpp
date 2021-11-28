#include <pch.h>

#include "config.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Project/Project.h"

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main() {
#endif
    
    //initializes application window and sets working directory
	ApplicationWindow::init();
    
	//Logger is initialized after working directory is defined to have log file access
	Logger::init();
#ifdef STACATO_DEBUG
	Logger::info("Stacato Version {}.{} {} - Debug Build", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME);
#else
	Logger::info("Stacato Version {}.{} {} - Release Build", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME);
#endif
	Logger::info("Application Working Directory: {}", std::filesystem::current_path().string());
    
	EtherCatDeviceFactory::loadDevices();
	NodeFactory::loadNodes();
	Project::load("Project");
	
	EtherCatFieldbus::updateNetworkInterfaceCardList();
	EtherCatFieldbus::init();

	ApplicationWindow::open(3500,2000);

	EtherCatFieldbus::terminate();

	Logger::terminate();

	ApplicationWindow::terminate();
	
}
