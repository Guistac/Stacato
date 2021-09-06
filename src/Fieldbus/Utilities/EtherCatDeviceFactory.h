#pragma once

#include "Fieldbus/EtherCatSlave.h"

namespace EtherCatDeviceFactory {

	struct EtherCatDeviceManufacturer {
		char name[128];
		std::vector<EtherCatSlave*> devices;
	};

	std::shared_ptr<EtherCatSlave> getDeviceByName(const char* name);

	std::vector<EtherCatDeviceManufacturer>& getDeviceTypes();

}