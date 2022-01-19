#include <pch.h>

#include "VIPA-053-1EC01.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

#include "VipaModule.h"

//TODO: this onConnection & onDisconnection Methods don't get called at the operational state transition and this might cause issues with subdevice readyness state
void VipaBusCoupler_053_1EC01::onDisconnection() {
	gpioDevice->b_detected = false;
	gpioDevice->b_online = false;
	gpioDevice->b_ready = false;
	for(auto& module : modules) module->onDisconnection();
}

void VipaBusCoupler_053_1EC01::onConnection() {
	gpioDevice->b_detected = true;
	gpioDevice->b_online = true;
	gpioDevice->b_ready = true;
	for(auto& module : modules) module->onConnection();
}

void VipaBusCoupler_053_1EC01::initialize() {
    //by default, this node only has one pin
    //no modules are loaded by default
    std::shared_ptr<Device> thisDevice = std::dynamic_pointer_cast<Device>(shared_from_this());
    gpioDevice->setParentDevice(thisDevice);
	gpioDeviceLink->assignData(gpioDevice);
    //gpio device link pin
    addNodePin(gpioDeviceLink);
}


bool VipaBusCoupler_053_1EC01::startupConfiguration() {
	
	//===== Module Parameter Configuration =====
	for(auto& module : modules) {
		if(!module->configureParameters()){
			return Logger::warn("Module {} Parameter Configuration Failed", module->getDisplayName());
		}
	}
	
    //===== Module PDO Mapping =====
    uint16_t RxPDO_SyncManager = 0x1C12;
    if (!writeSDO_U8(RxPDO_SyncManager, 0x0, 0)) return Logger::warn("Could not disable RxPDO Sync Manager"); //disable Sync Manager
	for(int i = 0; i < rxPdoAssignement.modules.size(); i++){
		uint16_t rxPdoModuleIndex = rxPdoAssignement.modules[i].index;
		if(!writeSDO_U16(RxPDO_SyncManager, i+1, rxPdoModuleIndex)) return Logger::warn("Could not assign RxPDO Mapping Module 0x{:X}", rxPdoModuleIndex);
	}
	uint8_t RxPDOmoduleCount = rxPdoAssignement.getModuleCount();
	if (!writeSDO_U8(RxPDO_SyncManager, 0x0, RxPDOmoduleCount)) return Logger::warn("Could not assign RxPDO module count {}", RxPDOmoduleCount);
	
	uint16_t TxPDO_SyncManager = 0x1C13;
	if (!writeSDO_U8(TxPDO_SyncManager, 0x0, 0)) return Logger::warn("Could not disable TxPDO Sync Manager"); //disable Sync Manager
	for(int i = 0; i < txPdoAssignement.modules.size(); i++){
		uint16_t txPdoModuleIndex = txPdoAssignement.modules[i].index;
		if(!writeSDO_U16(TxPDO_SyncManager, i+1, txPdoModuleIndex)) return Logger::warn("Could not assign TxPDO Mapping Module 0x{:X}", txPdoModuleIndex);
	}
	uint8_t TxPDOmoduleCount = txPdoAssignement.getModuleCount();
	
	//module does not work without TxPDO Data
	//if there are to modules with inputs we add the bus coupler diagnostic pdo mapping to satisfy the device
	if(TxPDOmoduleCount == 0){
		if(!writeSDO_U16(0x1C13, TxPDOmoduleCount+1, 0x1AFF)) return Logger::critical("Could not upload 0x1AFF");
		if (!writeSDO_U8(TxPDO_SyncManager, 0x0, TxPDOmoduleCount + 1)) return Logger::warn("Could not assign TxPDO module count {}", TxPDOmoduleCount);
	}else{
		if (!writeSDO_U8(TxPDO_SyncManager, 0x0, TxPDOmoduleCount)) return Logger::warn("Could not assign TxPDO module count {}", TxPDOmoduleCount);
	}
	
    return true;
}


void VipaBusCoupler_053_1EC01::readInputs() {
	txPdoAssignement.pullDataFrom(identity->inputs);
	for(auto& module : modules) module->readInputs();
	gpioDevice->b_ready = isStateOperational();
}


void VipaBusCoupler_053_1EC01::prepareOutputs(){
	for(auto& module : modules) module->writeOutputs();
	rxPdoAssignement.pushDataTo(identity->outputs);
}












void VipaBusCoupler_053_1EC01::configureFromDeviceModules() {
    configureFromDeviceModulesDownloadStatus = DataTransferState::TRANSFERRING;
    std::vector<std::shared_ptr<VipaModule>> downloadedModules;
    if (!downloadDeviceModules(downloadedModules)) {
        configureFromDeviceModulesDownloadStatus = DataTransferState::FAILED;
        return;
    }
	for(int i = modules.size() - 1; i >= 0; i--) removeModule(modules[i]);
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	for(auto& module : downloadedModules) addModule(module);
    configureFromDeviceModulesDownloadStatus = DataTransferState::SUCCEEDED;
}


bool VipaBusCoupler_053_1EC01::downloadDeviceModules(std::vector<std::shared_ptr<VipaModule>>& output) {
	uint8_t detectedModuleCount;
	if (!readSDO_U8(0xF050, 0x0, detectedModuleCount)) return false;
	int inputByteCount = 0;
	int outputByteCount = 0;
	for (int i = 0; i < detectedModuleCount; i++) {
		//contains the name of the module as well as its serial number and version info
		uint16_t moduleInformationIndex = 0x4100 + i;
		//download the name of the module to get the module type
		char nameBuffer[128];
		if (!readSDO_String(moduleInformationIndex, 0x1, nameBuffer,128)) return false;
		//create module and add it
		std::shared_ptr<VipaModule> detectedModule = VipaModuleFactory::getModule(nameBuffer);
		if(detectedModule == nullptr){
			Logger::error("{} : Could not identidy Module of Type {}", getName(), nameBuffer);
			return false;
		}
		output.push_back(detectedModule);
	}
	return true;
}






void VipaBusCoupler_053_1EC01::addModule(std::shared_ptr<VipaModule> module){
	std::shared_ptr<VipaBusCoupler_053_1EC01> thisBusCoupler = std::dynamic_pointer_cast<VipaBusCoupler_053_1EC01>(shared_from_this());
	module->setParentBusCoupler(thisBusCoupler);
	module->setIndex(modules.size());
	modules.push_back(module);
	selectedModule = module;
	for(auto& inputPin : module->inputPins) addNodePin(inputPin);
	for(auto& outputPin : module->outputPins) addNodePin(outputPin);
	
	module->addTxPdoMappingModule(txPdoAssignement);
	module->addRxPdoMappingModule(rxPdoAssignement);
}

void VipaBusCoupler_053_1EC01::removeModule(std::shared_ptr<VipaModule> module){
	for(int i = modules.size() - 1; i >= 0; i--){
		if(modules[i] == module){
			modules.erase(modules.begin() + i);
			break;
		}
	}
	if(selectedModule == module) selectedModule = nullptr;
	for(auto& inputPin : module->inputPins) removeIoData(inputPin);
	for(auto& outputPin : module->outputPins) removeIoData(outputPin);
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	for(int i = 0; i < modules.size(); i++) {
		modules[i]->setIndex(i);
		modules[i]->addTxPdoMappingModule(txPdoAssignement);
		modules[i]->addRxPdoMappingModule(rxPdoAssignement);
	}
}

void VipaBusCoupler_053_1EC01::reorderModule(int oldIndex, int newIndex){
	//stored the moved module in a temporary buffer
	std::shared_ptr<VipaModule> temp = modules[oldIndex];
	
	//erase the module and insert it at the new index
	modules.erase(modules.begin() + oldIndex);
	modules.insert(modules.begin() + newIndex, temp);
	
	//erase all input pins from the nodes pin vectors
	std::vector<std::shared_ptr<NodePin>>& inputPins = getInputPins();
	std::vector<std::shared_ptr<NodePin>>& outputPins = getOutputPins();
	inputPins.clear();
	outputPins.clear();
	
	//add back all pins in the correct order
	outputPins.push_back(gpioDeviceLink);
	for(auto& module : modules){
		inputPins.insert(inputPins.end(), module->inputPins.begin(), module->inputPins.end());
		outputPins.insert(outputPins.end(), module->outputPins.begin(), module->outputPins.end());
	}
	txPdoAssignement.clear();
	rxPdoAssignement.clear();
	for(int i = 0; i < modules.size(); i++) {
		modules[i]->setIndex(i);
		modules[i]->addTxPdoMappingModule(txPdoAssignement);
		modules[i]->addRxPdoMappingModule(rxPdoAssignement);
	}
}

void VipaBusCoupler_053_1EC01::moveModuleUp(std::shared_ptr<VipaModule> module){
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

void VipaBusCoupler_053_1EC01::moveModuleDown(std::shared_ptr<VipaModule> module){
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




bool VipaBusCoupler_053_1EC01::saveDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* ioModulesXML = xml->InsertNewChildElement("SLIOModules");
	for (auto& module : modules) {
		XMLElement* moduleXML = ioModulesXML->InsertNewChildElement("Module");
		moduleXML->SetAttribute("Type", module->getSaveName());
		module->save(moduleXML);
	}
	return true;
}


bool VipaBusCoupler_053_1EC01::loadDeviceData(tinyxml2::XMLElement* xml) {
	using namespace tinyxml2;
	XMLElement* ioModulesXML = xml->FirstChildElement("SLIOModules");
	if (ioModulesXML == nullptr) return Logger::warn("Could not load SLIOModules attribute");
	XMLElement* moduleXML = ioModulesXML->FirstChildElement("Module");
	while (moduleXML != nullptr) {
		const char* moduleTypeString = "";
		if(moduleXML->QueryStringAttribute("Type", &moduleTypeString) != XML_SUCCESS) return Logger::warn("Could not read module type attribute");
		std::shared_ptr<VipaModule> module = VipaModuleFactory::getModule(moduleTypeString);
		if(module == nullptr) return Logger::warn("Could not identify module type");
		module->load(moduleXML);
		addModule(module);
		moduleXML = moduleXML->NextSiblingElement("Module");
	}
	return true;
}
