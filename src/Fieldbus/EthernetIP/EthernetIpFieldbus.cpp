#include "EthernetIpFieldbus.h"

#include <ConnectionManager.h>
#include <DiscoveryManager.h>
#include <utils/Logger.h>

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

eipScanner::ConnectionManager EthernetIPFieldbus::connectionManager;
std::vector<std::shared_ptr<EipServoDrive>> EthernetIPFieldbus::servoDrives;
std::vector<EipDevice> EthernetIPFieldbus::discoveredDevices;
std::thread EthernetIPFieldbus::eipRuntimeThread;
bool EthernetIPFieldbus::running = false;
const char* EthernetIPFieldbus::broadcastAddress;

void EthernetIPFieldbus::init(const char* brdcstaddr) {
    eipScanner::utils::Logger::setLogLevel(eipScanner::utils::LogLevel::INFO);
    broadcastAddress = brdcstaddr;
    running = true;
    eipRuntimeThread = std::thread([]() {
        while (running) {
            runtime();
        }
        exit();
    });
}

void EthernetIPFieldbus::terminate() {
    running = false;
    eipRuntimeThread.join();
}

void EthernetIPFieldbus::runtime() {
    bool hasOpenConnections = connectionManager.hasOpenConnections();
    if(hasOpenConnections) connectionManager.handleConnections(std::chrono::milliseconds(10));
    if(!hasOpenConnections){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void EthernetIPFieldbus::exit() {
    //forwardclose all connection
}


void EthernetIPFieldbus::discoverDevices() {
    std::cout << "===== Discovering Ethernet/IP devices on the network " << broadcastAddress << std::endl;
    DiscoveryManager discoveryManager(broadcastAddress, 0xAF12, std::chrono::milliseconds(100));
    std::vector<eipScanner::IdentityItem> devices = discoveryManager.discover();
    discoveredDevices.clear();
    std::cout << "===== Found " << devices.size() << ((devices.size() == 1) ? " device :" : " devices :") << std::endl;
    for (eipScanner::IdentityItem& device : devices) {
        eipScanner::sockets::EndPoint& endpoint = device.socketAddress;
        
        bool deviceAlreadyAdded = false;
        for (auto drive : servoDrives) {
            if (strcmp(endpoint.getHost().c_str(), drive->identity.host) == 0) {
                deviceAlreadyAdded = true;
            }
        }
        if (deviceAlreadyAdded) continue;

        eipScanner::IdentityObject& identityObject = device.identityObject;
        EipDevice device;
        device.productCode = identityObject.getProductCode();
        device.revisionMajor = identityObject.getRevision().getMajorRevision();
        device.revisionMinor = identityObject.getRevision().getMinorRevision();
        device.serialNumber = identityObject.getSerialNumber();
        device.vendorId = identityObject.getVendorId();
        device.status = identityObject.getStatus();
        device.port = endpoint.getPort();
        strcpy(device.productName, identityObject.getProductName().c_str());
        strcpy(device.host, endpoint.getHost().c_str());
        sprintf(device.displayName, "%s (%s:%i)", device.productName, device.host, device.port);

        discoveredDevices.push_back(std::move(device));
    }
    for (EipDevice& device : EthernetIPFieldbus::discoveredDevices) {
        std::cout << "Product Name: " << device.productName
            << "\n = productCode: " << device.productCode
            << "\n = Revision: " << (uint32_t)device.revisionMajor
            << "." << (uint32_t)device.revisionMinor
            << "\n = SerialNumber: " << device.serialNumber
            << "\n = VendorID: " << device.vendorId
            << "\n = Status: " << device.status
            << "\n = IP: " << device.host
            << "\n = port: " << device.port
            << "\n = displayName: " << device.displayName
            << std::endl;
    }
}

void EthernetIPFieldbus::addDevice(EipDevice& device) {
    /*
    DiscoveryManager discoveryManager(device.host, device.port, std::chrono::milliseconds(100));
    std::vector<eipScanner::IdentityItem> identities = discoveryManager.discover();
    bool deviceOnline = false;
    for (eipScanner::IdentityItem& identity : identities) {
        const char* discoveredAddress = identity.socketAddress.getHost().c_str();
        if (strcmp(discoveredAddress, device.host) == 0) {
            deviceOnline = true;
            break;
        }
    }
    if (!deviceOnline) {
        std::cout << "Could not connect: Device Offline" << std::endl;
        return false;
    }
    else {
        std::cout << "Connecting to device...";
    }
     //servoDrive->startSession();
    //servoDrive->startImplicitMessaing(connectionManager);
    //std::cout << " Connected! " << std::endl;
    //return true;
    */
    std::shared_ptr<EipServoDrive> servoDrive = std::make_shared<EipServoDrive>();
    servoDrive->identity = std::move(device);
    servoDrives.push_back(servoDrive);

    for (int i = discoveredDevices.size() - 1; i >= 0; i--) {
        if (strcmp(discoveredDevices[i].host, device.host) == 0) {
            discoveredDevices.erase(discoveredDevices.begin() + i);
        }
    }
}

void EthernetIPFieldbus::removeDevice(std::shared_ptr<EipServoDrive> device) {
    for (int i = servoDrives.size() - 1; i >= 0; i--) {
        if (device == servoDrives[i]) {
            device->disconnect();
            servoDrives.erase(servoDrives.begin() + i);
            break;
        }
    }
}