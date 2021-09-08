#pragma once

#include "Fieldbus/EtherCatSlave.h"

namespace EtherCatDeviceFactory {

	struct EtherCatDeviceGroup {
		char name[128];
		std::vector<EtherCatSlave*> devices;
	};

	void loadDevices();

	std::shared_ptr<EtherCatSlave> getDeviceByName(const char* name);

	const std::vector<EtherCatDeviceGroup>& getDevicesByManufacturer();
	const std::vector<EtherCatDeviceGroup>& getDevicesByCategory();

}