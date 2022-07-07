#include "ConsoleMapping.h"

#include "Console.h"
#include "ConsoleIODevice.h"

bool ConsoleMapping::matchesConsole(std::shared_ptr<Console> console){
	if(getConsoleName() != console->getName()) return false;
	auto& consoleDevices = console->getIODevices();
	auto& expectedDeviceTypes = getDeviceTypes();
	if(consoleDevices.size() != expectedDeviceTypes.size()) return false;
	int deviceCount = expectedDeviceTypes.size();
	for(int i = 0; i < deviceCount; i++){
		if(consoleDevices[i]->getType() != expectedDeviceTypes[i]) return false;
	}
	return true;
}
