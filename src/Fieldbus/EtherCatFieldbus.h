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
    static void scanNetwork();
    static void configureSlaves();
    static void startCyclicExchange();
    static void stopCyclicExchange();
    static void terminate();
    static void process(bool);

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
    static bool b_networkScanned;

    //process data
    static uint8_t ioMap[4096];
    static int ioMapSize;
    static bool b_ioMapConfigured;
    static int expectedWorkingCounter;

    //runtime
    static std::thread etherCatRuntime;
    static bool b_processRunning;
    static bool b_clockStable;
};

const char* getStateString(uint16_t state);

