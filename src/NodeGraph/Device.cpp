#include <pch.h>

#include "Device.h"

std::vector<DeviceType> deviceTypes = {
	{Device::Type::ETHERCAT_DEVICE, "EtherCatDevice", "EtherCAT Device"},
	{Device::Type::NETWORK_DEVICE, "NetworkDevice", "Network Device"},
	{Device::Type::USB_DEVICE, "USBDevice", "USB Device"}
};

DeviceType* getDeviceType(const char* saveName) {
	for (DeviceType& deviceType : deviceTypes) {
		if (strcmp(saveName, deviceType.saveName) == 0) return &deviceType;
	}
	return nullptr;
}

DeviceType* getDeviceType(Device::Type t) {
	for (DeviceType& deviceType : deviceTypes) {
		if (t == deviceType.type) return &deviceType;
	}
	return nullptr;
}