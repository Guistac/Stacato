#pragma once

#include "Fieldbus/EtherCatSlave.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

class VIPA_053_1EC01 : public EtherCatSlave {
public:

    SLAVE_DEFINITION(VIPA_053_1EC01, "053-1EC01", "Yaskawa", "I/O")

    struct ModuleParameter {
        uint16_t index;         //for display of pdodata
        uint8_t subindex;       //for display of pdodata
        uint8_t bitCount;       //for display of pdodata and type matching
        int ioMapByteOffset;    //byte index in the ioMap
        int ioMapBitOffset;     //bit offset in the ioMap byte
        char saveName[64];      //used to match NodePin pin
        NodeData::Type dataType;      //used to match NodePin pin
        std::shared_ptr<NodePin> NodePin; //actual NodePin pin for nodegraph logic
    };

    struct ModuleType {
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
    static std::vector<ModuleType> moduleTypes;
    ModuleType* getModuleType(const char* saveName);
    ModuleType* getModuleType(ModuleType::Type type);

    struct Module {

        Module(ModuleType::Type t) : moduleType(t) {}

        ModuleType::Type moduleType;

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

    std::shared_ptr<NodePin> gpNodeLink = std::make_shared<NodePin>(NodeData::Type::GPIO_DEVICELINK, DataDirection::NODE_OUTPUT, "GPIO");
    std::vector<Module> ioModules;



    //std::vector<GpioDevice> 



    void detectIoModules();
};