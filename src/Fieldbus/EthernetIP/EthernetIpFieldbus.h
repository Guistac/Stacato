#pragma once

#include <ConnectionManager.h>
#include <thread>

#include "EipServoDrive.h"

class EthernetIPFieldbus {
public:
	static void init(int ip[4], int p);
	static void terminate();

	static std::vector<std::shared_ptr<EipServoDrive>> servoDrives;
	static std::vector<EipDevice> discoveredDevices;

	static void discoverDevices();
	static void addDevice(EipDevice&);
	static void removeDevice(std::shared_ptr<EipServoDrive>);

	static eipScanner::ConnectionManager& getConnectionManager() {
		return connectionManager;
	}

	static int broadcastIP[4];
	static int port;


private:

	static void runtime();
	static eipScanner::ConnectionManager connectionManager;
	static std::thread eipRuntimeThread;
	static bool running;
};
