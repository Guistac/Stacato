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

#ifdef STACATO_WIN32
    //set working directory to "Resources" folder located next to executable
    std::string defaultWorkingDirectory = std::filesystem::current_path().string();
    std::filesystem::current_path(defaultWorkingDirectory + "/Resources");
#endif
    
    //for macos application bundles, this sets the working directory to the resources folder of the bundle
	ApplicationWindow::init();
    
	Logger::init();
	Logger::info("Stacato Version {}.{} {}", VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME);
    
    Logger::info("Application Directory: {}", std::filesystem::current_path().string());
    
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
