#pragma once

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>

struct NetworkInterfaceCard {
    char* description;
    char* name;
};

void EtherCatTest();

const char* getStateString(uint16_t state);

void EtherCAT_dev(NetworkInterfaceCard nic);

