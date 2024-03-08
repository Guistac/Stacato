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
#include "Fieldbus/Devices/Beckhoff/EL7221-9014-MotionInterface.h"
#include "Fieldbus/Devices/Beckhoff/EL7222-0010.h"
#include "Fieldbus/Devices/ICPDAS/EtherCatFiberConverter.h"
#include "Fieldbus/Devices/Kinco/KincoFD.h"

namespace NodeFactory{
	
	std::vector<EtherCatDevice*> allEtherCatDevices;
	std::vector<NodeGroup> etherCatdevicesByManufacturer;
	std::vector<NodeGroup> etherCatdevicesByCategory;

	void loadEtherCatNodes(std::vector<Node*>& nodeList) {
		
		allEtherCatDevices = {
			//new EtherCatDevice(),
			new Lexium32(),
			new Lexium32i(),
			new ATV320(),
			new ATV340(),
			new VipaBusCoupler_053_1EC01(),
			new PhoenixContact::BusCoupler(),
			new PD4_E(),
			new MicroFlex_e190(),
			new CU1128(),
			new CU1124(),
			new EK1100(),
			new EK1122(),
			new EL5001(),
			new EL1008(),
			new EL2008(),
			new EL2912(),
			new EL2624(),
			new EL7221_9014(),
			new EL7222_0010(),
            new ECAT_2511_A_FiberConverter(),
            new ECAT_2511_B_FiberConverter(),
			new ECAT_2515_6PortJunction(),
			new KincoFD()
		};

		//sort devices by manufacturer
		for (EtherCatDevice* device : allEtherCatDevices) {
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
		for (EtherCatDevice* device : allEtherCatDevices) {
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
		for(EtherCatDevice* device : allEtherCatDevices){
			if(device->getManufacturerCode() == manufacturerCode && device->getIdentificationCode() == identificationCode){
				return std::static_pointer_cast<EtherCatDevice>(device->getNewInstance());
			}
		}
		return std::make_shared<EtherCatDevice>();
	}

	const std::vector<NodeGroup>& getEtherCatDevicesByManufacturer() { return etherCatdevicesByManufacturer; }
	const std::vector<NodeGroup>& getEtherCatDevicesByCategory() { return etherCatdevicesByCategory; }

}
