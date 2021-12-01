#include <pch.h>

#include "VIPA-053-1EC01.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

#include "VipaModule.h"

bool VipaBusCoupler_053_1EC01::isDeviceReady() {
    return isStateOperational();
}

void VipaBusCoupler_053_1EC01::enable() {
}

void VipaBusCoupler_053_1EC01::disable() {
}

bool VipaBusCoupler_053_1EC01::isEnabled() {
    return isStateOperational();
}

void VipaBusCoupler_053_1EC01::onDisconnection() {}

void VipaBusCoupler_053_1EC01::onConnection() {}

void VipaBusCoupler_053_1EC01::resetData() {
    gpioDevice->b_detected = false;
    gpioDevice->b_online = false;
    gpioDevice->b_ready = false;
}


void VipaBusCoupler_053_1EC01::assignIoData() {
    //by default, this node only has one pin
    //no modules are loaded by default
    std::shared_ptr<Device> thisDevice = std::dynamic_pointer_cast<Device>(shared_from_this());
    gpioDevice->setParentDevice(thisDevice);
    gpioDeviceLink->set(gpioDevice);
    //gpio device link pin
    addIoData(gpioDeviceLink);
}




bool VipaBusCoupler_053_1EC01::startupConfiguration() {
	
	/*
    //on startup configuration we try to match all modules to the slaves modules
    //if the modules are matching we are good to go
    //we then can safely upload the correct PDO mapping for the modules

    std::vector<Module> deviceModules;
    if (!downloadDeviceModules(deviceModules)) return Logger::warn("Could not download Device Module Information");
    int localModuleCount = modules.size();
    int deviceModuleCount = deviceModules.size();

    //TODO: do further matching of datatypes and sizes
    if (localModuleCount != deviceModuleCount) return Logger::warn("ioModule Count does not match. Local Node has {} modules, Slave reports having {} modules.", localModuleCount, deviceModuleCount);
    for (int i = 0; i < modules.size(); i++) {
        Module& localModule = modules[i];
        Module& deviceModule = deviceModules[i];

        if (localModule.moduleType != deviceModule.moduleType)
            return Logger::warn("Module #{} Type does not match device. Local Node Module is of type {}, Device reports module as type {}", i, getModuleType(localModule.moduleType)->displayName, getModuleType(deviceModule.moduleType)->displayName);
        if (localModule.inputs.size() != deviceModule.inputs.size())
            return Logger::warn("Module #{} Input Count does match device. Local Node Module has {} inputs, Device reports having {}", localModule.inputs.size(), deviceModule.inputs.size());
        if (localModule.outputs.size() != deviceModule.outputs.size())
            return Logger::warn("Module #{} Output Count does match device. Local Node Module has {} outputs, Device reports having {}", localModule.outputs.size(), deviceModule.outputs.size());
    }
    Logger::info("Node and Device Modules Match!");
*/
	
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
    if (!writeSDO_U8(TxPDO_SyncManager, 0x0, TxPDOmoduleCount)) return Logger::warn("Could not assign TxPDO module count {}", TxPDOmoduleCount);

    return true;
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
		
	/*
	ModuleType::Type moduleType = getModuleType(nameBuffer)->type;
	output.push_back(Module(moduleType));
	Module& module = output.back();

	uint16_t inputMappingModuleIndex = 0x1A00 + i;  //index of the mapping module object that stores all input PDO mappings
	uint16_t outputMappingModuleIndex = 0x1600 + i; //index of the mapping module object that stores all output PDO mappings
	uint16_t inputObjectIndex = 0x6000 + i;       //index of the object that holds all input values, index holds number of input parameters
	uint16_t outputObjectIndex = 0x7000 + i;      //index of the object that holds all output values, index holds number of output parameters

	uint8_t inputCount;
	if (readSDO_U8(inputObjectIndex, 0x0, inputCount)) {
		module.inputs = std::vector<ModuleParameter>(inputCount);
		module.inputBitCount = 0;
		for (int j = 0; j < inputCount; j++) {
			ModuleParameter& moduleParameter = module.inputs[j];
			uint32_t parameterMapping;
			if (!readSDO_U32(inputMappingModuleIndex, j + 1, parameterMapping)) return false;
			moduleParameter.index = (parameterMapping & 0xFFFF0000) >> 16;
			moduleParameter.subindex = (parameterMapping & 0xFF00) >> 8;
			moduleParameter.bitSize = (parameterMapping & 0xFF);
			moduleParameter.ioMapByteOffset = inputByteCount + module.inputBitCount / 8;    //Byte index of the data in the slaves ioMap
			moduleParameter.ioMapBitOffset = module.inputBitCount % 8;                      //Bit index in the byte of the ioMap
			module.inputBitCount += moduleParameter.bitSize;
			char pinDisplayName[64];
			char pinSaveName[64];
			sprintf(pinDisplayName, "Module %i %s Input %i", i+1, getModuleType(module.moduleType)->dataName, j+1);
			sprintf(pinSaveName, "Module%i%sInput%i", i+1, getModuleType(module.moduleType)->dataName, j+1);
			if (moduleParameter.bitSize == 1) {
				//if the bit count is one, the data type is boolean
				moduleParameter.nodePin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, pinDisplayName, pinSaveName);
			}
			else {
				moduleParameter.nodePin = std::make_shared<NodePin>(NodeData::REAL_VALUE, DataDirection::NODE_OUTPUT, pinDisplayName, pinSaveName);
				//for non boolean values, this will need to be handled differently
				//moduleParameter.NodePin = std::make_shared<NodePin>(DataType::INTEGER_VALUE, DataDirection::NODE_OUTPUT, paramet);
			}
		}
		//get the amount of full bytes the input section of the module fills (for example 4 bits are 1 byte, with 4 unused bits)
		module.inputByteCount = ceil((double)module.inputBitCount / 8.0);
		inputByteCount += module.inputByteCount;
	}

	uint8_t outputCount;
	if (readSDO_U8(outputObjectIndex, 0x0, outputCount)) {
		module.outputs = std::vector<ModuleParameter>(outputCount);
		module.outputBitCount = 0;
		for (int j = 0; j < outputCount; j++) {
			ModuleParameter& moduleParameter = module.outputs[j];
			uint32_t parameterMapping;
			if (!readSDO_U32(outputMappingModuleIndex, j + 1, parameterMapping)) return false;
			moduleParameter.index = (parameterMapping & 0xFFFF0000) >> 16;
			moduleParameter.subindex = (parameterMapping & 0xFF00) >> 8;
			moduleParameter.bitSize = (parameterMapping & 0xFF);
			moduleParameter.ioMapByteOffset = outputByteCount + module.outputBitCount / 8;  //Byte index of the data in the slaves ioMap
			moduleParameter.ioMapBitOffset = module.outputBitCount % 8;                     //Bit index in the byte of the ioMap
			module.outputBitCount += moduleParameter.bitSize;
			char pinDisplayName[128];
			char pinSaveName[128];
			sprintf(pinDisplayName, "Module %i %s Output %i", i+1, getModuleType(module.moduleType)->dataName, j+1);
			sprintf(pinSaveName, "Module%i%sOutput%i", i+1, getModuleType(module.moduleType)->dataName, j+1);
			if (moduleParameter.bitSize == 1) {
				moduleParameter.nodePin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, pinDisplayName, pinSaveName);
			}
			else {
				moduleParameter.nodePin = std::make_shared<NodePin>(NodeData::BOOLEAN_VALUE, DataDirection::NODE_INPUT, pinDisplayName, pinSaveName);
				Logger::critical("Can't Handle non boolean data in VIPA modules yet");
				//for non boolean values, this will need to be handled differently
				//moduleParameter.NodePin = std::make_shared<NodePin>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, parameterName);
			}
		}
		//get the amount of full bytes the input section of the module fills (for example 11 bits are 2 bytes, with 5 unused bits)
		module.outputByteCount = ceil((double)module.outputBitCount / 8.0);
		outputByteCount += module.outputByteCount;
	}
	*/
}







void VipaBusCoupler_053_1EC01::configureFromDeviceModules() {
    configureFromDeviceModulesDownloadStatus = DataTransferState::State::TRANSFERRING;

    std::vector<std::shared_ptr<VipaModule>> downloadedModules;
    if (!downloadDeviceModules(downloadedModules)) {
        configureFromDeviceModulesDownloadStatus = DataTransferState::State::FAILED;
        return;
    }

	for(int i = modules.size() - 1; i >= 0; i--) removeModule(modules[i]);
	for(auto& module : downloadedModules) addModule(module);

    configureFromDeviceModulesDownloadStatus = DataTransferState::State::SUCCEEDED;
}





void VipaBusCoupler_053_1EC01::readInputs() {
	
	txPdoAssignement.pullDataFrom(identity->inputs);
	
	
	for(auto& module : modules){
		module->readInputs();
	}
	/*
    uint8_t* inputBytes = identity->inputs;
    for (Module& module : modules) {
        for (auto inputParameter : module.inputs) {
            if (inputParameter.bitSize == 1) {
                bool value = 1 == (0x1 & (inputBytes[inputParameter.ioMapByteOffset] >> inputParameter.ioMapBitOffset));
                inputParameter.nodePin->set(value);
            }
            else {
				uint32_t mask = (0x1 << inputParameter.bitSize) - 1;
				uint32_t* valueAddress = (uint32_t*)&inputBytes[inputParameter.ioMapByteOffset];
				uint32_t value = *valueAddress & mask;
				double realValue = (double)value / (double)mask;
				inputParameter.nodePin->set(realValue);
				//Logger::critical("Can't read non boolean values in VIPA modules");
                //we need to format the data to an integer and pass it to the NodePin object
            }
        }
    }
    gpioDevice->b_detected = true;
    gpioDevice->b_online = true;
    gpioDevice->b_ready = isStateOperational();
	 */
}


void VipaBusCoupler_053_1EC01::prepareOutputs(){
	
	for(auto& module : modules){
		module->writeOutputs();
	}
	rxPdoAssignement.pushDataTo(identity->outputs);
	
	/*
    uint8_t* outputBytes = identity->outputs;
    for (Module& module : modules) {
        for (auto outputParameter : module.outputs) {
            if (outputParameter.bitSize == 1) {
                std::shared_ptr<NodePin> pin = outputParameter.nodePin;
                if (pin->isConnected()) pin->set(pin->getLinks().front()->getInputData()->getBoolean());
                bool value = pin->getBoolean();
                //set a bit in the output byte to 1
                if (outputParameter.nodePin->getBoolean()) outputBytes[outputParameter.ioMapByteOffset] |= 0x1 << outputParameter.ioMapBitOffset;
                //clear a bit in the output byte to 0
                else outputBytes[outputParameter.ioMapByteOffset] &= ~(0x1 << outputParameter.ioMapBitOffset);
            }
            else {
                //Logger::critical("Can't read non boolean values in VIPA modules");
                //we need to format the data to an integer and pass it to the NodePin object
            }
        }
    }
	 */
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
	
    //Loads saved ioModules and their data pins
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








void VipaBusCoupler_053_1EC01::addModule(std::shared_ptr<VipaModule> module){
	std::shared_ptr<VipaBusCoupler_053_1EC01> thisBusCoupler = std::dynamic_pointer_cast<VipaBusCoupler_053_1EC01>(shared_from_this());
	module->setParentBusCoupler(thisBusCoupler);
	module->setIndex(modules.size());
	modules.push_back(module);
	selectedModule = module;
	for(auto& inputPin : module->inputPins) addIoData(inputPin);
	for(auto& outputPin : module->outputPins) addIoData(outputPin);
	
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
	std::vector<std::shared_ptr<NodePin>>& inputPins = getNodeInputData();
	std::vector<std::shared_ptr<NodePin>>& outputPins = getNodeOutputData();
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
