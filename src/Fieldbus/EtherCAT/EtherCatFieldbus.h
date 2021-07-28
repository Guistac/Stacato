#pragma once

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>
#include <vector>

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

struct EtherCatSlave {
    ec_slavet* slave_ptr;
    char name[128];
    char displayName[128];
    int index;
    int manualAddress;
    int address;
    bool b_configured = false;
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

    static bool b_networkScanned;


    static std::vector<NetworkInterfaceCard> networkInterfaceCards;
    static NetworkInterfaceCard selectedNetworkInterfaceCard;
    
    static std::vector<EtherCatSlave> slaves;
    static uint8_t ioMap[4096];
    static int ioMapSize;
    static bool b_ioMapConfigured;

    static std::thread etherCatRuntime;
    static bool b_processRunning;

};

const char* getStateString(uint16_t state);

void EtherCAT_dev(NetworkInterfaceCard nic);

