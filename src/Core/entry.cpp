#include <pch.h>

#include "config.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Project/Project.h"
#include "Project/Environnement.h"

const char* doStuff();

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main() {
#endif
	
	//doStuff();
	//return;
	
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
enum class TemperatureUnit{
	   CELSIUS,
	   KELVIN,
	   FARENHEIT
   };

#define TemperatureUnitTypes \
	{TemperatureUnit::KELVIN, "Kelvin", "SaveKelvin", "Kelvins", "K", true, 1.1, 2.2},\
	{TemperatureUnit::CELSIUS, "Celsius", "SaveCelsius", "Celsii", "C", false, 3.3, 4.4},\
	{TemperatureUnit::FARENHEIT, "Farenheit", "SaveFarenheit", "Farenheits", "F", false, 5.5, 6.6}\

DEFINE_UNIT_ENUMERATOR(TemperatureUnit, TemperatureUnitTypes)

	
#include <iostream>

const char* doStuff(){
	for(auto& unitType : UnitEnumerator<TemperatureUnit>::getTypes()){
		TemperatureUnit enumerator = unitType.enumerator;
		
		std::cout << UnitEnumerator<TemperatureUnit>::getSaveString(enumerator) << " "
		<< UnitEnumerator<TemperatureUnit>::getDisplayString(enumerator) << " "
		<< UnitEnumerator<TemperatureUnit>::getDisplayStringPlural(enumerator) << " "
		<< UnitEnumerator<TemperatureUnit>::getDisplayStringAbbreviated(enumerator) << " "
		<< UnitEnumerator<TemperatureUnit>::isBaseUnit(enumerator) << " "
		<< UnitEnumerator<TemperatureUnit>::getBaseUnitMultiple(enumerator) << " "
		<< UnitEnumerator<TemperatureUnit>::getBaseUnitOffset(enumerator) << std::endl;
		
		std::cout << Enumerator<TemperatureUnit>::getSaveString(enumerator) << " "
		<< Enumerator<TemperatureUnit>::getDisplayString(enumerator) << std::endl;
		
		const char* display = UnitEnumerator<TemperatureUnit>::getDisplayString(enumerator);
		const char* save = UnitEnumerator<TemperatureUnit>::getSaveString(enumerator);
		const char* plural = UnitEnumerator<TemperatureUnit>::getDisplayStringPlural(enumerator);
		const char* abbreviated = UnitEnumerator<TemperatureUnit>::getDisplayStringAbbreviated(enumerator);
		
		bool isBase = UnitEnumerator<TemperatureUnit>::isBaseUnit(enumerator);
		
	}
}
 */
