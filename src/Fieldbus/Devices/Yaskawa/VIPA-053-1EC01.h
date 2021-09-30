#pragma once

#include "Fieldbus/EtherCatSlave.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

class VIPA_053_1EC01 : public EtherCatSlave {
public:

    SLAVE_DEFINITION(VIPA_053_1EC01, "053-1EC01", "Yaskawa", "I/O")

    struct ModuleParameter {
        uint16_t index;
        uint8_t subindex;
        uint8_t bitCount;
        int ioMapByteOffset; //byte index in the ioMap
        int ioMapBitOffset;  //bit offset in the ioMap byte
        std::shared_ptr<ioData> ioData;
    };

    struct SLIOModule {
        enum class Type {
            VIPA_022_1HD10,
            VIPA_021_1BF00,
            UNKNOWN_MODULE
        };
        Type type;
        char displayName[64];
        char saveName[64];
        char dataName[64];
    };
    static std::vector<SLIOModule> moduleTypes;
    SLIOModule* getModuleType(const char* saveName);
    SLIOModule* getModuleType(SLIOModule::Type type);

    struct Module {
        char name[64];
        uint32_t ID = 0x0;
        char SerialNumber[64];
        char ProductVersion[64];
        char HardwareVersion[64];
        char SoftwareVersion[64];
        uint16_t FPGAVersion = 0x0;
        char MxFile[64];

        SLIOModule::Type moduleType;

        bool b_hasInputs = false;
        int inputCount = 0;
        int inputByteCount = 0;
        int inputBitCount = 0;
        std::vector<ModuleParameter> inputs;

        bool b_hasOutputs = false;
        int outputCount = 0;
        int outputByteCount = 0;
        int outputBitCount = 0;
        std::vector<ModuleParameter> outputs;

        bool b_hasParameters = false;
        int parameterCount = 0x0;
        uint16_t parameterObject;
    };

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



    //std::vector<GpioDevice> 



    std::vector<Module> detectedModules;
    void detectIoModules();
};