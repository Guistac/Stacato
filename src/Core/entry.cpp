#include <pch.h>

#include "Legato/Application.h"
#include "Stacato/Stacato.h"

#if defined(STACATO_WIN32_APPLICATION)
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
	//configure application
	Application::setInitializationFunction(Stacato::Application::initialize);
	Application::setTerminationFunction(Stacato::Application::terminate);
	Application::setQuitRequestFunction(Stacato::Workspace::onQuitRequest);
	Workspace::setFileOpenCallback(Stacato::Workspace::openFile);
	Legato::Gui::setInitializationFunction(Stacato::Gui::initialize);
	Legato::Gui::setGuiSubmitFunction(Stacato::Gui::gui);
	Legato::Gui::setTerminationFunction(Stacato::Gui::terminate);
	
	//launch application
	Application::run();
}
