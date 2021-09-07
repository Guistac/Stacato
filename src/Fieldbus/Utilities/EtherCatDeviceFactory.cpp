#include <pch.h>

#include "EtherCatDeviceFactory.h"

#include "Fieldbus/Devices/SchneiderElectric/Lexium32.h"
#include "Fieldbus/Devices/Artefact/EasyCat/EasyCAT.h"
#include "Fieldbus/Devices/Artefact/LedsAndButtons/LedsAndButtons.h"

//TODO:
/*
ideally we want to have one method that adds each slave to a global list
then we can sort the devices by name, manufacturer and type
and we don't need to manually format multiple lists
each device can have a one line entry to be added to all lists

*/

namespace EtherCatDeviceFactory {
	
	std::shared_ptr<EtherCatSlave> getDeviceByName(const char* name) {
		RETURN_SLAVE_IF_TYPE_MATCHING(name, Lexium32);
		RETURN_SLAVE_IF_TYPE_MATCHING(name, EasyCAT);
		RETURN_SLAVE_IF_TYPE_MATCHING(name, LedsAndButtons);
		//RETURN_SLAVE_IF_MATCHING(name, OtherDeviceClassName)
		//RETURN_SLAVE_IF_MATCHING(name, DifferentDeviceClassName)
		//...
		//if the slave type is not recognized, return a basic slave instance
		return std::make_shared<EtherCatSlave>();
	}

	std::vector<EtherCatDeviceManufacturer> devices = {
		EtherCatDeviceManufacturer({"Schneider Electric",{
			new Lexium32()
		}}),
		EtherCatDeviceManufacturer({"Leo Becker",{
			new EasyCAT(),
			new LedsAndButtons()
		}})
	};

	std::vector<EtherCatDeviceManufacturer>& getDeviceTypes() {
		return devices;
	}

}