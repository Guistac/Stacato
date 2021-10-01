#pragma once

#include "Utilities/EtherCatMetrics.h"

class EtherCatSlave;

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

namespace tinyxml2 { class XMLElement; }

namespace EtherCatFieldbus {

    //Get Network Interface Cards
    void updateNetworkInterfaceCardList();
    extern std::vector<NetworkInterfaceCard> networkInterfaceCards;

    //Initialize EtherCAT using one or two Network Interface Cards
    bool init(NetworkInterfaceCard&);
    bool init(NetworkInterfaceCard&, NetworkInterfaceCard&);
    extern NetworkInterfaceCard networkInterfaceCard;
    extern NetworkInterfaceCard redundantNetworkInterfaceCard;
    extern bool b_redundant;

    //Terminate EtherCAT, releasing the network hardware
    void terminate();

    //Discover Slave Devices
    void scanNetwork();
    extern std::vector<std::shared_ptr<EtherCatSlave>> slaves;              //all slaves discovered on the network
    extern std::vector<std::shared_ptr<EtherCatSlave>> slaves_unassigned;   //discovered slaves that are not in the nodegraph

    //Cyclic Echange Timing Settings
    extern double processInterval_milliseconds;
    extern double processDataTimeout_milliseconds;
    extern double clockStableThreshold_milliseconds;
    extern double fieldbusTimeout_milliseconds;

    //Start Cyclic Exchange
    void start();
    extern int i_startupProgress;     //counts up during network configuration to display a progress bar
    extern bool b_startupError;       //high if configuration failed at some point
    extern char startupStatusString[128];   //updated to display the current network configuration step or error

    //Metrics to monitor the Cyclic Exchange
    extern EtherCatMetrics metrics;

    
    bool isNetworkInitialized();                //Is EtherCAT Initializer with a network interface card
    bool isCyclicExchangeStarting();            //Is the Cyclic Exchange in Startup
    bool isCyclicExchangeActive();              //Is the Cyclic Exchange Running
    bool isCyclicExchangeStartSuccessfull();    //Is the Cyclic Exchange Successfully Started

    //Stop Cyclic Exchange
    void stop();

    //Save and load EtherCAT settings
    bool save(tinyxml2::XMLElement* xml);
    bool load(tinyxml2::XMLElement* xml);

    double getReferenceClock_seconds();

};

