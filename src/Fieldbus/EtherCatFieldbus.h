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
    static NetworkInterfaceCard selectedNetworkInterfaceCard;
    
    //slave devices
    static std::vector<std::shared_ptr<EtherCatSlave>> slaves;

    //process data
    static uint8_t ioMap[4096];
    static int ioMapSize;
    static int expectedWorkingCounter;

    //runtime
    static std::thread etherCatRuntime;
    static bool b_processStarting;
    static bool b_processRunning;
    static bool b_clockStable;

    static std::thread errorWatchdog;
    static bool b_networkOpen;

private:

    static bool configureSlaves();
    static void startCyclicExchange();
};

const char* getStateString(uint16_t state);

