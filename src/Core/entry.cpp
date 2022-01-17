#include <pch.h>

#include "config.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Project/Project.h"
#include "Project/Environnement.h"

void doStuff();

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main() {
#endif
	
	doStuff();
	return;
	
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


	
	
	
struct Thing{
	int a = 2, b, c;
	float d, e, f;
};
	
class Pin {
public:
	
	enum class DataType{
		BOOLEAN,
		INTEGER,
		REAL,
		THING
	};
	
	std::weak_ptr<void> pointer;
	DataType type;
	
	//Constructor : assigns data and sets datatype enumerator
	//needs a template specialization for each supported type
	template<typename T>
	Pin(std::shared_ptr<T> sPtr);
	
	//Get a shared pointer of the specified type
	template<typename T>
	const std::shared_ptr<T> getSharedPointer(){
		return std::static_pointer_cast<T>(pointer.lock());
	}
	
	//Get a value directly or through implicit conversion
	//needs a template specialization for each supported type
	template<typename T>
	T get();
	
	//return type compatibility
	//indicate if type is the same or implicit conversion is possible
	bool canConnectTo(std::shared_ptr<Pin> other){
		switch(type){
			case DataType::BOOLEAN:
			case DataType::INTEGER:
			case DataType::REAL:
				switch(other->type){
					case DataType::BOOLEAN:
					case DataType::INTEGER:
					case DataType::REAL:
						return true;
					default:
						return false;
				}
			default:
				return other->type == type;
		}
	}
	
};
	
	
//==============================================================
//============= Direct Return / Implicit Conversion ============
//==============================================================
	
template<>
bool Pin::get(){
	switch(type){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return false;
	}
}
	
template<>
int Pin::get(){
	switch(type){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0;
	}
}

template<>
double Pin::get(){
	switch(type){
		case DataType::BOOLEAN: return *getSharedPointer<bool>();
		case DataType::INTEGER: return *getSharedPointer<int>();
		case DataType::REAL: return *getSharedPointer<double>();
		default: return 0.0;
	}
}


//==============================================================
//======================== Constructors ========================
//==============================================================
	
template<>
Pin::Pin(std::shared_ptr<bool> boolPtr) : pointer(boolPtr) {
	type = DataType::BOOLEAN;
}
	
template<>
Pin::Pin(std::shared_ptr<int> intPtr) : pointer(intPtr) {
	type = DataType::INTEGER;
}

template<>
Pin::Pin(std::shared_ptr<double> realPtr) : pointer(realPtr) {
	type = DataType::REAL;
}
	
template<>
Pin::Pin(std::shared_ptr<Thing> thingPtr) : pointer(thingPtr) {
	type = DataType::THING;
}
	
	
	
	
	
	

	
#include <iostream>

	
void doStuff(){
	
	std::shared_ptr<int> intPtr1 = std::make_shared<int>(3);
	std::shared_ptr<int> intPtr2 = std::make_shared<int>(4);
	std::shared_ptr<double> realPtr = std::make_shared<double>(3.3333);
	std::shared_ptr<Thing> thingPtr = std::make_shared<Thing>();
	
	std::shared_ptr<Pin> intPin1 = std::make_shared<Pin>(intPtr1);
	std::shared_ptr<Pin> intPin2 = std::make_shared<Pin>(intPtr2);
	std::shared_ptr<Pin> realPin = std::make_shared<Pin>(realPtr);
	std::shared_ptr<Pin> thingPin = std::make_shared<Pin>(thingPtr);
	
	std::vector<std::shared_ptr<Pin>> pins = {intPin1, intPin2, realPin, thingPin};
	
	int intval1 = intPin1->get<int>();
	int intval2 = intPin2->get<int>();
	double realVal = realPin->get<double>();
	auto thingPtr2 = thingPin->getSharedPointer<Thing>();
	
	auto insptr = thingPin->getSharedPointer<int>();
	int test = *insptr;
	
	std::cout << test << std::endl;
	
	std::cout << intval1 << " " << intval2 << " " << realVal << std::endl;
	
}
