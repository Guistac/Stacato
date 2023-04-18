#include <pch.h>

#include "Legato/Application.h"
#include "Stacato/StacatoApplication.h"
#include "Stacato/StacatoEditor.h"
#include "Stacato/StacatoGui.h"



class Prototype{
public:
	virtual void doThing(){}
	virtual Prototype* getPrototype(){ return this; }
};

class NodeClass : public Prototype{
public:
	void doNodeThing(){}
	virtual Prototype* getPrototype(){ return this; }
};

class AnimatableOwnerClass : public Prototype{
public:
	void doOwnerThing(){}
	virtual void doThing() override {}
	virtual Prototype* getPrototype() override { return this; }
};

class MachineClass : public NodeClass, public AnimatableOwnerClass{
public:
	virtual void doThing() override {}
	//virtual Prototype* getPrototype(){ return this; }
};







#if defined(STACATO_WIN32_APPLICATION)
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
	auto machine = std::make_shared<MachineClass>();
	machine->doThing();
	auto owner = std::static_pointer_cast<AnimatableOwnerClass>(machine);
	owner->doThing();
	
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
