#pragma once

#include <thread>
#include <vector>

#include "EtherCatMetrics.h"

class EtherCatSlave;

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

class EtherCatFieldbus {
public:
    static void updateNetworkInterfaceCardList();
    static bool init(NetworkInterfaceCard&);
    static bool init(NetworkInterfaceCard&, NetworkInterfaceCard&);
    static void terminate();

    static bool scanNetwork();
    static void start();
    static void stop();

    //process timing
    static double processInterval_milliseconds;
    static double processDataTimeout_milliseconds;
    static double clockStableThreshold_milliseconds;

    //metrics
    static EtherCatMetrics metrics;

    //network hardware
    static std::vector<NetworkInterfaceCard> networkInterfaceCards;
    static NetworkInterfaceCard networkInterfaceCard;
    static NetworkInterfaceCard redundantNetworkInterfaceCard;
    static bool b_redundant;

    //slave devices
    static std::vector<EtherCatSlave*> slaves;
    static int getSlaveCount();

    //process data
    static uint8_t ioMap[4096];
    static int ioMapSize;
    static int expectedWorkingCounter;

    //runtime
    static std::thread errorWatchdog;       //thread to read errors encountered by SOEM
    static std::thread etherCatRuntime;     //cyclic exchange thread (needs a full cpu core to achieve precise timing)

    static bool b_networkOpen;              //high when one or more network interface cards are opened
    static bool b_processStarting;          //high during initial fieldbus setup, before starting cyclic exchange (prevents concurrent restarting)   
    static int i_configurationProgress;     //counts up during network configuration to display a progress bar
    static bool b_configurationError;       //high if configuration failed at some point
    static char configurationStatus[128];   //updated to display the current network configuration step or error
    static bool b_processRunning;           //high while the cyclic exchange is running (also controls its shutdown)
    static bool b_clockStable;              //high when clock drift is under the threshold value
    static bool b_allOperational;           //high when all states reached operational state after clock stabilisation, indicates successful fiedlbus configuration


private:
    static void setup();
    static bool configureSlaves();
    static void startCyclicExchange();
};

const char* etherCatStateToString(uint16_t state);

