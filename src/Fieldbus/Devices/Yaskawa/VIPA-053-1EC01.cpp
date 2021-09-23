#include <pch.h>

#include "VIPA-053-1EC01.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include <tinyxml2.h>

bool VIPA_053_1EC01::hasDeviceError() {
    return false;
}

const char* VIPA_053_1EC01::getDeviceErrorString() {
    return "";
}

void VIPA_053_1EC01::clearDeviceError() {
}

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
