#pragma once

#include "Utilities/EtherCatMetrics.h"

class EtherCatDevice;

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

namespace tinyxml2 { class XMLElement; }

struct ProgressIndicator;

namespace EtherCatFieldbus {

	bool hasNetworkPermissions();

    //Get Network Interface Cards
    void updateNetworkInterfaceCardList();
	std::vector<std::shared_ptr<NetworkInterfaceCard>>& getNetworksInterfaceCards();
	std::shared_ptr<NetworkInterfaceCard> getActiveNetworkInterfaceCard();
	std::shared_ptr<NetworkInterfaceCard> getActiveRedundantNetworkInterfaceCard();

    //initialize and terminate
    bool init();
    bool init(std::shared_ptr<NetworkInterfaceCard>);
    bool init(std::shared_ptr<NetworkInterfaceCard>, std::shared_ptr<NetworkInterfaceCard>);
	void terminate();

    //scan network for devices
    void scanNetwork();
	bool hasDetectedDevices();
	std::vector<std::shared_ptr<EtherCatDevice>>& getDevices();             //all slaves discovered on the network
	std::vector<std::shared_ptr<EtherCatDevice>>& getUnassignedDevices();	//discovered slaves that are not in the environnement nodegraph
	void removeUnassignedDevice(std::shared_ptr<EtherCatDevice> removedDevice);

	//scan all network interfaces for devices
	void autoInit();
	bool isAutoInitRunning();

    //Cyclic Exchange Control
    void start();
	void stop();
	
	//Network State
	bool hasNetworkInterface();
	bool hasRedundantInterface();
	bool hasDetectedDevices();
	bool isStarting();
	bool isRunning();
	bool canScan();
	bool canStart();
	bool canStop();

	extern ProgressIndicator startupProgress;

	EtherCatMetrics& getMetrics();

	//cyclic exchange timing
	double getCycleProgramTime_seconds();
	long long getCycleProgramTime_nanoseconds();
	double getCycleTimeDelta_seconds();
	long long getCycleTimeDelta_nanoseconds();

	//Cyclic Echange Timing Settings
	extern double processInterval_milliseconds;
	extern double processDataTimeout_milliseconds;
	extern double clockStableThreshold_milliseconds;
	extern double fieldbusTimeout_milliseconds;

    //Save and load EtherCAT settings
    bool save(tinyxml2::XMLElement* xml);
    bool load(tinyxml2::XMLElement* xml);
};

