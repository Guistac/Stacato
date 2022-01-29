#include <pch.h>

#include "EtherCatDeviceFactory.h"

#include "Fieldbus/Devices/SchneiderElectric/Lexium32.h"
#include "Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.h"
#include "Fieldbus/Devices/Nanotec/PD4_E.h"


namespace EtherCatDeviceFactory {
	
	std::vector<EtherCatDevice*> allDevices;
	std::vector<EtherCatDeviceGroup> devicesByManufacturer;
	std::vector<EtherCatDeviceGroup> devicesByCategory;

	void loadDevices() {
		allDevices = {
			new Lexium32(),
			new VipaBusCoupler_053_1EC01(),
			new PD4_E()
		};

		//sort devices by manufacturer
		for (EtherCatDevice* device : allDevices) {
			const char * manufacturer = device->getManufacturerName();
			bool manufacturerExists = false;
			for (EtherCatDeviceGroup& group : devicesByManufacturer) {
				if (strcmp(manufacturer, group.name) == 0) {
					manufacturerExists = true;
					group.devices.push_back(device);
					break;
				}
			}
			if (!manufacturerExists) {
				devicesByManufacturer.push_back(EtherCatDeviceGroup());
				strcpy(devicesByManufacturer.back().name, device->getManufacturerName());
				devicesByManufacturer.back().devices.push_back(device);
			}
		}

		//sort device by manufacturer
		for (EtherCatDevice* device : allDevices) {
			const char* deviceCategory = device->getNodeCategory();
			bool categoryExists = false;
			for (EtherCatDeviceGroup& group : devicesByCategory) {
				if (strcmp(deviceCategory, group.name) == 0) {
					categoryExists = true;
					group.devices.push_back(device);
					break;
				}
			}
			if (!categoryExists) {
				devicesByCategory.push_back(EtherCatDeviceGroup());
				strcpy(devicesByCategory.back().name, device->getNodeCategory());
				devicesByCategory.back().devices.push_back(device);
			}
		}
	}

	std::shared_ptr<EtherCatDevice> getDeviceByEtherCatName(const char* etherCatName) {
		for (EtherCatDevice* device : allDevices) {
			if (strcmp(etherCatName, device->getEtherCatName()) == 0) return std::dynamic_pointer_cast<EtherCatDevice>(device->getNewInstance());
		}
		return std::make_shared<EtherCatDevice>();
	}

	std::shared_ptr<EtherCatDevice> getDeviceBySaveName(const char* saveName) {
		for (EtherCatDevice* device : allDevices) {
			if (strcmp(saveName, device->getSaveName()) == 0) return std::dynamic_pointer_cast<EtherCatDevice>(device->getNewInstance());
		}
		return std::make_shared<EtherCatDevice>();
	}

	const std::vector<EtherCatDeviceGroup>& getDevicesByManufacturer() { return devicesByManufacturer; }
	const std::vector<EtherCatDeviceGroup>& getDevicesByCategory() { return devicesByCategory; }

}
