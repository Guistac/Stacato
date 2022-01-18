#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

class VipaModule;


class VipaBusCoupler_053_1EC01 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(VipaBusCoupler_053_1EC01, "053-1EC01", "Vipa Bus Coupler (053_1EC01)", "VipaBusCoupler-053-1EC01", "Yaskawa", "I/O")
	
    //master GPIO Subdevice
    std::shared_ptr<GpioDevice> gpioDevice = std::make_shared<GpioDevice>("GPIO");
    std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_OUTPUT, "GPIO");
	
	//modules and module management
	std::vector<std::shared_ptr<VipaModule>> modules;
	void addModule(std::shared_ptr<VipaModule> module);
	void removeModule(std::shared_ptr<VipaModule> module);
	void reorderModule(int oldIndex, int newIndex);
	void moveModuleUp(std::shared_ptr<VipaModule> module);
	void moveModuleDown(std::shared_ptr<VipaModule> module);
	std::shared_ptr<VipaModule> selectedModule = nullptr;
	
	//module configuration
    bool downloadDeviceModules(std::vector<std::shared_ptr<VipaModule>>& output);
    void configureFromDeviceModules();
    DataTransferState configureFromDeviceModulesDownloadStatus = DataTransferState::NO_TRANSFER;
};

