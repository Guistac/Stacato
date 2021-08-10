#include "EtherCatDeviceIdentifier.h"

std::shared_ptr<EtherCatSlave> getSlaveByName(const char* name) {
	RETURN_SLAVE_IF_TYPE_MATCHING(name, Lexium32);
	//RETURN_SLAVE_IF_MATCHING(name, OtherDeviceClassName)
	//RETURN_SLAVE_IF_MATCHING(name, DifferentDeviceClassName)
	//...
	//if the slave type is not recognized, return a basic slave instance
	return std::make_shared<EtherCatSlave>();
}