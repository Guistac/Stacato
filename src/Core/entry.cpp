#include <pch.h>

#include "Legato/Application.h"
#include "Stacato/StacatoApplication.h"
#include "Stacato/StacatoEditor.h"
#include "Stacato/StacatoGui.h"



class Base{
public:
	virtual void doThing(){}
};

class SpecialClass{
public:
	void doSpecialThing(){}
};

class Derived1 : public Base{
public:
	virtual void doThing() override {}
};

class Derived2 : public Base, public SpecialClass{
public:
	virtual void doThing() override {}
};







#if defined(STACATO_WIN32_APPLICATION)
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	

	std::shared_ptr<Base> derived1 = std::make_shared<Derived1>();
	std::shared_ptr<Base> derived2 = std::make_shared<Derived2>();
	
	std::shared_ptr<SpecialClass> special1 = std::dynamic_pointer_cast<SpecialClass>(derived1);
	std::shared_ptr<SpecialClass> special2 = std::dynamic_pointer_cast<SpecialClass>(derived2);
	
	
	
	//configure application
	Application::setInitializationFunction(Stacato::Application::initialize);
	Application::setTerminationFunction(Stacato::Application::terminate);
	Application::setQuitRequestFunction(Stacato::Application::requestQuit);
	Workspace::setFileOpenCallback(Stacato::Editor::openFile);
	Legato::Gui::setInitializationFunction(Stacato::Gui::initialize);
	Legato::Gui::setGuiSubmitFunction(Stacato::Gui::gui);
	Legato::Gui::setTerminationFunction(Stacato::Gui::terminate);
	
	//launch application
	Application::run();
}
