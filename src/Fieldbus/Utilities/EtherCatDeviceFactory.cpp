#include <pch.h>

#include "EtherCatDeviceFactory.h"

#include "Fieldbus/Devices/SchneiderElectric/Lexium32.h"
#include "Fieldbus/Devices/Artefact/EasyCat/EasyCAT.h"
#include "Fieldbus/Devices/Artefact/LedsAndButtons/LedsAndButtons.h"
#include "Fieldbus/Devices/Yaskawa/VIPA-053-1EC01.h"

//TODO:
/*
ideally we want to have one method that adds each slave to a global list
then we can sort the devices by name, manufacturer and type
and we don't need to manually format multiple lists
each device can have a one line entry to be added to all lists

*/

namespace EtherCatDeviceFactory {
	
	std::vector<EtherCatDevice*> allDevices;
	std::vector<EtherCatDeviceGroup> devicesByManufacturer;
	std::vector<EtherCatDeviceGroup> devicesByCategory;

	void loadDevices() {
		allDevices = {
			new Lexium32(),
			new VIPA_053_1EC01(),
			new EasyCAT(),
			new LedsAndButtons()
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

	std::shared_ptr<EtherCatDevice> getDeviceByName(const char* name) {
		for (EtherCatDevice* device : allDevices) {
			if (strcmp(name, device->getNodeName()) == 0) return device->getNewDeviceInstance();
		}
		return std::make_shared<EtherCatDevice>();
	}

	const std::vector<EtherCatDeviceGroup>& getDevicesByManufacturer() { return devicesByManufacturer; }
	const std::vector<EtherCatDeviceGroup>& getDevicesByCategory() { return devicesByCategory; }

}