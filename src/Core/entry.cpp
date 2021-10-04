#include <pch.h>

#include "Gui/Framework/GuiWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Environnement/Environnement.h"

#include <filesystem>
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

    GuiWindow::init(); //sets working directory to macos application bundle resources folder
    Timing::start();
	Logger::init();

	EtherCatDeviceFactory::loadDevices();
	NodeFactory::loadNodes();
	Environnement::load("xmlFile.xml");

	EtherCatFieldbus::updateNetworkInterfaceCardList();
	EtherCatFieldbus::init(EtherCatFieldbus::networkInterfaceCards.front());

	GuiWindow::open(3000,2000);
	GuiWindow::refresh();
	GuiWindow::close();

	EtherCatFieldbus::terminate();

	Logger::terminate();
}
