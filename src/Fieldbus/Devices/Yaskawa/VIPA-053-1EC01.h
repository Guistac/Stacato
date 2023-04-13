#pragma once

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/ScrollingBuffer.h"

class VipaModule;


class VipaBusCoupler_053_1EC01 : public EtherCatDevice {
public:

    DEFINE_ETHERCAT_DEVICE(VipaBusCoupler_053_1EC01, "Yaskawa", "I/O", 0x22B, 0x531EC01)
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	
	virtual void onConstruction() override;
	
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		EtherCatDevice::onCopyFrom(source);
	}
	
	class VipaGpioDevice : public GpioInterface{
	public:
		
		VipaGpioDevice(std::shared_ptr<VipaBusCoupler_053_1EC01> busCoupler) : coupler(busCoupler){}
		
		virtual std::string getName() override{ return std::string(coupler->getName()) + " Gpio"; };
		
		virtual std::string getStatusString() override{
			return "";
		}
		
		std::shared_ptr<VipaBusCoupler_053_1EC01> coupler;
	};
	
    //master GPIO Subdevice
	std::shared_ptr<VipaGpioDevice> gpioDevice;
	std::shared_ptr<NodePin> gpioDeviceLink;
	
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

