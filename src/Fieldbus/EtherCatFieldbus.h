#pragma once

#include <thread>
#include <vector>

#include "ECatMetrics.h"
#include "ECatServoDrive.h"

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

class EtherCatFieldbus {
public:
    static void updateNetworkInterfaceCardList();
    static void init(NetworkInterfaceCard&);
    static void scanNetwork();
    static void configureSlaves();
    static void startCyclicExchange();
    static void stopCyclicExchange();
    static void terminate();
    static void process(bool);

    //process timing
    static double processInterval_milliseconds;
    static double processDataTimeout_milliseconds;

    //metrics
    static ECatMetrics metrics;

    //network hardware
    static std::vector<NetworkInterfaceCard> networkInterfaceCards;
    static NetworkInterfaceCard selectedNetworkInterfaceCard;
    
    //slave devices
    static std::vector<ECatServoDrive> servoDrives;
    static bool b_networkScanned;

    //process data
    static uint8_t ioMap[4096];
    static int ioMapSize;
    static bool b_ioMapConfigured;

    //runtime
    static std::thread etherCatRuntime;
    static bool b_processRunning;
};

const char* getStateString(uint16_t state);

