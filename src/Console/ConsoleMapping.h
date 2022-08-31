#pragma once

class Console;

#include "ConsoleIODevice.h"

class ConsoleMapping{
public:
	
	bool matchesConsole(std::shared_ptr<Console> console);
		
	virtual std::vector<IODevice::Type>& getDeviceTypes() = 0;
	virtual std::string& getConsoleName() = 0;
	virtual void apply(std::shared_ptr<Console> console) = 0;
	
	virtual void gui(float height) = 0;
	
};
