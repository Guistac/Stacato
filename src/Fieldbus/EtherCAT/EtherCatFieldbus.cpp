#include "EtherCatFieldbus.h"

#include <ethercat.h>
#include <iostream>
#include <chrono>

std::vector<NetworkInterfaceCard>   EtherCatFieldbus::networkInterfaceCards;
NetworkInterfaceCard                EtherCatFieldbus::selectedNetworkInterfaceCard;

std::vector<ECatServoDrive>         EtherCatFieldbus::servoDrives;
uint8_t                             EtherCatFieldbus::ioMap[4096];
int                                 EtherCatFieldbus::ioMapSize = 0;

bool                                EtherCatFieldbus::b_networkScanned = false;
bool                                EtherCatFieldbus::b_processRunning = false;
bool                                EtherCatFieldbus::b_ioMapConfigured = false;

ECatMetrics                         EtherCatFieldbus::metrics;

double                              EtherCatFieldbus::processInterval_milliseconds = 10.0;
double                              EtherCatFieldbus::processDataTimeout_milliseconds = 5.0;
std::thread                         EtherCatFieldbus::etherCatRuntime;


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
    metrics.init();
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

        metrics.reset();

        etherCatRuntime = std::thread([]() {

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

            std::cout << "===== Setting All Slaves to Operational state..." << std::endl;
            // Act on slave 0 (a virtual slave used for broadcasting)
            ec_slavet* broadcastSlave = &ec_slave[0];
            broadcastSlave->state = EC_STATE_OPERATIONAL;
            ec_writestate(0);

            //wait for all slaves to reach OP state
            uint16_t slaveState = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
            if (slaveState == EC_STATE_OPERATIONAL) {
                std::cout << "===== All slaves are operational, Starting Cyclic Process Data Echange !" << std::endl;

                using namespace std::chrono;
                
                //thread timing variables
                uint64_t processInterval_nanoseconds = processInterval_milliseconds * 1000000.0L;
                uint64_t processDataTimeout_microseconds = processDataTimeout_milliseconds * 1000.0L;
                uint64_t systemTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count() + processInterval_nanoseconds;
                uint64_t cycleStartTime_nanoseconds = systemTime_nanoseconds + processInterval_nanoseconds;
                uint64_t previousCycleStartTime_nanoseconds;

                //TODO: compute this depending on slave configuration
                int excpectedWorkingCounter = 6;

                while (b_processRunning) {

                    //bruteforce timing precision by using 100% of CPU core
                    //update and compare system time to next process 
                    do { systemTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count(); }
                    while (systemTime_nanoseconds < cycleStartTime_nanoseconds);

                    //send ethercat frame
                    ec_send_processdata();
                    uint64_t frameSentTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                    
                    //wait for return of the frame until timeout
                    int workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
                    uint64_t frameReceivedTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                    
                    //adjust the copy of the reference clock in case no frame was received
                    if (workingCounter != excpectedWorkingCounter) ec_DCtime += processInterval_nanoseconds;

                    //do axis processing to get next set of output data, only process input data if the working counter matches the expected one
                    process(workingCounter == excpectedWorkingCounter);

                    //dctime_offset: *reference clock* offset target for the receiving of a frame by the first dc slave
                    //the offset is calculated as a distance from the -dc sync time-, which is a whole multiple of the process interval time
                    //setting this to half the process interval time garantees a smoothn easily mesureable approach to the target value
                    //this approach can be monitored to check when the frame time is synchronized to the dc sync time
                    //since the frame receive time is offset 50% of the cycle interval, sync0 and sync1 events should be set to happen at dc sync time, or 0% offset.
                    //else they will interfere with the frame receive time, this 50% offset garantees one frame is received per Sync0 or Sync1 Event
                    static uint64_t dctime_offset_nanoseconds = processInterval_nanoseconds / 2;
                    //the code block below calculates an offset correction to be applied to the next cycle start time of the *master clock*
                    //this correction will make sure the frame is will be received at the correct ec_DCtime
                    //this effectively aligns the cycle interval of the master to the ethercat reference clock
                    //since the master clock and reference clock will drift over time, and the frames should be received at the correct slave reference time (ec_DCtime)
                    //this offset ensures that the chosen cycle interval is synchronous with the reference clock and the frames are received at the correct time
                    //this regulating code block is taken from the soem example red_test.c
                    //it implements a PI controller (proportional integral) that regulates ec_DCTime using an offset to be added to the system time
                    static int64_t integral = 0;
                    int64_t delta = (ec_DCtime - dctime_offset_nanoseconds) % processInterval_nanoseconds;
                    if (delta > (processInterval_nanoseconds / 2)) { delta = delta - processInterval_nanoseconds; }
                    if (delta > 0) { integral++; }
                    if (delta < 0) { integral--; }
                    int64_t offsetTime = -(delta / 100) - (integral / 20);
                    //the offset time should be added to the incrementation of the next cycle time of the master clock
                    //calculate the start of the next cycle, taking clock drift compensation into account
                    previousCycleStartTime_nanoseconds = cycleStartTime_nanoseconds;
                    cycleStartTime_nanoseconds += processInterval_nanoseconds + offsetTime;

                    int dummyLoad = 0;
                    for (int i = 0; i < 10000; i++) dummyLoad += std::sin(i);

                    //======= UPDATE METRICS =======

                    float dcTimeError_milliseconds = ((double)(ec_DCtime % processInterval_nanoseconds) - (double)dctime_offset_nanoseconds) / 1000000.0L;
                    if (metrics.cycleCounter == 0) {
                        metrics.startTime_nanoseconds = systemTime_nanoseconds;
                        metrics.averageDcTimeError_milliseconds = dcTimeError_milliseconds;
                    }
                    metrics.averageDcTimeError_milliseconds = metrics.averageDcTimeError_milliseconds * 0.99 + 0.01 * dcTimeError_milliseconds;

                    metrics.processTime_nanoseconds = systemTime_nanoseconds - metrics.startTime_nanoseconds;
                    metrics.processTime_seconds = (double)(systemTime_nanoseconds - metrics.startTime_nanoseconds) / 1000000000.0L;

                    double frameSendDelay_milliseconds = (double)(frameSentTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                    double frameReceiveDelay_milliseconds = (double)(frameReceivedTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                    double timeoutDelay_milliseconds = frameSendDelay_milliseconds + processDataTimeout_milliseconds;
                    double cycleLength_milliseconds = (double)(cycleStartTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                    
                    metrics.dcTimeErrors.addPoint(          glm::vec2(metrics.processTime_seconds, dcTimeError_milliseconds));
                    metrics.averageDcTimeErrors.addPoint(   glm::vec2(metrics.processTime_seconds, metrics.averageDcTimeError_milliseconds));
                    metrics.sendDelays.addPoint(            glm::vec2(metrics.processTime_seconds, frameSendDelay_milliseconds));
                    metrics.receiveDelays.addPoint(         glm::vec2(metrics.processTime_seconds, frameReceiveDelay_milliseconds));
                    metrics.timeoutDelays.addPoint(         glm::vec2(metrics.processTime_seconds, timeoutDelay_milliseconds));
;                   metrics.cycleLengths.addPoint(          glm::vec2(metrics.processTime_seconds, cycleLength_milliseconds));

                    
                    metrics.addWorkingCounter(workingCounter, metrics.processTime_seconds);

                    if (workingCounter == EC_NOFRAME) metrics.timeouts.addPoint(glm::vec2(metrics.processTime_seconds, frameReceiveDelay_milliseconds));

                    uint64_t processedTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                    double processDelay_milliseconds = (double)(processedTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                    metrics.processDelays.addPoint(glm::vec2(metrics.processTime_seconds, processDelay_milliseconds));

                    metrics.cycleCounter++;

                    //======= END UPDATE METRICS =======
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


void EtherCatFieldbus::process(bool inputDataValid) {
    for (ECatServoDrive& drive : servoDrives) drive.process(inputDataValid);
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