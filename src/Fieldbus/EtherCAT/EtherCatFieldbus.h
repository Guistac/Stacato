#pragma once

#include <thread>
#include <vector>

#include "Gui/ScrollingBuffer.h"
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

    static double processInterval_milliseconds;
    static double processDataTimeout_milliseconds;

    static ScrollingBuffer dcTimeError;
    static ScrollingBuffer averageDcTimeError;
    static ScrollingBuffer workingCounterHistory;
    static size_t scrollingBufferSize;

    static std::vector<NetworkInterfaceCard> networkInterfaceCards;
    static NetworkInterfaceCard selectedNetworkInterfaceCard;
    
    static std::vector<ECatServoDrive> servoDrives;
    static bool b_networkScanned;

    static uint8_t ioMap[4096];
    static int ioMapSize;
    static bool b_ioMapConfigured;

    static std::thread etherCatRuntime;
    static bool b_processRunning;
};

const char* getStateString(uint16_t state);

