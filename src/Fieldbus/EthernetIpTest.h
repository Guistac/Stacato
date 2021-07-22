#pragma once

#include <MessageRouter.h>
#include <ConnectionManager.h>
#include <DiscoveryManager.h>
#include <cip/connectionManager/NetworkConnectionParams.h>
#include <IdentityObject.h>
#include <utils/Logger.h>
#include <utils/Buffer.h>

#include <iostream>
#include <bitset>
#include <thread>
#include <chrono>
#include <iomanip>

using eipScanner::SessionInfo;
using eipScanner::MessageRouter;
using eipScanner::ConnectionManager;
using eipScanner::DiscoveryManager;
using eipScanner::IdentityObject;
using eipScanner::cip::connectionManager::ConnectionParameters;
using eipScanner::cip::connectionManager::NetworkConnectionParams;
using namespace eipScanner::cip;
using namespace eipScanner::utils;

void EthernetIpTest();

const char* generalStatusCodeToString(GeneralStatusCodes code);

void printDeviceNetworkConfiguration(std::shared_ptr<SessionInfo> session);

void rebootDevice(std::shared_ptr<SessionInfo> session);

void setupImplicitMessaging(std::shared_ptr<SessionInfo> session);

void printDiscoveredDevices(const char* broadcastAddress, uint16_t port);
