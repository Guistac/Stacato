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
			Lexium32::createInstance(),
			Lexium32i::createInstance(),
			ATV320::createInstance(),
			ATV340::createInstance(),
			VipaBusCoupler_053_1EC01::createInstance(),
			PhoenixContact::BusCoupler::createInstance(),
			PD4_E::createInstance(),
			MicroFlex_e190::createInstance(),
			CU1128::createInstance(),
			CU1124::createInstance(),
            ECAT_2511_A_FiberConverter::createInstance(),
            ECAT_2511_B_FiberConverter::createInstance(),
			ECAT_2515_6PortJunction::createInstance()
		};

		//sort devices by manufacturer
		for (auto device : allEtherCatDevices) {
			bool manufacturerExists = false;
			for (NodeGroup& group : etherCatdevicesByManufacturer) {
				if (group.name == device->getManufacturerName()) {
					manufacturerExists = true;
					group.nodes.push_back(device);
					break;
				}
			}
			if (!manufacturerExists) {
				etherCatdevicesByManufacturer.push_back(NodeGroup());
				etherCatdevicesByManufacturer.back().name = device->getManufacturerName();
				etherCatdevicesByManufacturer.back().nodes.push_back(device);
			}
		}

		//sort device by manufacturer
		for (auto device : allEtherCatDevices) {
			bool categoryExists = false;
			for (NodeGroup& group : etherCatdevicesByCategory) {
				if (group.name == device->getNodeCategory()) {
					categoryExists = true;
					group.nodes.push_back(device);
					break;
				}
			}
			if (!categoryExists) {
				etherCatdevicesByCategory.push_back(NodeGroup());
				etherCatdevicesByCategory.back().name = device->getNodeCategory();
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
