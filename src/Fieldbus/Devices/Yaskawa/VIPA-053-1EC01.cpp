#include <pch.h>

#include "VIPA-053-1EC01.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

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
    addIoData(relay1);
    addIoData(relay2);
    addIoData(relay3);
    addIoData(relay4);
    
    //node output data
    addIoData(gpioLink);
    addIoData(digitalIn1);
    addIoData(digitalIn2);
    addIoData(digitalIn3);
    addIoData(digitalIn4);
    addIoData(digitalIn5);
    addIoData(digitalIn6);
    addIoData(digitalIn7);
    addIoData(digitalIn8);

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


void VIPA_053_1EC01::readInputs() {
}


void VIPA_053_1EC01::prepareOutputs(){
}

bool VIPA_053_1EC01::saveDeviceData(tinyxml2::XMLElement* xml) {
    return true;
}

bool VIPA_053_1EC01::loadDeviceData(tinyxml2::XMLElement* xml) {
    return true;
}
