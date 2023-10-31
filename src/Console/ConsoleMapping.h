#pragma once

class Console;

#include "ConsoleIODevice.h"


#include "Animation/ManualControlChannel.h"


class ConsoleMapping{
public:
	
	bool matchesConsole(std::shared_ptr<Console> console);
		
	virtual std::vector<IODevice::Type>& getDeviceTypes() = 0;
	virtual std::string& getConsoleName() = 0;
	virtual void apply(std::shared_ptr<Console> console) = 0;
	
	virtual void gui(float height) = 0;
	
	virtual void onDisconnection() = 0;
	
	
	std::vector<std::shared_ptr<ManualControlDevice>>& getManualControlDevices(){ return manualControlDevices; }
	
protected:
	std::vector<std::shared_ptr<ManualControlDevice>> manualControlDevices = {};
	
	
};
