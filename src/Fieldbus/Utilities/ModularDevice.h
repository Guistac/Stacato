#pragma once

#include "Fieldbus/EtherCatDevice.h"

namespace EtherCAT{

class DeviceModule;

class ModularDevice : public EtherCatDevice{
public:
	
	//modules and module management
	std::vector<std::shared_ptr<DeviceModule>> modules = {};
	
	//module list management
	void addModule(std::shared_ptr<DeviceModule> module);
	void removeModule(std::shared_ptr<DeviceModule> module);
	void reorderModule(int oldIndex, int newIndex);
	void moveModuleUp(std::shared_ptr<DeviceModule> module);
	void moveModuleDown(std::shared_ptr<DeviceModule> module);
	
	virtual void beforeModuleReordering(){}
	
	//module configuration
	bool discoverDeviceModules();
	DataTransferState moduleDiscoveryStatus = DataTransferState::NO_TRANSFER;
	bool configureModules();
	
	//module process data
	void readModuleInputs();
	void writeModuleOutputs();
	
	bool saveModules(tinyxml2::XMLElement* xml);
	bool loadModules(tinyxml2::XMLElement* xml);
	
	//module factory
	virtual std::vector<EtherCAT::DeviceModule*>& getModuleFactory() = 0;
	std::shared_ptr<EtherCAT::DeviceModule> createModule(uint32_t identifier);
	std::shared_ptr<EtherCAT::DeviceModule> createModule(const char* saveString);
	
	//module management gui
	void moduleManagerGui();
	std::shared_ptr<DeviceModule> selectedModule = nullptr;
};


};
