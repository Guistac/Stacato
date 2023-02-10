#include <pch.h>

#include "Project/Workspace/Application.h"
#include "Project/Stacato.h"

#if defined(STACATO_WIN32_APPLICATION)
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
	//configure application
	Application::setInitializationFunction(Stacato::Application::initialize);
	Application::setTerminationFunction(Stacato::Application::terminate);
	Workspace::setFileOpenCallback(Stacato::Application::openFile);
	NewGui::setInitializationFunction(Stacato::Gui::initialize);
	NewGui::setGuiSubmitFunction(Stacato::Gui::gui);
	NewGui::setTerminationFunction(Stacato::Gui::terminate);
	
	//launch application
	Application::run();
}
