#include <pch.h>

#include "NodeFactory.h"

#include "Fieldbus/Devices/SchneiderElectric/Lexium32.h"
#include "Fieldbus/Devices/SchneiderElectric/ATV320.h"
#include "Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.h"
#include "Fieldbus/Devices/Nanotec/PD4_E.h"

namespace NodeFactory{
	
	std::vector<EtherCatDevice*> allEtherCatDevices;
	std::vector<NodeGroup> etherCatdevicesByManufacturer;
	std::vector<NodeGroup> etherCatdevicesByCategory;

	void loadEtherCatNodes(std::vector<Node*>& nodeList) {
		
		allEtherCatDevices = {
			new Lexium32(),
			new ATV320(),
			new VipaBusCoupler_053_1EC01(),
			new PD4_E()
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

	
	std::shared_ptr<EtherCatDevice> getDeviceByEtherCatName(const char* etherCatName) {
	   for (EtherCatDevice* device : allEtherCatDevices) {
		   if (strcmp(etherCatName, device->getEtherCatName()) == 0) return std::dynamic_pointer_cast<EtherCatDevice>(device->getNewInstance());
	   }
	   return std::make_shared<EtherCatDevice>();
	}

	const std::vector<NodeGroup>& getEtherCatDevicesByManufacturer() { return etherCatdevicesByManufacturer; }
	const std::vector<NodeGroup>& getEtherCatDevicesByCategory() { return etherCatdevicesByCategory; }

}
