#include <pch.h>

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Nodes/NodeFactory.h"
#include "Project/Project.h"
#include "Environnement/Environnement.h"
#include "Console/ConsoleHandler.h"

#include <sys/resource.h>

void setProcessPriority(){
    errno = 0;
    setpriority(PRIO_PROCESS, 0, -20);
    if(errno == EACCES) Logger::info("could not set process priority : permission denied.");
    else if(errno == 0) Logger::warn("successfully set process priority");
    else Logger::info("failed to set process priority : error#{}", errno);
    int p = getpriority(PRIO_PROCESS, 0);
    Logger::info("======== Process Priority : {} (highest: -20, lowest: 20)", p);
    
}

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
    //initialize application
	ApplicationWindow::init();
    
    setProcessPriority();
		
	//initialize node factory modules
	NodeFactory::load();
	
	//load environnement and plots, configure ethercat network interfaces
	Project::loadStartup();
	
	//load network interfaces, initialize networking, open ethercat network interface
	Environnement::initialize();
	
	//start looking for consoles, and load profile for previously connected ones
	ConsoleHandler::initialize();
	
	//application gui runtime, function returns when application is quit
	ApplicationWindow::open();
	
	//terminate serial communications
	ConsoleHandler::terminate();
	
	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();

	//terminate application
	ApplicationWindow::terminate();
}
