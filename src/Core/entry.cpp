#include <pch.h>

#include "config.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Project/Project.h"
#include "Project/Environnement.h"


#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main() {
#endif
    
    //initializes application window and sets working directory
	ApplicationWindow::init();
    
	//Logger is initialized after working directory is defined to have log file access
	Logger::init();
	
	Logger::info("Stacato Version {}.{} {} - "
#ifdef STACATO_DEBUG
				 "Debug Build",
#else
				 "Release Build",
#endif
				 VERSION_MAJOR, VERSION_MINOR, STACATO_OS_NAME);
	
	Logger::debug("Application Working Directory: {}", std::filesystem::current_path().string());
	
	//initialize node factory librairies
	EtherCatDeviceFactory::loadDevices();
	NodeFactory::loadNodes();
	
	//load network interfaces, initialize networking
	Environnement::initialize();
	
	//load environnement and plots, configure ethercat network interfaces
	Project::load("Project");
	
	//opens ethercat network interface
	Environnement::open();

	//application gui runtime, function returns when application is quit
	ApplicationWindow::open(3500,2000);

	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();

	//shut down logger
	Logger::terminate();

	//terminate application
	ApplicationWindow::terminate();
}


/*
	namespace TypeNamespace{
		
		enum class Enumerator{
			TYPE_1,
			TYPE_2,
			TYPE_3,
			TYPE_4
		};
		
		struct TypeStructure{
			Enumerator enumerator;
			char saveString[64];
			char displayString[64];
		};
		
		inline std::vector<TypeStructure>& getTypeStructures(){
			static std::vector<TypeStructure> typeStructures = {
				{Enumerator::TYPE_1, "Type1", "Type 1"},
				{Enumerator::TYPE_2, "Type2", "Type 2"},
				{Enumerator::TYPE_3, "Type3", "Type 3"},
				{Enumerator::TYPE_4, "Type4", "Type 4"}
			};
			return typeStructures;
		}
		
		TypeStructure* getTypeStructure(Enumerator enumerator){
			for(auto& typeStructure : getTypeStructures()) if(enumerator == typeStructure.enumerator) return &typeStructure;
			return nullptr;
		}
		
		const char* getSaveName(Enumerator enumerator){
			TypeStructure* typeStructure = getTypeStructure(enumerator);
			if(typeStructure) return typeStructure->saveString;
			return nullptr;
		}
		
		const char* getDisplayName(Enumerator enumerator){
			TypeStructure* typeStructure = getTypeStructure(enumerator);
			if(typeStructure) return typeStructure->displayString;
			return nullptr;
		}
		
		TypeStructure* getTypeStructureFromSaveString(const char* saveString){
			for(auto& typeStructure : getTypeStructures()) if(strcmp(saveString, typeStructure.saveString) == 0) return &typeStructure;
			return nullptr;
		}
		
		bool isValidSaveName(const char* saveString){
			return getTypeStructureFromSaveString(saveString) != nullptr;
		}
		
		Enumerator getEnumeratorFromSaveString(const char* saveString){
			return getTypeStructureFromSaveString(saveString)->enumerator;
		}
		
	}
*/

