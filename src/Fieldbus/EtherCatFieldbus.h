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

    //Get Network Interface Cards
    void updateNetworkInterfaceCardList();
    extern std::vector<std::shared_ptr<NetworkInterfaceCard>> networkInterfaceCards;

	bool hasNetworkPermissions();

    //Initialize EtherCAT using one or two Network Interface Cards
    bool init();
    bool init(std::shared_ptr<NetworkInterfaceCard>);
    bool init(std::shared_ptr<NetworkInterfaceCard>, std::shared_ptr<NetworkInterfaceCard>);
	bool autoInit();
    extern std::shared_ptr<NetworkInterfaceCard> primaryNetworkInterfaceCard;
    extern std::shared_ptr<NetworkInterfaceCard> redundantNetworkInterfaceCard;

    //Terminate EtherCAT, releasing the network hardware
    void terminate();

    //Discover Slave Devices
    void scanNetwork();
    extern std::vector<std::shared_ptr<EtherCatDevice>> slaves;              //all slaves discovered on the network
    extern std::vector<std::shared_ptr<EtherCatDevice>> slaves_unassigned;   //discovered slaves that are not in the nodegraph
    void removeFromUnassignedSlaves(std::shared_ptr<EtherCatDevice> removedDevice);

    //Cyclic Echange Timing Settings
    extern double processInterval_milliseconds;
    extern double processDataTimeout_milliseconds;
    extern double clockStableThreshold_milliseconds;
    extern double fieldbusTimeout_milliseconds;

    //Start Cyclic Exchange
    void start();
	extern ProgressIndicator startupProgress;

    //Metrics to monitor the Cyclic Exchange
    extern EtherCatMetrics metrics;

    bool isNetworkInitialized();                //Is EtherCAT Initialized with a network interface card
	bool isNetworkRedundant();					//Is EtherCAT Open with two network interface cards
    bool isCyclicExchangeStarting();            //Is the Cyclic Exchange in Startup
    bool isCyclicExchangeActive();              //Is the Cyclic Exchange Running
    bool isCyclicExchangeStartSuccessfull();    //Is the Cyclic Exchange Successfully Started

	bool hasNetworkInterface();
	bool isStarting();
	bool isRunning();

    //Stop Cyclic Exchange
    void stop();

    //Save and load EtherCAT settings
    bool save(tinyxml2::XMLElement* xml);
    bool load(tinyxml2::XMLElement* xml);

    double getCycleProgramTime_seconds();
    long long int getCycleProgramTime_nanoseconds();
	double getCycleTimeDelta_seconds();
	long long int getCycleTimeDelta_nanoseconds();
};

