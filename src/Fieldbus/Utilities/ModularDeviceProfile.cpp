#include <pch.h>

#include "ModularDeviceProfile.h"

#include <tinyxml2.h>

namespace EtherCAT::ModularDeviceProfile{

void ModularDevice::onConstruction(){
	EtherCatDevice::onConstruction();
	moduleList = Legato::ListComponent<DeviceModule>::createInstance();
	moduleList->setSaveString("Modules");
	moduleList->setEntrySaveString("Module");
}

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
			Logger::info("Identified module {} as {}", moduleNumber, detectedModule->getName());
		}
		
		detectedModules.push_back(createModule(detectedModuleIdent));
	}
	
	while(!getModules().empty()) removeModule(getModules()[0]);
	
	for(auto& module : detectedModules) addModule(module);
	
	moduleDiscoveryStatus = DataTransferState::SUCCEEDED;
}


std::shared_ptr<DeviceModule> ModularDevice::createModule(uint32_t identifier){
	for(auto factoryModule : getModuleFactory()){
		if(factoryModule->getIdentifier() == identifier){
			return factoryModule->duplicate<DeviceModule>();
		}
	}
	return nullptr;
}

std::shared_ptr<DeviceModule> ModularDevice::createModule(const char* saveString){
	for(auto factoryModule : getModuleFactory()){
		if(strcmp(factoryModule->getSaveString().c_str(), saveString) == 0){
			return factoryModule->duplicate<DeviceModule>();
		}
	}
	return nullptr;
}


bool ModularDevice::configureModules(){
	
	for(int i = 0; i < getModules().size(); i++){
		auto deviceModule = getModules()[i];
		uint8_t subindex = i + 1;
		if(!writeSDO_U32(0xF030, subindex, deviceModule->getIdentifier())) {
			return Logger::error("{} : Could not write Configured Module Ident of module {}", getName(), deviceModule->getName());
		}
	}
	
	//===== Module Parameter Configuration =====
	for(auto& deviceModule : getModules()) {
		if(!deviceModule->configureParameters()){
			return Logger::error("{} : Module {} configuration failed", getName(), deviceModule->getName());
		}else Logger::trace("{} : Configured Module {}", getName(), deviceModule->getName());
	}
	Logger::info("{} : successfully configured {} modules", getName(), getModules().size());
	
	//===== Module PDO Mapping =====
	if(!rxPdoAssignement.mapToRxPdoSyncManager(getSlaveIndex(), supportsCoE_PDOconfig())){
		return Logger::error("{} : Failed to upload Rx-PDO Configuration", getName());
	}
	if(!txPdoAssignement.mapToTxPdoSyncManager(getSlaveIndex(), supportsCoE_PDOconfig())){
		return Logger::error("{} : Failed to upload Tx-PDO Configuration", getName());
	}
	
	Logger::info("{} : successfully configured module process data", getName(), getModules().size());
	
	return true;
	
}


void ModularDevice::readModuleInputs(){
	for(auto& deviceModule : getModules()) deviceModule->readInputs();
}

void ModularDevice::writeModuleOutputs(){
	for(auto& deviceModule : getModules()) deviceModule->writeOutputs();
}


void ModularDevice::addModule(std::shared_ptr<DeviceModule> deviceModule){
	if(getModules().empty()) beforeModuleReordering();
	
	std::shared_ptr<ModularDevice> thisDevice = downcasted_shared_from_this<ModularDevice>();
	deviceModule->setParentDevice(thisDevice);
	deviceModule->setIndex((int)getModules().size());
	moduleList->addEntry(deviceModule);
	selectedModule = deviceModule;
	for(auto& inputPin : deviceModule->inputPins) addNodePin(inputPin);
	for(auto& outputPin : deviceModule->outputPins) addNodePin(outputPin);
	
	deviceModule->addTxPdoMappingModule(txPdoAssignement);
	deviceModule->addRxPdoMappingModule(rxPdoAssignement);
}

void ModularDevice::removeModule(std::shared_ptr<DeviceModule> deviceModule){
	moduleList->removeEntry(deviceModule);
	if(selectedModule == deviceModule) selectedModule = nullptr;
	for(auto& inputPin : deviceModule->inputPins) removeNodePin(inputPin);
	for(auto& outputPin : deviceModule->outputPins) removeNodePin(outputPin);
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	for(int i = 0; i < getModules().size(); i++) {
		getModules()[i]->setIndex(i);
		getModules()[i]->addTxPdoMappingModule(txPdoAssignement);
		getModules()[i]->addRxPdoMappingModule(rxPdoAssignement);
	}
}

void ModularDevice::reorderModule(int oldIndex, int newIndex){
	//stored the moved module in a temporary buffer
	std::shared_ptr<DeviceModule> temp = getModules()[oldIndex];
	
	//erase the module and insert it at the new index
	getModules().erase(getModules().begin() + oldIndex);
	getModules().insert(getModules().begin() + newIndex, temp);
	
	//erase all input pins from the nodes pin vectors
	std::vector<std::shared_ptr<NodePin>>& inputPins = getInputPins();
	std::vector<std::shared_ptr<NodePin>>& outputPins = getOutputPins();
	inputPins.clear();
	outputPins.clear();
	
	//add back all pins in the correct order
	//outputPins.push_back(gpioDeviceLink);
	for(auto& module : getModules()){
		inputPins.insert(inputPins.end(), module->inputPins.begin(), module->inputPins.end());
		outputPins.insert(outputPins.end(), module->outputPins.begin(), module->outputPins.end());
	}
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	beforeModuleReordering();
	for(int i = 0; i < getModules().size(); i++) {
		getModules()[i]->setIndex(i);
		getModules()[i]->addTxPdoMappingModule(txPdoAssignement);
		getModules()[i]->addRxPdoMappingModule(rxPdoAssignement);
	}
}

void ModularDevice::moveModuleUp(std::shared_ptr<DeviceModule> movedModule){
	int oldIndex;
	for(int i = 0; i < getModules().size(); i++){
		if(movedModule == getModules()[i]){
			oldIndex = i;
			break;
		}
	}
	if(oldIndex <= 0) return;
	else reorderModule(oldIndex, oldIndex - 1);
}

void ModularDevice::moveModuleDown(std::shared_ptr<DeviceModule> movedModule){
	int oldIndex;
	for(int i = 0; i < getModules().size(); i++){
		if(movedModule == getModules()[i]){
			oldIndex = i;
			break;
		}
	}
	if(oldIndex >= getModules().size() - 1) return;
	else reorderModule(oldIndex, oldIndex + 1);
}

bool ModularDevice::onSerialization() {
	bool success = true;
	success &= EtherCatDevice::onSerialization();
	success &= moduleList->serializeIntoParent(this);
	return success;
}

bool ModularDevice::onDeserialization() {
	bool success = true;
	success &= EtherCatDevice::onDeserialization();
	
	//load all modules into a temporary list
	std::shared_ptr<Legato::ListComponent<DeviceModule>> loadedModules = Legato::ListComponent<DeviceModule>::createInstance();
	loadedModules->setSaveString("Modules");
	loadedModules->setEntrySaveString("Module");
	loadedModules->setEntryConstructor([this](Serializable& abstractEntry) -> std::shared_ptr<DeviceModule> {
		std::string className;
		if(abstractEntry.deserializeAttribute("ClassName", className)){
			for(auto factoryModule : getModuleFactory()){
				if(factoryModule->getClassName() == className){
					std::shared_ptr<DeviceModule> newModule = factoryModule->duplicate<DeviceModule>();
					return newModule;
				}
			}
		}
		return nullptr;
	});
	success &= loadedModules->deserializeFromParent(this);
	
	//add all modules to the device
	for(auto loadedModule : loadedModules->getEntries()){
		addModule(loadedModule);
	}
	
	return success;
}

bool DeviceModule::onSerialization() {
	bool success = true;
	success &= Component::onSerialization();
	success &= serializeAttribute("ClassName", getClassName());
	return success;
}

bool DeviceModule::onDeserialization() {
	bool success = true;
	success &= Component::onDeserialization();
	return success;
}


};
