#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

#include <map>
#include <string>

class VIPA_053_1EC01 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(VIPA_053_1EC01, "053-1EC01", "VIPA 053-1EC01", "VIPA-053-1EC01", "Yaskawa", "I/O")

    struct ModuleParameter {
        uint16_t index;         //Coe Data Index (for display of pdodata)
        uint8_t subindex;       //Coe Data Subindex (for display of pdodata)
        int bitSize;            //for type matching and display of pdodata
        int ioMapByteOffset;    //byte index in the ioMap
        int ioMapBitOffset;     //bit offset in the ioMap byte
        std::shared_ptr<NodePin> nodePin;   //actual NodePin pin for nodegraph logic
    };

    
    //TODO: Add expected input and output count and size for each module type
    struct ModuleType {
        enum class Type {
            VIPA_022_1HD10, //4x Relais Output
            VIPA_021_1BF00, //8x Digital Input
            VIPA_050_1BS00, //SSI Input
            VIPA_032_1BD70, //4x 12bit Analog Input
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

    class Module {
    public:
        Module(ModuleType::Type t) : moduleType(t) {}
        ModuleType::Type moduleType;
        
        int inputByteCount = 0;
        int inputBitCount = 0;
        std::vector<ModuleParameter> inputs;

        int outputByteCount = 0;
        int outputBitCount = 0;
        std::vector<ModuleParameter> outputs;
    };

    //subdevices
    std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("GPIO");
    std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodeData::Type::GPIO_DEVICELINK, DataDirection::NODE_OUTPUT, "GPIO");
    std::vector<Module> modules;



    //std::vector<GpioDevice> 


    bool downloadDeviceModules(std::vector<Module>& output);

    void configureFromDeviceModules();
    DataTransferState::State configureFromDeviceModulesDownloadStatus = DataTransferState::State::NO_TRANSFER;
};
