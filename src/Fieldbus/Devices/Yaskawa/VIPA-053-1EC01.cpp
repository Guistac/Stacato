#include <pch.h>

#include "VIPA-053-1EC01.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>
#include "Gui/Gui.h"

bool VIPA_053_1EC01::isDeviceReady() {
    return true;
}

void VIPA_053_1EC01::enable() {
}

void VIPA_053_1EC01::disable() {
}

bool VIPA_053_1EC01::isEnabled() {
    return true;
}

void VIPA_053_1EC01::onDisconnection() {}
void VIPA_053_1EC01::onConnection() {}

void VIPA_053_1EC01::resetData() {}

void VIPA_053_1EC01::assignIoData() {
    //by default, this node only has one pin
    //no modules are loaded by default
    std::shared_ptr<Device> thisDevice = std::dynamic_pointer_cast<Device>(shared_from_this());
    gpioDevice->setParentDevice(thisDevice);
    gpioDeviceLink->set(gpioDevice);
    //gpio device link pin
    addIoData(gpioDeviceLink);
}


bool VIPA_053_1EC01::startupConfiguration() {

    //on startup configuration we try to match all modules to the slaves modules
    //if the modules are matching we are good to go
    //we then can safely upload the correct PDO mapping for the modules

    std::vector<Module> deviceModules;
    if (!downloadDeviceModules(deviceModules))return Logger::warn("Could not download Device Module Information");
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

    //===== PDO Mapping =====

    uint16_t RxPDO_SyncManager = 0x1C12;
    uint16_t TxPDO_SyncManager = 0x1C13;
    if (!writeSDO_U8(RxPDO_SyncManager, 0x0, 0)) return Logger::warn("Could not disable RxPDO Sync Manager"); //disable Sync Manager
    if (!writeSDO_U8(TxPDO_SyncManager, 0x0, 0)) return Logger::warn("Could not disable TxPDO Sync Manager"); //disable Sync Manager
    uint8_t RxPDOmoduleCount = 0;
    uint8_t TxPDOmoduleCount = 0;
    for (int i = 0; i < modules.size(); i++) {
        Module& module = modules[i];
        if (!module.outputs.empty()) {
            RxPDOmoduleCount++;
            uint16_t RxPDOmappingModule = 0x1600 + i;
            if (!writeSDO_U16(RxPDO_SyncManager, RxPDOmoduleCount, RxPDOmappingModule)) return Logger::warn("Could not assign RxPDO Mapping Module 0x{:X}", RxPDOmappingModule);
        }
        if (!module.inputs.empty()) {
            TxPDOmoduleCount++;
            uint16_t TxPDOmappingModule = 0x1A00 + i;
            if (!writeSDO_U16(TxPDO_SyncManager, TxPDOmoduleCount, TxPDOmappingModule)) return Logger::warn("Could not assign TxPDO Mapping Module 0x{:X}", TxPDOmappingModule);
        }
    }
    if (!writeSDO_U8(RxPDO_SyncManager, 0x0, RxPDOmoduleCount)) return Logger::warn("Could not assign RxPDO module count {}", RxPDOmoduleCount);
    if (!writeSDO_U8(TxPDO_SyncManager, 0x0, TxPDOmoduleCount)) return Logger::warn("Could not assign TxPDO module count {}", TxPDOmoduleCount);

    return true;
}



bool VIPA_053_1EC01::downloadDeviceModules(std::vector<Module>& output) {

    uint8_t detectedModuleCount;
    if (!readSDO_U8(0xF050, 0x0, detectedModuleCount)) return false;

    int inputByteCount = 0;
    int outputByteCount = 0;

    for (int i = 0; i < detectedModuleCount; i++) {

        uint16_t moduleInformationIndex = 0x4100 + i;   //contains the name of the module as well as its serial number and version info

        //download the name of the module to get the module type
        char nameBuffer[128];
        if (!readSDO_String(moduleInformationIndex, 0x1, nameBuffer,128)) return false;
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
                    Logger::critical("Can't Handle non boolean data in VIPA modules yet");
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
                    Logger::critical("Can't Handle non boolean data in VIPA modules yet");
                    //for non boolean values, this will need to be handled differently
                    //moduleParameter.NodePin = std::make_shared<NodePin>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, parameterName);
                }
            }
            //get the amount of full bytes the input section of the module fills (for example 11 bits are 2 bytes, with 5 unused bits)
            module.outputByteCount = ceil((double)module.outputBitCount / 8.0);
            outputByteCount += module.outputByteCount;
        }
    }

    return true;
}


void VIPA_053_1EC01::configureFromDeviceModules() {
    configureFromDeviceModulesDownloadStatus = DataTransferState::State::TRANSFERRING;

    std::vector<Module> deviceModules;
    if (!downloadDeviceModules(deviceModules)) {
        configureFromDeviceModulesDownloadStatus = DataTransferState::State::FAILED;
        return;
    }

    GuiMutex.lock();
    modules.clear();
    modules.swap(deviceModules);
    //remove all old NodePins from Node and NodeGraph
    std::vector<std::shared_ptr<NodePin>>& nodeInputData = getNodeInputData();
    while (!nodeInputData.empty()) removeIoData(nodeInputData.back());
    std::vector<std::shared_ptr<NodePin>>& nodeOutputData = getNodeOutputData();
    while (!nodeOutputData.empty()) removeIoData(nodeOutputData.back());
    //add new NodePins to Node and NodeGraph
    addIoData(gpioDeviceLink);
    for (Module& module : modules) {
        for (auto& input : module.inputs) addIoData(input.nodePin);
        for (auto& output : module.outputs) addIoData(output.nodePin);
    }
    GuiMutex.unlock();

    configureFromDeviceModulesDownloadStatus = DataTransferState::State::SUCCEEDED;
}


void VIPA_053_1EC01::readInputs() {
    uint8_t* inputBytes = identity->inputs;
    for (Module& module : modules) {
        for (auto inputParameter : module.inputs) {
            if (inputParameter.bitSize == 1) {
                bool value = 1 == (0x1 & (inputBytes[inputParameter.ioMapByteOffset] >> inputParameter.ioMapBitOffset));
                inputParameter.nodePin->set(value);
            }
            else {
                Logger::critical("Can't read non boolean values in VIPA modules");
                //we need to format the data to an integer and pass it to the NodePin object
            }
        }
    }
}


void VIPA_053_1EC01::prepareOutputs(){
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
                Logger::critical("Can't read non boolean values in VIPA modules");
                //we need to format the data to an integer and pass it to the NodePin object
            }
        }
    }
}

bool VIPA_053_1EC01::saveDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;

    XMLElement* ioModulesXML = xml->InsertNewChildElement("IOModules");
    for (Module& module : modules) {
        XMLElement* moduleXML = ioModulesXML->InsertNewChildElement("Module");
        moduleXML->SetAttribute("Type", getModuleType(module.moduleType)->saveName);

        XMLElement* moduleInputsXML = moduleXML->InsertNewChildElement("Inputs");
        for (ModuleParameter& inputParameter : module.inputs) {
            XMLElement* inputXML = moduleInputsXML->InsertNewChildElement("Input");
            inputXML->SetAttribute("PinSaveName", inputParameter.nodePin->getSaveName());
            inputXML->SetAttribute("PinDataType", getNodeDataType(inputParameter.nodePin->getType())->saveName);
            inputXML->SetAttribute("IOMapByteOffset", inputParameter.ioMapByteOffset);
            inputXML->SetAttribute("IOMapBitOffset", inputParameter.ioMapBitOffset);
            inputXML->SetAttribute("BitSize", inputParameter.bitSize);
            std::stringstream indexHex;
            indexHex << std::hex << inputParameter.index;
            inputXML->SetAttribute("Index", indexHex.str().c_str());
            std::stringstream subindexHex;
            subindexHex << std::hex << (uint16_t)inputParameter.subindex;
            inputXML->SetAttribute("Subindex", subindexHex.str().c_str());
        }

        XMLElement* moduleOutputsXML = moduleXML->InsertNewChildElement("Outputs");
        for (ModuleParameter& outputParameter : module.outputs) {
            XMLElement* outputXML = moduleOutputsXML->InsertNewChildElement("Output");
            outputXML->SetAttribute("PinSaveName", outputParameter.nodePin->getSaveName());
            outputXML->SetAttribute("PinDataType", getNodeDataType(outputParameter.nodePin->getType())->saveName);
            outputXML->SetAttribute("IOMapByteOffset", outputParameter.ioMapByteOffset);
            outputXML->SetAttribute("IOMapBitOffset", outputParameter.ioMapBitOffset);
            outputXML->SetAttribute("BitSize", outputParameter.bitSize);
            std::stringstream indexHex;
            indexHex << std::hex << outputParameter.index;
            outputXML->SetAttribute("Index", indexHex.str().c_str());
            std::stringstream subindexHex;
            subindexHex << std::hex << (uint16_t)outputParameter.subindex;
            outputXML->SetAttribute("Subindex", subindexHex.str().c_str());
        }
    }

    return true;
}

bool VIPA_053_1EC01::loadDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;

    //Loads saved ioModules and their data pins

    XMLElement* ioModulesXML = xml->FirstChildElement("IOModules");
    if (ioModulesXML == nullptr) return Logger::warn("Could not load IOModules attribute");

    XMLElement* moduleXML = ioModulesXML->FirstChildElement("Module");
    while (moduleXML != nullptr) {
        const char* moduleTypeString = "";
        moduleXML->QueryStringAttribute("Type", &moduleTypeString);
        if (getModuleType(moduleTypeString) == nullptr) return Logger::warn("Could not read Module Type Attribute");
        ModuleType::Type moduleType = getModuleType(moduleTypeString)->type;
        modules.push_back(Module(moduleType));
        Module& module = modules.back();

        XMLElement* moduleInputsXML = moduleXML->FirstChildElement("Inputs");
        if (moduleInputsXML == nullptr) return Logger::warn("Could not find Module Inputs Attribute");
        XMLElement* moduleInputXML = moduleInputsXML->FirstChildElement("Input");
        while (moduleInputXML != nullptr) {
            module.inputs.push_back(ModuleParameter());
            ModuleParameter& moduleParameter = module.inputs.back();
            if (moduleInputXML->QueryIntAttribute("IOMapByteOffset", &moduleParameter.ioMapByteOffset) != XML_SUCCESS) return Logger::warn("Could not load IOMapByteOffset Attribute");
            if (moduleInputXML->QueryIntAttribute("IOMapBitOffset", &moduleParameter.ioMapBitOffset) != XML_SUCCESS) return Logger::warn("Could not load IOMapBitOffset Attribute");
            if (moduleInputXML->QueryIntAttribute("BitSize", &moduleParameter.bitSize) != XML_SUCCESS) return Logger::warn("Could not load BitSize Attribute");
            const char* indexString;
            if (moduleInputXML->QueryStringAttribute("Index", &indexString) != XML_SUCCESS) return Logger::warn("Could not load Index Attribute");
            std::stringstream indexHex;
            indexHex << std::hex << indexString;
            indexHex >> moduleParameter.index;
            const char* subindexString;
            if (moduleInputXML->QueryStringAttribute("Subindex", &subindexString) != XML_SUCCESS) return Logger::warn("Could not load Subindex Attribute");
            std::stringstream subindexHex;
            subindexHex << std::hex << subindexString;
            subindexHex >> moduleParameter.subindex;
            const char* saveNameString;
            const char* dataTypeString;
            if(moduleInputXML->QueryStringAttribute("PinSaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load PinSaveName Attribute");
            moduleInputXML->QueryStringAttribute("PinDataType", &dataTypeString);
            if (getNodeDataType(dataTypeString) == nullptr) return Logger::warn("Could not read PinDataType attribute");
            moduleParameter.nodePin = std::make_shared<NodePin>(getNodeDataType(dataTypeString)->type, DataDirection::NODE_OUTPUT, saveNameString);
            //get next input xml
            moduleInputXML = moduleInputXML->NextSiblingElement("Input");
        }

        XMLElement* moduleOutputsXML = moduleXML->FirstChildElement("Outputs");
        if (moduleOutputsXML == nullptr) return Logger::warn("Could not find Module Outputs Attribute");
        XMLElement* moduleOutputXML = moduleOutputsXML->FirstChildElement("Output");
        while (moduleOutputXML != nullptr) {
            module.outputs.push_back(ModuleParameter());
            ModuleParameter& moduleParameter = module.outputs.back();
            if (moduleOutputXML->QueryIntAttribute("IOMapByteOffset", &moduleParameter.ioMapByteOffset) != XML_SUCCESS) return Logger::warn("Could not load IOMapByteOffset Attribute");
            if (moduleOutputXML->QueryIntAttribute("IOMapBitOffset", &moduleParameter.ioMapBitOffset) != XML_SUCCESS) return Logger::warn("Could not load IOMapBitOffset Attribute");
            if (moduleOutputXML->QueryIntAttribute("BitSize", &moduleParameter.bitSize) != XML_SUCCESS) return Logger::warn("Could not load BitSize Attribute");
            const char* indexString;
            if (moduleOutputXML->QueryStringAttribute("Index", &indexString) != XML_SUCCESS) return Logger::warn("Could not load Index Attribute");
            std::stringstream indexHex;
            indexHex << std::hex << indexString;
            indexHex >> moduleParameter.index;
            const char* subindexString;
            if (moduleOutputXML->QueryStringAttribute("Subindex", &subindexString) != XML_SUCCESS) return Logger::warn("Could not load Subindex Attribute");
            std::stringstream subindexHex;
            subindexHex << std::hex << subindexString;
            subindexHex >> moduleParameter.subindex;
            const char* saveNameString;
            const char* dataTypeString;
            if (moduleOutputXML->QueryStringAttribute("PinSaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Could not load PinSaveName Attribute");
            moduleOutputXML->QueryStringAttribute("PinDataType", &dataTypeString);
            if (getNodeDataType(dataTypeString) == nullptr) return Logger::warn("Could not read PinDataType attribute");
            moduleParameter.nodePin = std::make_shared<NodePin>(getNodeDataType(dataTypeString)->type, DataDirection::NODE_INPUT, saveNameString);
            //get next output xml
            moduleOutputXML = moduleOutputXML->NextSiblingElement("Output");
        }

        moduleXML = moduleXML->NextSiblingElement("Module");
    }

    //add loaded pins to the nodes input and output pin lists
    //pins will get the rest of their attributes, such as unique id, attributed later
    for (Module& module : modules) {
        for (auto& input : module.inputs) addIoData(input.nodePin);
        for (auto& output : module.outputs) addIoData(output.nodePin);
    }

    return true;
}









std::vector<VIPA_053_1EC01::ModuleType> VIPA_053_1EC01::moduleTypes = {
    {VIPA_053_1EC01::ModuleType::Type::VIPA_022_1HD10, "DO4x Relais (1.8A)", "VIPA 022-1HD10", "Relais"},
    {VIPA_053_1EC01::ModuleType::Type::VIPA_021_1BF00, "DI8x (DC24V)", "VIPA 021-1BF00", "Digital"},
    {VIPA_053_1EC01::ModuleType::Type::UNKNOWN_MODULE, "Unknown Module", "UnknownModule", "None"}
};

VIPA_053_1EC01::ModuleType* VIPA_053_1EC01::getModuleType(const char* saveName) {
    for (auto& sliomodule : moduleTypes) {
        if (strcmp(sliomodule.saveName, saveName) == 0) return &sliomodule;
    }
    return &moduleTypes.back();
}

VIPA_053_1EC01::ModuleType* VIPA_053_1EC01::getModuleType(ModuleType::Type type) {
    for (auto& sliomodule : moduleTypes) {
        if (sliomodule.type == type) return &sliomodule;
    }
    return &moduleTypes.back();
}