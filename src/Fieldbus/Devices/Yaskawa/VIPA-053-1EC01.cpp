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

void VIPA_053_1EC01::assignIoData() {
    std::shared_ptr<DeviceNode> thisDevice = std::dynamic_pointer_cast<DeviceNode>(shared_from_this());
    gpioDevice->setParentDevice(thisDevice);
    gpioLink->set(gpioDevice);

    //node input data
    //addIoData(relay1);
    //addIoData(relay2);
    //addIoData(relay3);
    //addIoData(relay4);
    
    //node output data
    addIoData(gpioLink);
    //addIoData(digitalIn1);
    //addIoData(digitalIn2);
    //addIoData(digitalIn3);
    //addIoData(digitalIn4);
    //addIoData(digitalIn5);
    //addIoData(digitalIn6);
    //addIoData(digitalIn7);
    //addIoData(digitalIn8);

    rxPdoAssignement.addNewModule(0x1603);
    //rxPdoAssignement.addEntry(0x6040, 0x0, 2, "DCOMcontrol", &DCOMcontrol);

    txPdoAssignement.addNewModule(0x1A03);
    //txPdoAssignement.addEntry(0x6041, 0x0, 2, "_DCOMstatus", &_DCOMstatus);
}


bool VIPA_053_1EC01::startupConfiguration() {


    std::vector<uint16_t> inputModules;
    std::vector<uint16_t> outputModules;

    for (int i = 0; i < 64; i++) {
        uint16_t inputModuleIndex = 0x6000 + i;
        uint16_t outputModuleIndex = 0x7000 + i;
        uint16_t parameterIndex = 0x3100 + i;
        bool hasOutput = false;
        bool hasInput = false;
        bool hasParameter = false;
        uint8_t inputDataSize;
        if (readSDO_U8(inputModuleIndex, 0x0, inputDataSize)) {
            hasInput = true;
            inputModules.push_back(0x1A00 + i);
            Logger::warn("Input Data: {:X} size: {}", inputModuleIndex, inputDataSize);
        }
        uint8_t outputDataSize;
        if (readSDO_U8(outputModuleIndex, 0x0, outputDataSize)) {
            hasOutput = true;
            outputModules.push_back(0x1600 + i);
            Logger::warn("Output Data: {:X} size: {}", outputModuleIndex, outputDataSize);
        }
        uint8_t parameterDataSize;
        if (readSDO_U8(parameterIndex, 0x0, parameterDataSize)) {
            hasParameter = true;
            Logger::warn("Parameter Data: {:X} size: {}", parameterIndex, parameterDataSize);
        }
        if (!hasInput && !hasOutput) break;
    }


    
    uint16_t RxPDO = 0x1C12;
    if(!writeSDO_U8(RxPDO, 0x0, 0)) return false; //disable Sync Manager
    uint8_t RxPDOmoduleCount = 0;
    for (uint16_t outputModuleIndex : outputModules) {
        RxPDOmoduleCount++;
        if (!writeSDO_U16(RxPDO, RxPDOmoduleCount, outputModuleIndex)) return false;
        Logger::warn("Set RxPDOmodule {:X} to index {}", outputModuleIndex, RxPDOmoduleCount);
    }
    if (!writeSDO_U8(RxPDO, 0x0, RxPDOmoduleCount)) return false;

    uint16_t TxPDO = 0x1C13;
    if (!writeSDO_U8(TxPDO, 0x0, 0)) return false; //disable Sync Manager
    uint8_t TxPDOmoduleCount = 0;
    for (uint16_t inputModuleIndex : inputModules) {
        TxPDOmoduleCount++;
        if (!writeSDO_U16(TxPDO, TxPDOmoduleCount, inputModuleIndex)) return false;
        Logger::warn("Set TxPDOmodule {:X} to index {}", inputModuleIndex, TxPDOmoduleCount);
    }
    if (!writeSDO_U8(TxPDO, 0x0, TxPDOmoduleCount)) return false;

    return true;
}



void VIPA_053_1EC01::detectIoModules() {

    uint8_t detectedModuleCount;
    if(!readSDO_U8(0xF050, 0x0, detectedModuleCount)) return;
    std::vector<Module> newModules;

    int inputByteCount = 0;
    int outputByteCount = 0;

    for (int i = 0; i < detectedModuleCount; i++) {

        uint16_t moduleInformationIndex = 0x4100 + i;   //contains the name of the module as well as its serial number and version info
        uint16_t inputObjectIndex = 0x6000 + i;         //index of the object that holds all input values, index holds number of input parameters
        uint16_t inputMappingModule = 0x1A00 + i;       //index of the mapping module object that stores all input PDO mappings
        uint16_t outputObjectIndex = 0x7000 + i;        //index of the object that holds all output values, index holds number of output parameters
        uint16_t outputMappingModule = 0x1600 + i;      //index of the mapping module object that stores all output PDO mappings
        //uint16_t parameterObjectIndex = 0x3100 + i;   //index of the object that stores all module parameters (there may not be one for every module)

        char nameBuffer[128];
        if (!readSDO_String(moduleInformationIndex, 0x1, nameBuffer)) return;
        ModuleType::Type moduleType = getModuleType(nameBuffer)->type;
        newModules.push_back(Module(moduleType));
        Module& module = newModules.back();

        uint8_t inputCount;
        if (readSDO_U8(inputObjectIndex, 0x0, inputCount)) {
            module.b_hasInputs = true;
            module.inputCount = inputCount;
            module.inputs = std::vector<ModuleParameter>(inputCount);
            module.inputBitCount = 0;
            for (int j = 0; j < inputCount; j++) {
                ModuleParameter& moduleParameter = module.inputs[j];
                uint32_t parameterMapping;
                readSDO_U32(inputMappingModule, j + 1, parameterMapping);
                moduleParameter.index = (parameterMapping & 0xFFFF0000) >> 16;
                moduleParameter.subindex = (parameterMapping & 0xFF00) >> 8;
                moduleParameter.bitCount = (parameterMapping & 0xFF);
                moduleParameter.ioMapByteOffset = inputByteCount + module.inputBitCount / 8;    //Byte index of the data in the slaves ioMap
                moduleParameter.ioMapBitOffset = module.inputBitCount % 8;                      //Bit index in the byte of the ioMap
                module.inputBitCount += moduleParameter.bitCount;
                char parameterDisplayName[64];
                sprintf(parameterDisplayName, "Module %i %s Input %i", i+1, getModuleType(module.moduleType)->dataName, j+1);
                sprintf(moduleParameter.saveName, "Module%i%sInput%i", i+1, getModuleType(module.moduleType)->dataName, j+1);
                if (moduleParameter.bitCount == 1) moduleParameter.ioData = std::make_shared<ioData>(ioDataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, parameterDisplayName, moduleParameter.saveName);
                else {
                    Logger::critical("Can't Handle non boolean data in VIPA modules");
                    //TODO: for non boolean values, this will need to be handled differently
                    //moduleParameter.ioData = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_OUTPUT, paramet);
                }
            }
            //get the amount of full bytes the input section of the module fills (for example 4 bits are 1 byte, with 4 unused bits)
            module.inputByteCount = ceil((double)module.inputBitCount / 8.0);
            inputByteCount += module.inputByteCount;
        }

        uint8_t outputCount;
        if (readSDO_U8(outputObjectIndex, 0x0, outputCount)) {
            module.b_hasOutputs = true;
            module.outputCount = outputCount;
            module.outputs = std::vector<ModuleParameter>(outputCount);
            module.outputBitCount = 0;
            for (int j = 0; j < outputCount; j++) {
                ModuleParameter& moduleParameter = module.outputs[j];
                uint32_t parameterMapping;
                readSDO_U32(outputMappingModule, j + 1, parameterMapping);
                moduleParameter.index = (parameterMapping & 0xFFFF0000) >> 16;
                moduleParameter.subindex = (parameterMapping & 0xFF00) >> 8;
                moduleParameter.bitCount = (parameterMapping & 0xFF);
                moduleParameter.ioMapByteOffset = outputByteCount + module.outputBitCount / 8;  //Byte index of the data in the slaves ioMap
                moduleParameter.ioMapBitOffset = module.outputBitCount % 8;                     //Bit index in the byte of the ioMap
                module.outputBitCount += moduleParameter.bitCount;
                char parameterDisplayName[128];
                sprintf(parameterDisplayName, "Module %i %s Output %i", i+1, getModuleType(module.moduleType)->dataName, j+1);
                sprintf(moduleParameter.saveName, "Module%i%sOutput%i", i+1, getModuleType(module.moduleType)->dataName, j+1);
                if (moduleParameter.bitCount == 1) moduleParameter.ioData = std::make_shared<ioData>(ioDataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, parameterDisplayName, moduleParameter.saveName);
                else {
                    Logger::critical("Can't Handle non boolean data in VIPA modules");
                    //TODO: for non boolean values, this will need to be handled differently
                    //moduleParameter.ioData = std::make_shared<ioData>(DataType::INTEGER_VALUE, DataDirection::NODE_INPUT, parameterName);
                }
            }
            //get the amount of full bytes the input section of the module fills (for example 11 bits are 2 bytes, with 5 unused bits)
            module.outputByteCount = ceil((double)module.outputBitCount / 8.0);
            outputByteCount += module.outputByteCount;
        }

        //uint8_t parameterCount;
        //if (readSDO_U8(0x3100 + i, 0x0, parameterCount)) {
        //    module.b_hasParameters = true;
        //    module.parametersObject = 0x3100 + i;
        //}
        //else module.b_hasParameters = false;
    }

    GuiMutex.lock();
    ioModules.clear();
    ioModules.swap(newModules);
    //remove all old ioData
    std::vector<std::shared_ptr<ioData>>& nodeInputData = getNodeInputData();
    while (!nodeInputData.empty()) removeIoData(nodeInputData.back());
    std::vector<std::shared_ptr<ioData>>& nodeOutputData = getNodeOutputData();
    while (!nodeOutputData.empty()) removeIoData(nodeOutputData.back());
    //add new ioData
    addIoData(gpioLink);
    for (Module& module : ioModules) {
        for (auto& input : module.inputs) addIoData(input.ioData);
        for (auto& output : module.outputs) addIoData(output.ioData);
    }
    GuiMutex.unlock();

}



void VIPA_053_1EC01::readInputs() {
    uint8_t* inputBytes = identity->inputs;
    for (Module& module : ioModules) {
        for (auto inputParameter : module.inputs) {
            if (inputParameter.bitCount == 1) {
                bool value = 1 == (0x1 & (inputBytes[inputParameter.ioMapByteOffset] >> inputParameter.ioMapBitOffset));
                inputParameter.ioData->set(value);
            }
            else {
                Logger::critical("Can't read non boolean values in VIPA modules");
                //we need to format the data to an integer and pass it to the ioData object
            }
        }
    }
}


void VIPA_053_1EC01::prepareOutputs(){
    uint8_t* outputBytes = identity->outputs;
    for (Module& module : ioModules) {
        for (auto outputParameter : module.outputs) {
            if (outputParameter.bitCount == 1) {
                std::shared_ptr<ioData> pin = outputParameter.ioData;
                if (pin->isConnected()) pin->set(pin->getLinks().front()->getInputData()->getBoolean());
                bool value = pin->getBoolean();
                //set a bit in the output byte to 1
                if (outputParameter.ioData->getBoolean()) outputBytes[outputParameter.ioMapByteOffset] |= 0x1 << outputParameter.ioMapBitOffset;
                //clear a bit in the output byte to 0
                else outputBytes[outputParameter.ioMapByteOffset] &= ~(0x1 << outputParameter.ioMapBitOffset);
            }
            else {
                Logger::critical("Can't read non boolean values in VIPA modules");
                //we need to format the data to an integer and pass it to the ioData object
            }
        }
    }
}

bool VIPA_053_1EC01::saveDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;

    XMLElement* ioModulesXML = xml->InsertNewChildElement("IOModules");
    for (Module& module : ioModules) {
        XMLElement* moduleXML = ioModulesXML->InsertNewChildElement("Module");
        moduleXML->SetAttribute("Type", getModuleType(module.moduleType)->saveName);

        XMLElement* moduleInputs = moduleXML->InsertNewChildElement("Inputs");
        for (ModuleParameter& inputParameter : module.inputs) {
            XMLElement* inputXML = moduleInputs->InsertNewChildElement("Input");
            inputXML->SetAttribute("SaveName", inputParameter.saveName);
            inputXML->SetAttribute("IOMapByteOffset", inputParameter.ioMapByteOffset);
            inputXML->SetAttribute("IOMapBitOffset", inputParameter.ioMapBitOffset);
        }

        XMLElement* moduleOutputs = moduleXML->InsertNewChildElement("Outputs");
        for (ModuleParameter& outputParameter : module.outputs) {
            XMLElement* outputXML = moduleOutputs->InsertNewChildElement("Output");
            outputXML->SetAttribute("SaveName", outputParameter.saveName);
            outputXML->SetAttribute("IOMapByteOffset", outputParameter.ioMapByteOffset);
            outputXML->SetAttribute("IOMapBitOffset", outputParameter.ioMapBitOffset);
        }
    }

    return true;
}

bool VIPA_053_1EC01::loadDeviceData(tinyxml2::XMLElement* xml) {
    using namespace tinyxml2;

    XMLElement* ioModulesXML = xml->FirstChildElement("IOModules");
    if (ioModulesXML == nullptr) return Logger::warn("Could not load IOModules attribute");

    XMLElement* moduleXML = ioModulesXML->FirstChildElement("Module");

    while (moduleXML != nullptr) {
        const char* moduleTypeString = "";
        moduleXML->QueryStringAttribute("Type", &moduleTypeString);
        if (getModuleType(moduleTypeString) == nullptr) return Logger::warn("Could not read Module Type Attribute");
        ModuleType::Type moduleType = getModuleType(moduleTypeString)->type;
        ioModules.push_back(Module(moduleType));
        Module& module = ioModules.back();

        XMLElement* moduleInputsXML = moduleXML->FirstChildElement("Inputs");
        if (moduleInputsXML == nullptr) return Logger::warn("Could not find Module Inputs Attribute");
        XMLElement* moduleInputXML = moduleInputsXML->FirstChildElement("Input");
        while (moduleInputXML != nullptr) {
            module.inputs.push_back(ModuleParameter());
            ModuleParameter& moduleParameter = module.inputs.back();
            if (moduleInputXML->QueryIntAttribute("IOMapByteOffset", &moduleParameter.ioMapByteOffset) != XML_SUCCESS) return Logger::warn("Could not load IOMapByteOffset Attribute");
            if (moduleInputXML->QueryIntAttribute("IOMapBitOffset", &moduleParameter.ioMapBitOffset) != XML_SUCCESS) return Logger::warn("Could not load IOMapBitOffset Attribute");
            const char* saveNameString;
            if(moduleInputXML->QueryStringAttribute("SaveName", &saveNameString) != XML_SUCCESS) return Logger::warn("Couldnot load SaveName Attribute");
            strcpy(moduleParameter.saveName, saveNameString);
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

            moduleOutputXML = moduleOutputXML->NextSiblingElement("Output");
        }

        moduleXML = moduleXML->NextSiblingElement("Module");
    }

    for (Module& module : ioModules) {
        for (auto& input : module.inputs) addIoData(input.ioData);
        for (auto& output : module.outputs) addIoData(output.ioData);
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