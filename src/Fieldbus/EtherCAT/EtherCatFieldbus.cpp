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
    int workingCounter = 0;

    for (ECatServoDrive& drive : servoDrives) {
        if (strcmp(drive.identity.name, "LXM32M EtherCAT") == 0) {
            //asign RxPDO and TxPDO here:
            drive.identity.slave_ptr->PO2SOconfigx = [](ecx_contextt* context, uint16_t slave) -> int {
                /*
                std::cout << "===== Remapping TxPDO Parameters..." << std::endl;
                //remap used TxPDO to two arbitray parameters of the drive (iMax and vMax)
                uint16_t TxPDOmodule = 0x1A03;
                uint8_t zero = 0;
                uint8_t TxPDOparameterCount = 2;
                uint32_t TxPDOparameter1 = 0x30110C10; //parameter 3011, index 0C, size 10 (16 bits)
                uint32_t TxPDOparameter2 = 0x30111020; //parameter 3011, index 10, size 20 (32 bits)
                //disable pdo by setting the sub-index size to zero
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &zero, EC_TIMEOUTSAFE);
                //add first parameter at first index of PDO
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x1, false, 4, &TxPDOparameter1, EC_TIMEOUTSAFE);
                //add second parameter at second index of PDO
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x2, false, 4, &TxPDOparameter2, EC_TIMEOUTSAFE);
                //enable pdo by setting the index equal to the number of parameters in the pdo
                workingCounter += ec_SDOwrite(1, TxPDOmodule, 0x0, false, 1, &TxPDOparameterCount, EC_TIMEOUTSAFE);
                if (workingCounter == TxPDOparameterCount + 2) std::cout << "===== Successfully set custom pdo mapping !" << std::endl;
                else std::cout << "===== Failed to set custom pdo mapping..." << std::endl;
                */

                std::cout << "===== Begin PDO assignement..." << std::endl;
                int workingCounter = 0;
                uint8_t PDOoff = 0x00;
                uint8_t PDOon = 0x01;
                //Sync Manager (SM2, SM3) registers that store the mapping objects (modules) which decribe PDO data
                uint16_t RxPDO = 0x1C12;
                uint16_t TxPDO = 0x1C13;
                //mapping object (module) to be stored in each pdo register
                uint16_t RxPDOmodule = 0x1603;
                uint16_t TxPDOmodule = 0x1A03;
                //turn the pdo off by writing a zero to the 0 index, set the mapping object at subindex 1, enable the pdo by writing a 1 (module count) to the index
                workingCounter += ec_SDOwrite(slave, RxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, RxPDO, 0x1, false, 2, &RxPDOmodule, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, RxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
                //do the same for the TxPDO
                workingCounter += ec_SDOwrite(slave, TxPDO, 0x0, false, 1, &PDOoff, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, TxPDO, 0x1, false, 2, &TxPDOmodule, EC_TIMEOUTSAFE);
                workingCounter += ec_SDOwrite(slave, TxPDO, 0x0, false, 1, &PDOon, EC_TIMEOUTSAFE);
                if (workingCounter == 6) std::cout << "===== PDO assignement successfull !" << std::endl;
                else std::cout << "===== PDO assignement failed..." << std::endl;
                return 0;
            };
        }
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

            int workingCounter;

            //thread timing
            using namespace std::chrono;
            microseconds cycleInterval(50000);
            time_point now = high_resolution_clock::now();
            time_point previousCycleStartTime = now;

            //benchmarking
            int transmissions = 0;
            int transmissionSuccesses = 0;
            int transmissionFailures = 0;
            int maxTransmissions = 10000;
            std::vector<long long> tripLengths_nanoseconds(maxTransmissions, 0);
            std::vector<long long> cycleLengths_nanoseconds(maxTransmissions, 0);
            time_point begin = high_resolution_clock::now();

            while (b_processRunning) {
                //brute force timing precision by using 100% of CPU core
                while (now - previousCycleStartTime < cycleInterval) now = high_resolution_clock::now();

                //measure the time between the previous cycle start and this cycle start
                cycleLengths_nanoseconds[transmissions] = duration_cast<nanoseconds>(now - previousCycleStartTime).count();
                previousCycleStartTime = now;

                //send ethercat frame and wait for return of the data until timeout
                ec_send_processdata();
                workingCounter = ec_receive_processdata(EC_TIMEOUTRET);

                process();

                //measure time between sending and receiving the frame or timeout
                tripLengths_nanoseconds[transmissions] = duration_cast<nanoseconds>(high_resolution_clock::now() - previousCycleStartTime).count();

                //TODO: detect difference between transmission corruption and timeout
                if (workingCounter == 3) transmissionSuccesses++;
                else transmissionFailures++;

                //count transmissions and exit the loop when the target has been reached
                transmissions++;
                if (transmissions == maxTransmissions) break;
            }

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












void EtherCAT_dev(NetworkInterfaceCard nic) {
    //ec_statecheck(slave,state,timeout) checks the status of the slave and returns the state
    //ec_readstate() reads state of all slaves and stores it in ec_slave[]
    //ec_writestate() sets the state of all slaves as it was set in ec_slave[]

    //ec_B--  broadcast addressing 
    //ec_AP-  position addressing  
    //ec_FP-  node addressing      
    //ec_L--  logical addressing   
    //are these commands event needed for regular use ?

    /*
    std::cout << "slave 1: " << slave->name
        << "\n activeports: " << slave->activeports
        << "\n aliasadr " << slave->aliasadr
        << "\n configadr " << slave->configadr
        << "\n configindex " << slave->configindex
        << "\n alstatuscode " << ec_ALstatuscode2string(slave->ALstatuscode)
        << "\n entryport " << slave->entryport
        << "\n itype " << slave->Itype
        << "\n state " << slave->state
        << "\n topology " << slave->topology
        << "\n ibytes " << slave->Ibytes
        << "\n ibits " << slave->Ibits
        << "\n obytes " << slave->Obytes
        << "\n obits " << slave->Obits
        << std::endl;
    */
}

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