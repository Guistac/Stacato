#pragma once

#include "Fieldbus/EtherCatDevice.h"

namespace EtherCatDeviceFactory {

	struct EtherCatDeviceGroup {
		char name[128];
		std::vector<EtherCatDevice*> devices;
	};

	void loadDevices();

	std::shared_ptr<EtherCatDevice> getDeviceByName(const char* name);

	const std::vector<EtherCatDeviceGroup>& getDevicesByManufacturer();
	const std::vector<EtherCatDeviceGroup>& getDevicesByCategory();

}