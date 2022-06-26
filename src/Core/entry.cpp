#include <pch.h>

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Nodes/NodeFactory.h"
#include "Project/Project.h"
#include "Environnement/Environnement.h"


#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
    //initialize application
	ApplicationWindow::init();
	
	//initialize node factory modules
	NodeFactory::load();
	
	//load environnement and plots, configure ethercat network interfaces
	Project::loadStartup();
	
	//load network interfaces, initialize networking, open ethercat network interface
	Environnement::initialize();
	
	//application gui runtime, function returns when application is quit
	ApplicationWindow::open();

	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();

	//terminate application
	ApplicationWindow::terminate();
}
