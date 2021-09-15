#include <pch.h>

#include "EtherCatFieldbus.h"

#include "Utilities/EtherCatDeviceFactory.h"
#include "Environnement/Environnement.h"

namespace EtherCatFieldbus {

    std::vector<NetworkInterfaceCard> networkInterfaceCards;
    NetworkInterfaceCard networkInterfaceCard;
    NetworkInterfaceCard redundantNetworkInterfaceCard;
    bool b_redundant = false;

    std::vector<std::shared_ptr<EtherCatSlave>> slaves;
    uint8_t ioMap[4096];
    int ioMapSize = 0;
    int expectedWorkingCounter;

    bool b_networkOpen = false;
    bool b_processStarting = false;
    bool b_configurationError = false;
    int i_configurationProgress = 0;
    char configurationStatus[128] = "";
    bool b_processRunning = false;
    bool b_clockStable = false;
    bool b_allOperational = false;

    EtherCatMetrics metrics;

    double processInterval_milliseconds = 3.0;
    double processDataTimeout_milliseconds = 1.5;
    double clockStableThreshold_milliseconds = 0.1;
    int slaveStateCheckCycleCount = 50;

    std::thread etherCatRuntime;    //thread to read errors encountered by SOEM
    std::thread errorWatcher;       //cyclic exchange thread (needs a full cpu core to achieve precise timing)

    //non public function
    void setup();
    bool configureSlaves();
    void startCyclicExchange();


    //------ Find Network Interface Cards --------

    void updateNetworkInterfaceCardList() {
        Logger::info("===== Refreshing Network Interface Card List");
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
        Logger::info("===== Found {} Network Interface Card{}", networkInterfaceCards.size(), networkInterfaceCards.size() == 1 ? "" : "s");
        for (NetworkInterfaceCard& nic : networkInterfaceCards)
            Logger::debug("    = {} (ID: {})", nic.description, nic.name);
    }




    //----- Initialize A Single or A Pair of Network Interface Cards -----

    bool init(NetworkInterfaceCard& nic) {
        if (b_networkOpen) terminate();
        networkInterfaceCard = std::move(nic);
        Logger::debug("===== Initializing EtherCAT Fieldbus on Network Interface Card '{}'", networkInterfaceCard.description);
        int nicInitResult = ec_init(networkInterfaceCard.name);
        if (nicInitResult < 0) {
            Logger::error("===== Failed to initialize network interface card ...");
            b_networkOpen = false;
            b_redundant = false;
            return false;
        }
        b_redundant = false;
        b_networkOpen = true;
        Logger::info("===== Initialized network interface card '{}'", networkInterfaceCard.description);
        setup();
        return true;
    }

    bool init(NetworkInterfaceCard& nic, NetworkInterfaceCard& redNic) {
        if (b_networkOpen) terminate();
        networkInterfaceCard = std::move(nic);
        redundantNetworkInterfaceCard = std::move(redNic);
        Logger::debug("===== Initializing EtherCAT Fieldbus on Network Interface Card '{}' with redundancy on '{}'", networkInterfaceCard.description, redundantNetworkInterfaceCard.description);
        int nicInitResult = ec_init_redundant(networkInterfaceCard.name, redundantNetworkInterfaceCard.name);
        if (nicInitResult < 0) {
            Logger::error("===== Failed to initialize network interface cards ...");
            b_networkOpen = false;
            b_redundant = false;
            return false;
        }
        b_redundant = true;
        b_networkOpen = true;
        Logger::info("===== Initialized network interface cards '{}' & '{}'", networkInterfaceCard.description, redundantNetworkInterfaceCard.description);
        setup();
        return true;
    }

    void setup() {
        errorWatcher = std::thread([]() {
            Logger::debug("===== Started EtherCAT Error Watchdog");
            while (b_networkOpen) {
                while (EcatError) Logger::error("##### EtherCAT Error: {}", ec_elist2string());
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            Logger::debug("===== Exited EtherCAT Error Watchdog");
            });
        scanNetwork();
        metrics.init(processInterval_milliseconds);
    }




    //----- Stop Using A Network Interface Card -----

    void terminate() {
        stop();
        Logger::debug("===== Closing EtherCAT Network Interface Card");
        ec_close();
        b_networkOpen = false;
        errorWatcher.join();
        Logger::info("===== Stopped EtherCAT fieldbus");
    }





    //----- Scan The Fieldbus For Slaves -----

    bool scanNetwork() {

        i_configurationProgress = 0;
        sprintf(configurationStatus, "Scanning Network");

        //when rescanning the network, all previous slaves are now considered to be offline before being detected again
        //Environnement::setAllEtherCatSlavesOffline();
        //don't delete regular slaves since they might be in the environnement, just clear the list
        slaves.clear();

        //setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
        int workingCounter = ec_config_init(FALSE); //what is usetable??

        if (workingCounter > 0) {
            Logger::info("===== Found and Configured {} EtherCAT Slave{}", ec_slavecount, ((ec_slavecount == 1) ? ": " : "s: "));

            for (int i = 1; i <= ec_slavecount; i++) {
                ec_slavet& slv = ec_slave[i];
                //create device class depending on slave name
                std::shared_ptr<EtherCatSlave> slave = EtherCatDeviceFactory::getDeviceByName(slv.name);
                //we need the station alias to be able to compare the slave to the environnement slaves
                slave->stationAlias = slv.aliasadr;

                bool environnementHasSlave = Environnement::hasEtherCatSlave(slave);

                //compare the slave with existing slaves in the environnement
                if (environnementHasSlave) {
                    std::shared_ptr<EtherCatSlave>matchingSlave = Environnement::getMatchingEtherCatSlave(slave);
                    //if the slave is already in the environnement
                    //transfer the identity object so the environnement slave has the newly acquired one
                    //(the old one is obsolete since we rescanned the network)
                    matchingSlave->identity = &slv;
                    //also reassign index, since it might have changed
                    matchingSlave->slaveIndex = i;
                    //set slave to online
                    //matchingSlave->setOnline(true);
                    //delete the slave we just created since its only use was to match the environnement slave and transfer its identity
                    //reassign so we can log the slave and add it to the slave list
                    slave = matchingSlave;
                }
                else {
                    //if the slave is not in the environnement
                    //add it to the available slave list
                    slave->identity = &slv;
                    slave->slaveIndex = i;
                    //slave->setOnline(true);
                    char name[128];
                    sprintf(name, "#%i '%s' @%i", slave->getSlaveIndex(), slave->getNodeName(), slave->getStationAlias());
                    slave->setName(name);
                    //add the slave to the list of unassigned slaves (not in the environnement)
                }

                //add the slave to the list of slaves regardless of environnement presence
                slaves.push_back(slave);

                Logger::info("    = Slave {} : '{}'  Address: {}  StationAlias {}  KnownDevice: {}  InEnvironnement: {}",
                    slave->getSlaveIndex(),
                    slave->getNodeName(),
                    slave->getAssignedAddress(),
                    slave->getStationAlias(),
                    slave->isSlaveKnown() ? "Yes" : "No",
                    environnementHasSlave ? "Yes" : "No");
            }
            return true;
        }

        //if no device was found on the network
        b_configurationError = true;
        sprintf(configurationStatus, "Found no EtherCAT slaves on the network");
        Logger::warn("===== No EtherCAT Slaves found...");
        return false;
    }



    //----- Map Slave Memory And do Slave configuration -----

    bool configureSlaves() {


        i_configurationProgress = 1;
        sprintf(configurationStatus, "Configuring Distributed Clocks");

        Logger::debug("===== Configuring Distributed Clocks");
        if (!ec_configdc()) {
            b_configurationError = true;
            sprintf(configurationStatus, "Could not configuredistributed clocks...");
            Logger::error("===== Could not configure distributed clocks ...");
            return false;
        }
        Logger::info("===== Finished Configuring Distributed Clocks");



        //build ioMap for PDO data, configure FMMU and SyncManager, request SAFE-OP state for all slaves
        //also assign and execute slave startup methods
        Logger::debug("===== Begin Building I/O Map and Slave Configuration...");

        for (int i = 1; i <= ec_slavecount; i++) {
            //set hook callback for configuring the PDOs of each slave
            //we don't use the PO2SOconfigx hook since it isn't supported by ec_reconfig_slave()
            ec_slave[i].PO2SOconfig = [](uint16_t slaveIndex) -> int {
                for (auto slave : slaves) {
                    if (slave->getSlaveIndex() == slaveIndex) {
                        sprintf(configurationStatus, "Configuring Slave #%i '%s'", slave->getSlaveIndex(), slave->getName());
                        i_configurationProgress = slaveIndex + 1; //for progress bar display
                        slave->b_mapped = slave->startupConfiguration();
                        break;
                    }
                }
                return 0;
            };
        }
        ioMapSize = ec_config_map(ioMap); //this function starts the configuration
        if (ioMapSize <= 0) {
            b_configurationError = true;
            sprintf(configurationStatus, "Failed to Configure I/O Map");
            for (auto slave : slaves) slave->b_mapped = false;
            Logger::error("===== Failed To Configure I/O Map...");
            return false;
        }
        Logger::info("===== Finished Building I/O Map (Size : {} bytes)", ioMapSize);


        expectedWorkingCounter = 0;
        for (auto slave : slaves) {
            if (slave->identity->Ibits > 0 && slave->identity->Obits > 0) expectedWorkingCounter += 3;
            else if (slave->identity->Ibits > 0 || slave->identity->Obits) expectedWorkingCounter += 1;
        }
        Logger::info("===== Calculated Expected Working Counter: {}", expectedWorkingCounter);


        for (auto slave : slaves) {
            Logger::debug("   [{}] '{}' {} bytes ({} bits)",
                slave->getSlaveIndex(),
                slave->getNodeName(),
                slave->identity->Ibytes + slave->identity->Obytes,
                slave->identity->Ibits + slave->identity->Obits);
            Logger::debug("          Inputs: {} bytes ({} bits)", slave->identity->Ibytes, slave->identity->Ibits);
            Logger::debug("          Outputs: {} bytes ({} bits)", slave->identity->Obytes, slave->identity->Obits);
        }

        i_configurationProgress = ec_slavecount + 2;
        sprintf(configurationStatus, "Checking For Safe-Operational State");

        Logger::debug("===== Checking For Safe-Operational State...");
        if (ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE) != EC_STATE_SAFE_OP) {
            b_configurationError = true;
            sprintf(configurationStatus, "Not All Slaves Reached Safe-Operational State");
            Logger::error("===== Not all slaves have reached Safe-Operational State...");
            return false;
        }
        Logger::info("===== All slaves are Safe-Operational");

        return true;
    }


    //----- Start EtherCAT Fieldbus ------

    void startCyclicExchange() {
        //don't allow the thread to start if it is already running
        if (b_processRunning) return;

        i_configurationProgress = ec_slavecount + 3;
        sprintf(configurationStatus, "Starting Cyclic Exchange");
        Logger::debug("===== Starting Cyclic Process Data Exchange");

        b_processRunning = true;
        b_clockStable = false;

        metrics.init(processInterval_milliseconds);

        etherCatRuntime = std::thread([]() {

            //thread timing variables
            using namespace std::chrono;
            uint64_t processInterval_nanoseconds = processInterval_milliseconds * 1000000.0L;
            uint64_t processDataTimeout_microseconds = processDataTimeout_milliseconds * 1000.0L;
            uint64_t systemTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count() + processInterval_nanoseconds;
            uint64_t cycleStartTime_nanoseconds = systemTime_nanoseconds + processInterval_nanoseconds;
            uint64_t previousCycleStartTime_nanoseconds;

            Logger::info("===== Waiting For clocks to stabilize before requesting Operational State...");

            //initialize average clock error to its max absolute value;
            metrics.averageDcTimeError_milliseconds = processInterval_milliseconds / 2.0;

            i_configurationProgress = ec_slavecount + 4;
            sprintf(configurationStatus, "Waiting For Clocks To Stabilize");

            while (b_processRunning) {

                //bruteforce timing precision by using 100% of CPU core
                //update and compare system time to next process 
                do { systemTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count(); } while (systemTime_nanoseconds < cycleStartTime_nanoseconds);

                //send ethercat frame
                ec_send_processdata();
                uint64_t frameSentTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                //TODO: in case sending the data takes longer than a certain time, the rest of the loop should be aborted

                //wait for return of the frame until timeout
                int workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
                uint64_t frameReceivedTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();

                //adjust the copy of the reference clock in case no frame was received
                if (workingCounter != expectedWorkingCounter) ec_DCtime += processInterval_nanoseconds;

                //interpret the data that was received for all slaves
                for (auto slave : slaves) slave->readInputs();
                /*
                //process the data to generate output values, taking into account if new data wasn't received
                //(auto slave : slaves) slave->process(workingCounter == expectedWorkingCounter);
                */

                Environnement::nodeGraph.evaluate(DeviceType::ETHERCATSLAVE);

                //prepare the output data to be sent
                for (auto slave : slaves) slave->prepareOutputs();

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
                if (metrics.cycleCounter == 0) integral = 0; //this value should be reset on cyclic exchange start
                int64_t delta = (ec_DCtime - dctime_offset_nanoseconds) % processInterval_nanoseconds;
                if (delta > (processInterval_nanoseconds / 2)) { delta = delta - processInterval_nanoseconds; }
                if (delta > 0) { integral++; }
                if (delta < 0) { integral--; }
                int64_t offsetTime = -(delta / 100) - (integral / 20);
                //the offset time should be added to the incrementation of the next cycle time of the master clock
                //calculate the start of the next cycle, taking clock drift compensation into account
                previousCycleStartTime_nanoseconds = cycleStartTime_nanoseconds;
                cycleStartTime_nanoseconds += processInterval_nanoseconds + offsetTime;

                //======= UPDATE METRICS =======

                float dcTimeError_milliseconds = ((double)(ec_DCtime % processInterval_nanoseconds) - (double)dctime_offset_nanoseconds) / 1000000.0L;
                if (metrics.cycleCounter == 0) {
                    metrics.startTime_nanoseconds = systemTime_nanoseconds;
                }
                metrics.averageDcTimeError_milliseconds = metrics.averageDcTimeError_milliseconds * 0.97 + 0.03 * abs(dcTimeError_milliseconds);

                metrics.processTime_nanoseconds = systemTime_nanoseconds - metrics.startTime_nanoseconds;
                metrics.processTime_seconds = (double)(systemTime_nanoseconds - metrics.startTime_nanoseconds) / 1000000000.0L;

                double frameSendDelay_milliseconds = (double)(frameSentTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                double frameReceiveDelay_milliseconds = (double)(frameReceivedTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                double timeoutDelay_milliseconds = frameSendDelay_milliseconds + processDataTimeout_milliseconds;
                double cycleLength_milliseconds = (double)(cycleStartTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;

                metrics.dcTimeErrors.addPoint(glm::vec2(metrics.processTime_seconds, dcTimeError_milliseconds));
                metrics.averageDcTimeErrors.addPoint(glm::vec2(metrics.processTime_seconds, metrics.averageDcTimeError_milliseconds));
                metrics.sendDelays.addPoint(glm::vec2(metrics.processTime_seconds, frameSendDelay_milliseconds));
                metrics.receiveDelays.addPoint(glm::vec2(metrics.processTime_seconds, frameReceiveDelay_milliseconds));
                metrics.timeoutDelays.addPoint(glm::vec2(metrics.processTime_seconds, timeoutDelay_milliseconds));
                metrics.cycleLengths.addPoint(glm::vec2(metrics.processTime_seconds, cycleLength_milliseconds));;
                metrics.addWorkingCounter(workingCounter, metrics.processTime_seconds);

                if (workingCounter == EC_NOFRAME) metrics.timeouts.addPoint(glm::vec2(metrics.processTime_seconds, frameReceiveDelay_milliseconds));

                uint64_t processedTime_nanoseconds = duration_cast<nanoseconds>(high_resolution_clock::now().time_since_epoch()).count();
                double processDelay_milliseconds = (double)(processedTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
                metrics.processDelays.addPoint(glm::vec2(metrics.processTime_seconds, processDelay_milliseconds));

                metrics.cycleCounter++;

                //======= HANDLE STATE TRANSITIONS =======

                if (!b_clockStable && abs(metrics.averageDcTimeError_milliseconds) < clockStableThreshold_milliseconds) {
                    b_clockStable = true;
                    sprintf(configurationStatus, "Setting All Slaves to Operational State");
                    std::thread safeOpStarter([]() {
                        Logger::debug("===== Clocks Stabilized, Setting All Slaves to Operational state...");
                        // Act on slave 0 (a virtual slave used for broadcasting)
                        ec_slave[0].state = EC_STATE_OPERATIONAL;
                        ec_writestate(0);
                        //wait for all slaves to reach OP state
                        uint16_t slaveState = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
                        if (slaveState == EC_STATE_OPERATIONAL) {
                            sprintf(configurationStatus, "Successfully Started EtherCAT Fieldbus");
                            b_allOperational = true;
                            Logger::info("===== All slaves are operational");
                            Logger::info("===== Successfully started EtherCAT Fieldbus");
                        }
                        else {
                            stop();
                            b_configurationError = true;
                            sprintf(configurationStatus, "Not all slaves reached Operational State");
                            Logger::error("===== Not all slaves reached operational state... ");
                        }
                        });
                    safeOpStarter.detach();
                }
                else if (metrics.cycleCounter % slaveStateCheckCycleCount == 0) {
                    //read the state of all slaves
                    for (auto slave : slaves) slave->saveCurrentState();
                    ec_readstate();
                    for (auto slave : slaves) slave->compareNewState();
                }

                //===== EtherCat Runtime End =====
            }
            b_processRunning = false;
            });
    }

    void start() {
        if (!b_processStarting) {
            b_configurationError = false;
            i_configurationProgress = 0;
            sprintf(configurationStatus, "Starting Fieldbus Configuration");
            Logger::info("===== Starting Fieldbus Configuration");
            b_processStarting = true;
            std::thread etherCatProcessStarter([]() {
                if (scanNetwork() && configureSlaves()) startCyclicExchange();
                b_processStarting = false;
                });
            etherCatProcessStarter.detach();
        }
    }

    void stop() {
        if (b_processRunning) {
            Logger::info("===== Stopping Cyclic Exchange");
            b_allOperational = false;
            b_processRunning = false;
            if (etherCatRuntime.joinable()) etherCatRuntime.join();
            Logger::info("===== Cyclic Exchange Stopped !");
        }
    }

}