#pragma once

#include <ConnectionManager.h>
#include <thread>

#include "EipServoDrive.h"

class EthernetIPFieldbus {
public:
	static void init(const char* brdcstaddr);
	static void terminate();

	static std::vector<std::shared_ptr<EipServoDrive>> servoDrives;
	static std::vector<EipDevice> discoveredDevices;

	static void discoverDevices();
	static void addDevice(EipDevice&);
	static void removeDevice(std::shared_ptr<EipServoDrive>);

	static eipScanner::ConnectionManager& getConnectionManager() {
		return connectionManager;
	}

private:

	static void runtime();
	static void exit();
	static eipScanner::ConnectionManager connectionManager;
	static std::thread eipRuntimeThread;
	static bool running;

	static const char* broadcastAddress;
};
