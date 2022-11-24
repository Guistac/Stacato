#pragma once


#include "Fieldbus/Utilities/ModularDevice.h"
//#include "Fieldbus/Utilities/DeviceModule.h"

namespace PhoenixContact{

	class BusCoupler : public EtherCAT::ModularDevice{
	public:
		DEFINE_ETHERCAT_DEVICE(BusCoupler, "Phoenix Contact Bus Coupler", "IL_EC_BK_BusCoupler", "Phoenix Contact", "I/O", 0x84, 0x293CAB)
		
		class PhoenixContactGpioDevice : public GpioDevice{
		public:
			
			PhoenixContactGpioDevice(std::shared_ptr<BusCoupler> busCoupler) : GpioDevice(), coupler(busCoupler){}
			
			virtual std::string getName() override{ return std::string(coupler->getName()) + " Gpio"; };
			
			virtual std::string getStatusString() override{
				return "";
			}
			
			std::shared_ptr<BusCoupler> coupler;
		};
		
		//master GPIO Subdevice
		std::shared_ptr<PhoenixContactGpioDevice> gpioDevice;
		std::shared_ptr<NodePin> gpioDeviceLink = std::make_shared<NodePin>(NodePin::DataType::GPIO, NodePin::Direction::NODE_OUTPUT, "GPIO");
		
		virtual std::vector<EtherCAT::DeviceModule*>& getModuleFactory() override;
		
		virtual void beforeModuleReordering() override;
		
		//mandatory diagnostics TxPDO data
		uint16_t diagnosticsWord0;
		uint16_t diagnosticsWord1;
		uint16_t diagnosticsWord2;
		uint16_t diagnosticsWord3;
	};

/*
	class BusCouplerDiagnosticsModule : public EtherCAT::DeviceModule{
	public:
		
		DEFINE_DEVICE_MODULE(BusCouplerDiagnosticsModule, "BusCouplerDiagnosticsModule", "BusCouplerDiagnosticsModule", 0x0)

		uint16_t word1;
		uint16_t word2;
		uint16_t word3;
		uint16_t word4;
		
	};
*/

}
