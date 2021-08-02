#include "EtherCatFieldbus.h"

#include <ethercat.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>

std::vector<NetworkInterfaceCard>   EtherCatFieldbus::networkInterfaceCards;
NetworkInterfaceCard                EtherCatFieldbus::selectedNetworkInterfaceCard;
bool                                EtherCatFieldbus::b_networkScanned = false;
std::vector<ECatServoDrive>         EtherCatFieldbus::servoDrives;
uint8_t                             EtherCatFieldbus::ioMap[4096];
int                                 EtherCatFieldbus::ioMapSize = 0;
std::thread                         EtherCatFieldbus::etherCatRuntime;
bool                                EtherCatFieldbus::b_processRunning = false;
bool                                EtherCatFieldbus::b_ioMapConfigured = false;
ScrollingBuffer                     EtherCatFieldbus::timingHistory(1000);
ScrollingBuffer                     EtherCatFieldbus::workingCounterHistory(1000);
int                                 EtherCatFieldbus::processInterval_microseconds = 10000;
int                                 EtherCatFieldbus::processDataTimeout_microseconds = 7000;

void EtherCatFieldbus::updateNetworkInterfaceCardList() {
    std::cout << "===== Refreshing Network Interface Card List" << std::endl;
    networkInterfaceCards.clear();
    ec_adaptert* nics = ec_find_adapters();
    if (nics != nullptr) {
        while (nics != nullptr) {
            NetworkInterfaceCard nic;
            strcpy(nic.name, nics->name);
            strcpy(nic.description, nics->desc);
            networkInterfaceCards.push_back(std::move(nic));
            nics = nics->next;
        }
    }
    std::cout << "===== Found " << networkInterfaceCards.size() << " Network Interface Card" << (networkInterfaceCards.size() == 1 ? "" : "s") << std::endl;
    for (NetworkInterfaceCard& nic : networkInterfaceCards) {
        std::cout << "    = " << nic.description << " (ID: " << nic.name << ")" << std::endl;
    }

    //ec_dcs
}

void EtherCatFieldbus::init(NetworkInterfaceCard& nic) {
    selectedNetworkInterfaceCard = std::move(nic);
    std::cout << "===== Initializing EtherCAT Fieldbus on Network Interface Card '" << selectedNetworkInterfaceCard.description << "'" << std::endl;
    int nicInitResult = ec_init(selectedNetworkInterfaceCard.name);
    if (nicInitResult > 0) std::cout << "===== Initialized network interface card !" << std::endl;
    else std::cout << "===== Failed to initialize network interface card ..." << std::endl;
}

void EtherCatFieldbus::scanNetwork() {
    servoDrives.clear();
    b_ioMapConfigured = false;
    //setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
    int workingCounter = ec_config_init(FALSE); //what is usetable??
    for (int i = 1; i <= ec_slavecount; i++) {
        ec_slavet& slv = ec_slave[i];
        EtherCatSlave slave;
        slave.slave_ptr = &slv;
        strcpy(slave.name, slv.name);
        slave.index = i;
        slave.manualAddress = slv.aliasadr;
        slave.address = slv.configadr;
        sprintf(slave.displayName, "%s (Node %i #%i)", slave.name, slave.index, slave.manualAddress);
        servoDrives.push_back(ECatServoDrive());
        servoDrives.back().identity = slave;

    }
    if (workingCounter > 0) {
        b_networkScanned = true;
        std::cout << "===== Found and Configured " << ec_slavecount << " EtherCAT Slave" << ((ec_slavecount == 1) ? ": " : "s: ") << std::endl;
        for (ECatServoDrive& drive : servoDrives)
            std::cout << "    = Slave "
            << drive.identity.index << " : '"
            << drive.identity.name << "' Address: "
            << drive.identity.address << " Manual Address: "
            << drive.identity.manualAddress << std::endl;
    }
    else {
        b_networkScanned = false;
        std::cout << "===== No EtherCAT Slaves found..." << std::endl;
    }
}

void EtherCatFieldbus::configureSlaves() {
    for (int i = 1; i <= ec_slavecount; i++) {
        //set hook callback for configuring the PDOs of each slave
        ec_slave[i].PO2SOconfigx = [](ecx_contextt* context, uint16_t slave) -> int {
            for (ECatServoDrive& servoDrive : servoDrives) {
                if (servoDrive.identity.index == slave) {
                    servoDrive.configurePDOs();
                    break;
                }
            }
            return 0;
        };   
    }
    
    //build ioMap for PDO data, configure FMMU and SyncManager, request SAFE-OP state for all slaves
    std::cout << "===== Begin Building I/O Map..." << std::endl;
    ioMapSize = ec_config_map(ioMap);
    std::cout << "===== Finished Building I/O Map (Size : " << ioMapSize << " bytes)" << std::endl;
    if (ioMapSize > 0) b_ioMapConfigured = true;

    for (ECatServoDrive& drive : servoDrives) {
        drive.identity.b_configured = true;
    }

    for (ECatServoDrive& drive : servoDrives) {
        std::cout << "   [" << drive.identity.index << "] '" << drive.identity.name << "' " << drive.identity.slave_ptr->Ibytes + drive.identity.slave_ptr->Obytes << " bytes (" << drive.identity.slave_ptr->Ibits + drive.identity.slave_ptr->Obits << " bits)" << std::endl;
        std::cout << "          Inputs: " << drive.identity.slave_ptr->Ibytes << " bytes (" << drive.identity.slave_ptr->Ibits << " bits)" << std::endl;
        std::cout << "          Outputs: " << drive.identity.slave_ptr->Obytes << " bytes (" << drive.identity.slave_ptr->Obits << " bits)" << std::endl;
    }

    std::cout << "===== Configuring Distributed Clocks" << std::endl;
    bool distributedClockConfigurationResult = ec_configdc();
    if (distributedClockConfigurationResult) std::cout << "===== Finished Configuring Distributed Clocks" << std::endl;
    else std::cout << "===== Could not configure distributed clocks ..." << std::endl;
}

void EtherCatFieldbus::terminate() {
    stopCyclicExchange();
    std::cout << "===== Closing EtherCAT Network Interface Card" << std::endl;
    ec_close();
    std::cout << "===== Ending EtherCAT test program" << std::endl;
}

void EtherCatFieldbus::startCyclicExchange() {
    if (!b_processRunning) {
        if (etherCatRuntime.joinable()) etherCatRuntime.join();
        b_processRunning = true;
        etherCatRuntime = std::thread([]() {            

            std::cout << "===== Setting All Slaves to Operational state..." << std::endl;
            // Act on slave 0 (a virtual slave used for broadcasting)
            ec_slavet* broadcastSlave = &ec_slave[0];
            broadcastSlave->state = EC_STATE_OPERATIONAL;
            ec_writestate(0);

            //wait for all slaves to reach OP state
            uint16_t slaveState = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            if (slaveState == EC_STATE_OPERATIONAL) std::cout << "===== All slaves are operational !" << std::endl;

            std::cout << "===== Begin Cyclic Process Data Echange !" << std::endl;


            //thread timing
            using namespace std::chrono;
            microseconds cycleInterval(processInterval_microseconds);
            time_point now = high_resolution_clock::now();
            time_point previousCycleStartTime = now;

            int workingCounter;
            
            /*
            //benchmarking
            int transmissions = 0;
            int transmissionSuccesses = 0;
            int transmissionFailures = 0;
            int maxTransmissions = 1000;
            std::vector<long long> tripLengths_nanoseconds(maxTransmissions, 0);
            std::vector<long long> cycleLengths_nanoseconds(maxTransmissions, 0);
            time_point begin = high_resolution_clock::now();
            */

            int counter = 0;
            time_point previousSendTime = high_resolution_clock::now();
            time_point thisSendTime = high_resolution_clock::now();

            while (b_processRunning) {
                //brute force timing precision by using 100% of CPU core
                while (now - previousCycleStartTime < cycleInterval) now = high_resolution_clock::now();

                //measure the time between the previous cycle start and this cycle start
                //cycleLengths_nanoseconds[transmissions] = duration_cast<nanoseconds>(now - previousCycleStartTime).count();
                previousCycleStartTime = now;

                //send ethercat frame
                ec_send_processdata();
                thisSendTime = high_resolution_clock::now();
                double intervalMicros = (double)duration_cast<nanoseconds>(thisSendTime - previousSendTime).count() / 1000.0L;
                timingHistory.addPoint(TimeInterval{ intervalMicros,(double)counter });
                previousSendTime = thisSendTime;
                counter++;
                //wait for return of the frame until timeout
                workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
                workingCounterHistory.addPoint(TimeInterval{ (double)workingCounter, (double)counter });

                //do axis processing to get next set of output data
                process();

                /*
                if (workingCounter == ec_slavecount * 3) {}
                else if (workingCounter == EC_NOFRAME) {}
                else if (workingCounter == EC_TIMEOUT) {}
                */
            }

            /*
            //compute average cycle frequency
            long long duration_microseconds = duration_cast<microseconds>(high_resolution_clock::now() - begin).count();
            double durationSeconds = (double)duration_microseconds / 1000000.0L;
            double cycleFrequency = (double)maxTransmissions / durationSeconds;

            float transmissionSuccessRate = 100.0f * (float)transmissionSuccesses / (float)maxTransmissions;
            std::cout << std::endl;
            std::cout << "===== Ended Cycle Process Data Exchange After " << maxTransmissions << " cycles !" << std::endl;
            std::cout << "===== Sucessfull transmissions: " << transmissionSuccesses << "   Failures: " << transmissionFailures << std::endl;
            std::cout << "===== Success Rate: " << transmissionSuccessRate << "%" << std::endl;
            std::cout << "===== Cycle Frequency: " << cycleFrequency << "Hz" << std::endl;

            long long longestTripLength_nanoseconds = 0;
            long long shortestTripLength_nanoseconds = LLONG_MAX;
            long long sumTripLength_nanoseconds = 0;

            long long longestCycleLength_nanoseconds = 0;
            long long shortesCycleLength_nanoseconds = LLONG_MAX;
            long long sumCycleLength_nanoseconds = 0;

            for (int i = 0; i < maxTransmissions; i++) {
                if (tripLengths_nanoseconds[i] > longestTripLength_nanoseconds) longestTripLength_nanoseconds = tripLengths_nanoseconds[i];
                if (tripLengths_nanoseconds[i] < shortestTripLength_nanoseconds) shortestTripLength_nanoseconds = tripLengths_nanoseconds[i];
                sumTripLength_nanoseconds += tripLengths_nanoseconds[i];
                if (cycleLengths_nanoseconds[i] > longestCycleLength_nanoseconds) longestCycleLength_nanoseconds = cycleLengths_nanoseconds[i];
                if (cycleLengths_nanoseconds[i] < shortesCycleLength_nanoseconds) shortesCycleLength_nanoseconds = cycleLengths_nanoseconds[i];
                sumCycleLength_nanoseconds += cycleLengths_nanoseconds[i];
            }

            float averageTripLength_milliseconds = ((float)sumTripLength_nanoseconds / (float)maxTransmissions) / 1000000.0f;
            float maxTripLength_milliseconds = (float)longestTripLength_nanoseconds / 1000000.0;
            float minTripLength_milliseconds = (float)shortestTripLength_nanoseconds / 1000000.0;
            
            float averageCycleLength_milliseconds = ((float)sumCycleLength_nanoseconds / (float)maxTransmissions) / 1000000.0f;
            float maxCycleLength_milliseconds = (float)longestCycleLength_nanoseconds / 1000000.0;
            float minCycleLength_milliseconds = (float)shortesCycleLength_nanoseconds / 1000000.0;

            std::cout << "===== Frame Trip Length   longest: " << maxTripLength_milliseconds << "ms   shortest: " << minTripLength_milliseconds << "ms   average: " << averageTripLength_milliseconds << "ms" << std::endl;
            std::cout << "===== Cycle Length        longest: " << maxCycleLength_milliseconds << "ms   shortest: " << minCycleLength_milliseconds << "ms   average: " << averageCycleLength_milliseconds << "ms " << std::endl;
            */

            b_processRunning = false;
        });
    }
}

void EtherCatFieldbus::stopCyclicExchange() {
    std::cout << "===== Stopping Cyclic Exchange" << std::endl;
    b_processRunning = false;
    if (etherCatRuntime.joinable()) etherCatRuntime.join();
    std::cout << "===== Cyclic Exchange Stopped !" << std::endl;
}


void EtherCatFieldbus::process() {
    for (ECatServoDrive& drive : servoDrives) drive.process();
}













    //ec_B--  broadcast addressing 
    //ec_AP-  position addressing  
    //ec_FP-  node addressing      
    //ec_L--  logical addressing   
    //are these commands even needed for regular use ?

const char* getStateString(uint16_t state) {
    switch (state) {
    case EC_STATE_NONE: return "NO STATE";
    case EC_STATE_INIT: return "INIT";
    case EC_STATE_PRE_OP: return "PRE-OPERATIONAL";
    case EC_STATE_BOOT: return "BOOT";
    case EC_STATE_SAFE_OP: return "SAFE-OPERATIONAL";
    case EC_STATE_OPERATIONAL: return "OPERATIONAL";
    case EC_STATE_ACK: return "STATE-ACK / ERROR";
    default: return "UNKNOWN STATE";
    }
}