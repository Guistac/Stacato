#include <pch.h>

#include "NodeFactory.h"

#include "Fieldbus/Devices/SchneiderElectric/Lexium32.h"
#include "Fieldbus/Devices/SchneiderElectric/Lexium32i.h"
#include "Fieldbus/Devices/SchneiderElectric/ATV320.h"
#include "Fieldbus/Devices/SchneiderElectric/ATV340.h"
#include "Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.h"
#include "Fieldbus/Devices/Nanotec/PD4_E.h"
#include "Fieldbus/Devices/ABB/MicroFlexE190.h"
#include "Fieldbus/Devices/PhoenixContact/BusCouper_IL-EC-BK.h"
#include "Fieldbus/Devices/Beckhoff/EtherCatJunctions.h"
#include "Fieldbus/Devices/ICPDAS/EtherCatFiberConverter.h"

namespace NodeFactory{
	
	std::vector<std::shared_ptr<EtherCatDevice>> allEtherCatDevices;
	std::vector<NodeGroup> etherCatdevicesByManufacturer;
	std::vector<NodeGroup> etherCatdevicesByCategory;

	void loadEtherCatNodes(std::vector<std::shared_ptr<Node>>& nodeList) {
		
		allEtherCatDevices = {
			//new EtherCatDevice(),
			/*
			Lexium32::createInstance().get(),
			Lexium32i::createInstance().get(),
			ATV320::createInstance().get(),
			ATV340::createInstance().get(),
			VipaBusCoupler_053_1EC01::createInstance().get(),
			PhoenixContact::BusCoupler::createInstance().get(),
			PD4_E::createInstance().get(),
			MicroFlex_e190::createInstance().get(),
			CU1128::createInstance().get(),
			CU1124::createInstance().get(),
            ECAT_2511_A_FiberConverter::createInstance().get(),
            ECAT_2511_B_FiberConverter::createInstance().get(),
			ECAT_2515_6PortJunction::createInstance().get()
			 */
		};

		//sort devices by manufacturer
		for (auto device : allEtherCatDevices) {
			const char * manufacturer = device->getManufacturerName();
			bool manufacturerExists = false;
			for (NodeGroup& group : etherCatdevicesByManufacturer) {
				if (strcmp(manufacturer, group.name) == 0) {
					manufacturerExists = true;
					group.nodes.push_back(device);
					break;
				}
			}
			if (!manufacturerExists) {
				etherCatdevicesByManufacturer.push_back(NodeGroup());
				strcpy(etherCatdevicesByManufacturer.back().name, device->getManufacturerName());
				etherCatdevicesByManufacturer.back().nodes.push_back(device);
			}
		}

		//sort device by manufacturer
		for (auto device : allEtherCatDevices) {
			const char* deviceCategory = device->getNodeCategory();
			bool categoryExists = false;
			for (NodeGroup& group : etherCatdevicesByCategory) {
				if (strcmp(deviceCategory, group.name) == 0) {
					categoryExists = true;
					group.nodes.push_back(device);
					break;
				}
			}
			if (!categoryExists) {
				etherCatdevicesByCategory.push_back(NodeGroup());
				strcpy(etherCatdevicesByCategory.back().name, device->getNodeCategory());
				etherCatdevicesByCategory.back().nodes.push_back(device);
			}
		}
		
		nodeList.insert(nodeList.end(), allEtherCatDevices.begin(), allEtherCatDevices.end());
	}

	std::shared_ptr<EtherCatDevice> getEtherCatDeviceByIdCodes(uint32_t manufacturerCode, uint32_t identificationCode){
		for(auto device : allEtherCatDevices){
			if(device->getManufacturerCode() == manufacturerCode && device->getIdentificationCode() == identificationCode){
				return std::static_pointer_cast<EtherCatDevice>(device->duplicate());
			}
		}
		return EtherCatDevice::createInstance();
	}

	const std::vector<NodeGroup>& getEtherCatDevicesByManufacturer() { return etherCatdevicesByManufacturer; }
	const std::vector<NodeGroup>& getEtherCatDevicesByCategory() { return etherCatdevicesByCategory; }

}
