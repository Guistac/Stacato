#pragma once

#include "Utilities/EtherCatMetrics.h"

class EtherCatSlave;

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

namespace tinyxml2 { class XMLElement; }

namespace EtherCatFieldbus {

    void updateNetworkInterfaceCardList();
    bool init(NetworkInterfaceCard&);
    bool init(NetworkInterfaceCard&, NetworkInterfaceCard&);
    void terminate();

    bool scanNetwork();
    void start();
    void stop();

    bool save(tinyxml2::XMLElement* xml);
    bool load(tinyxml2::XMLElement* xml);

    bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID);

    double getCurrentCycleDeltaT_seconds();

    //process timing
    extern double processInterval_milliseconds;
    extern double processDataTimeout_milliseconds;
    extern double clockStableThreshold_milliseconds;
    extern int slaveStateCheckCycleCount;

    //metrics
    extern EtherCatMetrics metrics;

    //network hardware
    extern std::vector<NetworkInterfaceCard> networkInterfaceCards;
    extern NetworkInterfaceCard networkInterfaceCard;
    extern NetworkInterfaceCard redundantNetworkInterfaceCard;
    extern bool b_redundant;

    //slave devices
    extern std::vector<std::shared_ptr<EtherCatSlave>> slaves;
    extern std::vector<std::shared_ptr<EtherCatSlave>> slaves_unassigned;

    //process data
    extern int expectedWorkingCounter;

    extern bool b_networkOpen;              //high when one or more network interface cards are opened
    extern bool b_processStarting;          //high during initial fieldbus setup, before starting cyclic exchange (prevents concurrent restarting)   
    extern int i_configurationProgress;     //counts up during network configuration to display a progress bar
    extern bool b_configurationError;       //high if configuration failed at some point
    extern char configurationStatus[128];   //updated to display the current network configuration step or error
    extern bool b_processRunning;           //high while the cyclic exchange is running (also controls its shutdown)
    extern bool b_clockStable;              //high when clock drift is under the threshold value
    extern bool b_allOperational;           //high when all states reached operational state after clock stabilisation, indicates successful fiedlbus configuration

};

