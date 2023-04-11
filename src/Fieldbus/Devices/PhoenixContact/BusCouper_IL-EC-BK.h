#pragma once


#include "Fieldbus/Utilities/ModularDeviceProfile.h"

namespace PhoenixContact{

	class BusCoupler : public EtherCAT::ModularDeviceProfile::ModularDevice{
	public:
		DEFINE_ETHERCAT_DEVICE(BusCoupler, "Phoenix Contact", "I/O", 0x84, 0x293CAB)
		
		virtual void onConstruction() override;
		virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
			EtherCAT::ModularDeviceProfile::ModularDevice::onCopyFrom(source);
		};
		
		class PhoenixContactGpioDevice : public GpioInterface{
		public:
			PhoenixContactGpioDevice(std::shared_ptr<BusCoupler> busCoupler) : coupler(busCoupler){}
			std::shared_ptr<BusCoupler> coupler;
			virtual std::string getName() override{ return std::string(coupler->getName()) + " Gpio"; };
			virtual std::string getStatusString() override{ return "";}
		};
		
		//master GPIO Subdevice
		std::shared_ptr<PhoenixContactGpioDevice> gpioDevice;
		std::shared_ptr<NodePin> gpioDeviceLink;
		
		virtual std::vector<EtherCAT::ModularDeviceProfile::DeviceModule*>& getModuleFactory() override;
		
		virtual void beforeModuleReordering() override;
		
		//mandatory diagnostics TxPDO data
		uint16_t diagnosticsWord0;
		uint16_t diagnosticsWord1;
		uint16_t diagnosticsWord2;
		uint16_t diagnosticsWord3;
	};

}
