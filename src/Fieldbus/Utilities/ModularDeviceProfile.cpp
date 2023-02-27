#include <pch.h>

#include "ModularDeviceProfile.h"

#include <tinyxml2.h>

namespace EtherCAT::ModularDeviceProfile{

bool ModularDevice::discoverDeviceModules(){
	
	moduleDiscoveryStatus = DataTransferState::TRANSFERRING;
	
	
	//for some reason the phoenix contact coupler needs to be rescanned to be able to read modules again
	
	/*
	 //TODO: how is this actuallly used ?
	//Modular Device Profile: Detect Modules Command
	uint8_t dummy;
	if(!readSDO_U8(0xF002, 0x0, dummy)) {
		moduleDiscoveryStatus = EtherCatDevice::DataTransferState::FAILED;
		return Logger::error("Could not execute Detect Modules Command");
	}
	 */
	
	uint8_t detectedModuleCount;
	if(!readSDO_U8(0xF050, 0x0, detectedModuleCount)){
		moduleDiscoveryStatus = DataTransferState::FAILED;
		return Logger::error("Could not read Detected Module Count");
	}else{
		Logger::info("Detected {} modules", detectedModuleCount);
	}
	
	std::vector<std::shared_ptr<DeviceModule>> detectedModules = {};
	
	for(int i = 0; i < detectedModuleCount; i++){
		uint8_t moduleNumber = i + 1;
		uint32_t detectedModuleIdent;
		//Modular Device Profile: Module Ident list of detected Modules
		if(!readSDO_U32(0xF050, moduleNumber, detectedModuleIdent)) {
			moduleDiscoveryStatus = DataTransferState::FAILED;
			Logger::error("Could not download module {} identifier", moduleNumber);
			return false;
		}
		
		std::shared_ptr<DeviceModule> detectedModule = createModule(detectedModuleIdent);
		if(detectedModule == nullptr){
			moduleDiscoveryStatus = EtherCatDevice::DataTransferState::FAILED;
			Logger::error("Could not identify module {}", moduleNumber);
			return false;
		}else{
			Logger::info("Identified module {} as {}", moduleNumber, detectedModule->getDisplayName());
		}
		
		detectedModules.push_back(createModule(detectedModuleIdent));
	}
	
	while(!modules.empty()) removeModule(modules[0]);
	
	for(auto& module : detectedModules) addModule(module);
	
	moduleDiscoveryStatus = DataTransferState::SUCCEEDED;
}


std::shared_ptr<DeviceModule> ModularDevice::createModule(uint32_t identifier){
	for(auto factoryModule : getModuleFactory()){
		if(factoryModule->getIdentifier() == identifier){
			return factoryModule->getNewInstance();
		}
	}
	return nullptr;
}

std::shared_ptr<DeviceModule> ModularDevice::createModule(const char* saveString){
	for(auto factoryModule : getModuleFactory()){
		if(strcmp(factoryModule->getSaveName(), saveString) == 0){
			return factoryModule->getNewInstance();
		}
	}
	return nullptr;
}


bool ModularDevice::configureModules(){
	
	for(int i = 0; i < modules.size(); i++){
		auto module = modules[i];
		uint8_t subindex = i + 1;
		if(!writeSDO_U32(0xF030, subindex, module->getIdentifier())) {
			return Logger::error("{} : Could not write Configured Module Ident of module {}", getName(), module->getDisplayName());
		}
	}
	
	//===== Module Parameter Configuration =====
	for(auto& module : modules) {
		if(!module->configureParameters()){
			return Logger::error("{} : Module {} configuration failed", getName(), module->getDisplayName());
		}else Logger::trace("{} : Configured Module {}", getName(), module->getDisplayName());
	}
	Logger::info("{} : successfully configured {} modules", getName(), modules.size());
	
	//===== Module PDO Mapping =====
	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex(), supportsCoE_PDOconfig())){
		return Logger::error("{} : Failed to upload Rx-PDO Configuration", getName());
	}
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex(), supportsCoE_PDOconfig())){
		return Logger::error("{} : Failed to upload Tx-PDO Configuration", getName());
	}
	
	Logger::info("{} : successfully configured module process data", getName(), modules.size());
	
	return true;
	
}


void ModularDevice::readModuleInputs(){
	for(auto& module : modules) module->readInputs();
}

void ModularDevice::writeModuleOutputs(){
	for(auto& module : modules) module->writeOutputs();
}


void ModularDevice::addModule(std::shared_ptr<DeviceModule> deviceModule){
	if(modules.empty()) beforeModuleReordering();
	
	std::shared_ptr<ModularDevice> thisDevice = std::static_pointer_cast<ModularDevice>(shared_from_this());
	deviceModule->setParentDevice(thisDevice);
	deviceModule->setIndex(modules.size());
	modules.push_back(deviceModule);
	selectedModule = deviceModule;
	for(auto& inputPin : deviceModule->inputPins) addNodePin(inputPin);
	for(auto& outputPin : deviceModule->outputPins) addNodePin(outputPin);
	
	deviceModule->addTxPdoMappingModule(txPdoAssignement);
	deviceModule->addRxPdoMappingModule(rxPdoAssignement);
}

void ModularDevice::removeModule(std::shared_ptr<DeviceModule> deviceModule){
	for(int i = modules.size() - 1; i >= 0; i--){
		if(modules[i] == deviceModule){
			modules.erase(modules.begin() + i);
			break;
		}
	}
	if(selectedModule == deviceModule) selectedModule = nullptr;
	for(auto& inputPin : deviceModule->inputPins) removeIoData(inputPin);
	for(auto& outputPin : deviceModule->outputPins) removeIoData(outputPin);
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	for(int i = 0; i < modules.size(); i++) {
		modules[i]->setIndex(i);
		modules[i]->addTxPdoMappingModule(txPdoAssignement);
		modules[i]->addRxPdoMappingModule(rxPdoAssignement);
	}
}

void ModularDevice::reorderModule(int oldIndex, int newIndex){
	//stored the moved module in a temporary buffer
	std::shared_ptr<DeviceModule> temp = modules[oldIndex];
	
	//erase the module and insert it at the new index
	modules.erase(modules.begin() + oldIndex);
	modules.insert(modules.begin() + newIndex, temp);
	
	//erase all input pins from the nodes pin vectors
	std::vector<std::shared_ptr<NodePin>>& inputPins = getInputPins();
	std::vector<std::shared_ptr<NodePin>>& outputPins = getOutputPins();
	inputPins.clear();
	outputPins.clear();
	
	//add back all pins in the correct order
	//outputPins.push_back(gpioDeviceLink);
	for(auto& module : modules){
		inputPins.insert(inputPins.end(), module->inputPins.begin(), module->inputPins.end());
		outputPins.insert(outputPins.end(), module->outputPins.begin(), module->outputPins.end());
	}
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	beforeModuleReordering();
	for(int i = 0; i < modules.size(); i++) {
		modules[i]->setIndex(i);
		modules[i]->addTxPdoMappingModule(txPdoAssignement);
		modules[i]->addRxPdoMappingModule(rxPdoAssignement);
	}
}

void ModularDevice::moveModuleUp(std::shared_ptr<DeviceModule> module){
	int oldIndex;
	for(int i = 0; i < modules.size(); i++){
		if(module == modules[i]){
			oldIndex = i;
			break;
		}
	}
	if(oldIndex <= 0) return;
	else reorderModule(oldIndex, oldIndex - 1);
}

void ModularDevice::moveModuleDown(std::shared_ptr<DeviceModule> module){
	int oldIndex;
	for(int i = 0; i < modules.size(); i++){
		if(module == modules[i]){
			oldIndex = i;
			break;
		}
	}
	if(oldIndex >= modules.size() - 1) return;
	else reorderModule(oldIndex, oldIndex + 1);
}

bool ModularDevice::saveModules(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* modularDeviceProfileXML = xml->InsertNewChildElement("ModularDeviceProfile");
	XMLElement* modulesXML = modularDeviceProfileXML->InsertNewChildElement("Modules");
	
	for(auto module : modules){
		XMLElement* moduleXML = modulesXML->InsertNewChildElement("Module");
		moduleXML->SetAttribute("Type", module->getSaveName());
		module->save(moduleXML);
	}

	return true;
}

bool ModularDevice::loadModules(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* modularDeviceProfileXML = xml->FirstChildElement("ModularDeviceProfile");
	if(modularDeviceProfileXML == nullptr) return Logger::warn("failed to load Modular Device Profile");
	
	XMLElement* modulesXML = modularDeviceProfileXML->FirstChildElement("Modules");
	if(modulesXML == nullptr) return Logger::warn("failed to load Device Modules");
	
	XMLElement* moduleXML = modulesXML->FirstChildElement("Module");
	while(moduleXML != nullptr){
		const char* moduleTypeString;
		if(moduleXML->QueryStringAttribute("Type", &moduleTypeString) != XML_SUCCESS) return Logger::warn("Failed to load module type");
		std::shared_ptr<DeviceModule> loadedModule = createModule(moduleTypeString);
		if(loadedModule == nullptr) return Logger::warn("Failed to identify module type '{}'", moduleTypeString);
		if(!loadedModule->load(moduleXML)) return Logger::warn("Failed to load module {}", moduleTypeString);
		addModule(loadedModule);
		moduleXML = moduleXML->NextSiblingElement("Module");
	}
	
	return true;
	
}

};
