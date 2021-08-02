#pragma once

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>
#include <vector>

#include "ECatServoDrive.h"

struct TimeInterval {
    double microseconds = 0.0;
    double frameNumber = 0;
};

struct ScrollingBuffer {
public:

    ScrollingBuffer() {}
    ScrollingBuffer(size_t s) {
        setMaxSize(s);
    }

    void setMaxSize(size_t s) {
        data.reserve(s);
        s_maxSize = s;
    }

    void addPoint(TimeInterval p) {
        if (data.size() < s_maxSize) {
            data.push_back(p);
        }
        else {
            data[s_offset] = p;
            s_offset = (s_offset + 1) % s_maxSize;
        }
    }

    void clear() {
        data.clear();
        s_offset = 0;
    }

    TimeInterval& front() {
        if (data.empty()) return TimeInterval();
        return data.front();
    }

    TimeInterval& back() {
        if (data.empty()) return TimeInterval();
        if (s_offset == 0) return data.back();
        return data[s_offset - 1];
    }

    size_t size() { return data.size(); }

    size_t maxSize() { return s_maxSize; }

    bool empty() { return data.empty(); }

    size_t offset() { return s_offset; }

    size_t stride() { return sizeof(TimeInterval); }

private:

    std::vector<TimeInterval> data;
    size_t s_maxSize;
    size_t s_offset = 0;
};

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
    static void process();

    static int processInterval_microseconds;
    static int processDataTimeout_microseconds;

    static ScrollingBuffer timingHistory;
    static ScrollingBuffer workingCounterHistory;

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

void EtherCAT_dev(NetworkInterfaceCard nic);

