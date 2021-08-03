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
ScrollingBuffer                     EtherCatFieldbus::timingHistory;
ScrollingBuffer                     EtherCatFieldbus::workingCounterHistory;
ScrollingBuffer                     EtherCatFieldbus::clockDrift;
ScrollingBuffer                     EtherCatFieldbus::averageClockDrift;
int                                 EtherCatFieldbus::processInterval_microseconds = 5000;
int                                 EtherCatFieldbus::processDataTimeout_microseconds = 3500;


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
    for (NetworkInterfaceCard& nic : networkInterfaceCards) std::cout << "    = " << nic.description << " (ID: " << nic.name << ")" << std::endl;
}

void EtherCatFieldbus::init(NetworkInterfaceCard& nic) {
    selectedNetworkInterfaceCard = std::move(nic);
    std::cout << "===== Initializing EtherCAT Fieldbus on Network Interface Card '" << selectedNetworkInterfaceCard.description << "'" << std::endl;
    int nicInitResult = ec_init(selectedNetworkInterfaceCard.name);
    if (nicInitResult > 0) std::cout << "===== Initialized network interface card !" << std::endl;
    else std::cout << "===== Failed to initialize network interface card ..." << std::endl;

    timingHistory.setMaxSize(10000);
    workingCounterHistory.setMaxSize(10000);
    clockDrift.setMaxSize(10000);
    averageClockDrift.setMaxSize(10000);
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
                    servoDrive.preOperationalToSafeOperationalConfiguration();
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
            if (slaveState == EC_STATE_OPERATIONAL) {

                std::cout << "===== All slaves are operational, Starting Cyclic Process Data Echange !" << std::endl;

                //thread timing
                using namespace std::chrono;
                microseconds processInterval(processInterval_microseconds);
                //time_point processStart = high_resolution_clock::now();
                //time_point previousCycleStartTime = now;
                
                uint64_t systemTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                uint64_t processInterval_nanoseconds = duration_cast<nanoseconds>(processInterval).count();
                uint64_t cycleStartTime_nanoseconds = systemTime_nanoseconds + processInterval_nanoseconds;
                uint64_t previousCycleStartTime_nanoseconds = systemTime_nanoseconds;
                    
                uint64_t processStartTime_nanoseconds;
                uint64_t referenceClockStartTime_nanoseconds;

                uint64_t previousReferenceClock_nanoseconds;

                int cycle = 0;

                while (b_processRunning) {

                    //bruteforce timing precision by using 100% of CPU core
                    while (systemTime_nanoseconds < cycleStartTime_nanoseconds) {
                        //update and compare system time to next process timestamp
                        systemTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                    }
                    
                    //send ethercat frame
                    ec_send_processdata();
                    
                    //wait for return of the frame until timeout
                    int workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
                    
                    //adjust the copy of the reference clock in case no frame was received
                    if (workingCounter != 6) ec_DCtime += processInterval_nanoseconds;


                    /*
                    Most DC slaves expect a stable and syncronised PDO transfer during safe-OP. Only when the LRW or LRD/LWR sequence has "proven" to the slave in question it is in sync with the SYNC0 signal the slave will allow transition to OP.
                    So your SOEM application has to:
                        -configure the slave
                        -map the slave
                        -configure distributed clock
                        -go to safe-op
                        -start pdo data transfer (LRW or LRD/LWR) at the desired DC interval (for example 1ms)
                        -check for stable DC clock in all slaves (difference timer)
                        -check for stable master clock (digital PLL locked to reference slave)
                        -only then request OP
                    An example how to do this is in red_test.c in the soem test map.
                    */
                    
                    
                    if (cycle == 0) {
                        processStartTime_nanoseconds = systemTime_nanoseconds;
                        referenceClockStartTime_nanoseconds = ec_DCtime;
                        previousReferenceClock_nanoseconds = ec_DCtime - processInterval_nanoseconds;
                    }

                    static float referenceClockCycleDrift_milliseconds = 0.0;
                    int64_t referenceClockTimeDifference_nanoseconds = ec_DCtime - previousReferenceClock_nanoseconds;
                    referenceClockCycleDrift_milliseconds += (float)(referenceClockTimeDifference_nanoseconds - (int64_t)processInterval_nanoseconds) / 1000000.0l;

                    //log timing and data integrity performance
                    int64_t processTime = systemTime_nanoseconds - processStartTime_nanoseconds;
                    int64_t referenceTime = ec_DCtime - referenceClockStartTime_nanoseconds;
                    float timeDifference_milliseconds = (float)(processTime - referenceTime) / 1000000.0;
                    static float averageTimeDifference = 0.0;
                    averageTimeDifference = averageTimeDifference * 0.99 + timeDifference_milliseconds * 0.01;
                    clockDrift.addPoint(glm::vec2(cycle, referenceClockCycleDrift_milliseconds));
                    averageClockDrift.addPoint(glm::vec2(cycle, referenceClockCycleDrift_milliseconds));
                    timingHistory.addPoint(glm::vec2(cycle, (float)(cycleStartTime_nanoseconds - previousCycleStartTime_nanoseconds)/1000.0));
                    workingCounterHistory.addPoint(glm::vec2(cycle, workingCounter));

                    //TODO: need to make sure the frame gets send exactly in between two sync0 events
                    //currently it just gets send in sync, in relation to the first send time, not in relation the ethercat reference clock
                    //

                    //do axis processing to get next set of output data
                    process();



                    //this code block calculates an offset to be applied to the next cycle start
                    //this effectively aligns the cycle interval of the master to the ethercat reference clock
                    //since the master clock and reference clock will drift over time
                    static int64_t offsetTime = 0;
                    static int64 integral = 0;
                    int64_t delta;
                    // set linux sync point 50us later than DC sync, just as example
                    delta = (ec_DCtime - 50000) % processInterval_nanoseconds;
                    if (delta > (processInterval_nanoseconds / 2)) { delta = delta - processInterval_nanoseconds; }
                    if (delta > 0) { integral++; }
                    if (delta < 0) { integral--; }
                    offsetTime = -(delta / 100) - (integral / 20);
                    

                    //std::cout << ec_DCtime << std::endl;
                    

                    previousReferenceClock_nanoseconds = ec_DCtime;
                    previousCycleStartTime_nanoseconds = cycleStartTime_nanoseconds;
                    cycleStartTime_nanoseconds += processInterval_nanoseconds + offsetTime;
                    cycle++;
                }
            } else {
                std::cout << "===== Not all slaves are operational, cancelling cyclic exchange... " << std::endl;
            }
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