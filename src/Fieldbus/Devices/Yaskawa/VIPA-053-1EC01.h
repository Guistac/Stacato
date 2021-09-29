#pragma once

#include "Fieldbus/EtherCatSlave.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

struct VIPAparameter {
    uint16_t mappingModule;
    uint8_t subindex;
    uint8_t bitCount;
};

struct VIPAModule {
    char name[64];

    uint32_t ID;

    bool b_hasInputs;
    bool b_hasOutputs;
    bool b_hasParameters;

    uint16_t inputsObject;
    uint16_t outputsObject;
    uint16_t parametersObject;

    std::vector<VIPAparameter> inputParameters;
    std::vector<VIPAparameter> outputParameters;

};

class VIPA_053_1EC01 : public EtherCatSlave {
public:

    SLAVE_DEFINITION(VIPA_053_1EC01, "053-1EC01", "Yaskawa", "I/O")

    //subdevices
    std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("GPIO");

    //node input data
    std::shared_ptr<ioData> relay1 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Relay-1");
    std::shared_ptr<ioData> relay2 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Relay-2");
    std::shared_ptr<ioData> relay3 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Relay-3");
    std::shared_ptr<ioData> relay4 =        std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_INPUT, "Relay-4");

    //node output data
    std::shared_ptr<ioData> digitalIn1 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-1", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn2 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-2", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn3 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-3", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn4 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-4", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn5 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-5", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn6 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-6", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn7 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-7", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> digitalIn8 =    std::make_shared<ioData>(DataType::BOOLEAN_VALUE, DataDirection::NODE_OUTPUT, "Input-8", ioDataFlags_DisableDataField);
    std::shared_ptr<ioData> gpioLink =      std::make_shared<ioData>(DataType::GPIO_DEVICELINK, DataDirection::NODE_OUTPUT, "GPIO");





    std::vector<VIPAModule> detectedModules;
    void detectIoModules();
};