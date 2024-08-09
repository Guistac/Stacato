#include <pch.h>

#include "Legato/Application.h"
#include "Stacato/StacatoApplication.h"
#include "Stacato/StacatoEditor.h"
#include "Stacato/StacatoGui.h"

#if defined(STACATO_WIN32_APPLICATION)
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
	//configure application
	Application::setInitializationFunction(Stacato::Application::initialize);
	Application::setTerminationFunction(Stacato::Application::terminate);
	Application::setQuitRequestFunction(Stacato::Application::requestQuit);
	Workspace::setFileOpenCallback(Stacato::Editor::openFile);
	Legato::Gui::setInitializationFunction(Stacato::Gui::initialize);
	Legato::Gui::setGuiSubmitFunction(Stacato::Gui::gui);
	Legato::Gui::setTerminationFunction(Stacato::Gui::terminate);
	
	//launch application
	Application::run(argcount, args);
}
