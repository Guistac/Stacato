#include <pch.h>

#include "EtherCatFieldbus.h"

#include "Utilities/EtherCatDeviceFactory.h"
#include "Environnement/Environnement.h"
#include "Utilities/EtherCatError.h"

namespace EtherCatFieldbus {

    std::vector<NetworkInterfaceCard> networkInterfaceCards;
    NetworkInterfaceCard networkInterfaceCard;
    NetworkInterfaceCard redundantNetworkInterfaceCard;
    bool b_redundant = false;

    std::vector<std::shared_ptr<EtherCatSlave>> slaves;
    std::vector<std::shared_ptr<EtherCatSlave>> slaves_unassigned;
    uint8_t ioMap[4096];
    int ioMapSize = 0;

    bool b_networkOpen = false;             //high when one or more network interface cards are opened
    bool b_processStarting = false;         //high during initial fieldbus setup, before starting cyclic exchange (prevents concurrent restarting)   
    bool b_processRunning = false;          //high while the cyclic exchange is running (also controls its shutdown)
    bool b_clockStable = false;             //high when clock drift is under the threshold value
    bool b_allOperational = false;          //high when all states reached operational state after clock stabilisation, indicates successful fiedlbus configuration

    bool isNetworkInitialized() { return b_networkOpen; }
    bool isCyclicExchangeStarting() { return b_processStarting; }
    bool isCyclicExchangeActive() { return b_processRunning; }
    bool isCyclicExchangeStartSuccessfull() { return b_processRunning && b_clockStable && b_allOperational; }

    bool b_startupError = false;
    int i_startupProgress = 0;
    char startupStatusString[128] = "";

    EtherCatMetrics metrics;

    double processInterval_milliseconds = 3.0;
    double processDataTimeout_milliseconds = 1.5;
    double clockStableThreshold_milliseconds = 0.1;
    int slaveStateCheckCycleCount = 50;
    double fieldbusTimeout_milliseconds = 100.0;
    std::chrono::high_resolution_clock::time_point lastProcessDataFrameReturnTime;

    bool b_detectionHandlerRunning = false;
    std::thread slaveDetectionHandler;
    std::thread etherCatRuntime;    //thread to read errors encountered by SOEM
    std::thread slaveStateHandler;  //thread to periodically check the state of all slaves and recover them if necessary
    bool b_errorWatcherRunning = false;
    std::thread errorWatcher;       //cyclic exchange thread (needs a full cpu core to axchieve precise timing)

    std::chrono::high_resolution_clock::time_point previousCycleTime;
    double currentCycleDeltaT_seconds = 0.0;

    //====== non public functions ======

    void setup();

    //SOEM extension to read Explicit Device ID
    bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID);

    bool configureSlaves();
    void startCyclicExchange();
    void cyclicExchange();
    void transitionToOperationalState();
    void handleStateTransitions();

    void startSlaveDetectionHandler();
    void stopSlaveDetectionHandler();
    void startErrorWatcher();
    void stopErrorWatcher();

    bool configureSlaves();
    void startCyclicExchange();


    //============ LIST NETWORK INTERFACE CARDS ===============

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

    //============== INTIALIZE FIELDBUS WITH AND OPEN NETWORK INTERFACE CARD ==============

    bool init(NetworkInterfaceCard& nic) {
        if (b_networkOpen) terminate();
        networkInterfaceCard = nic;
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
        networkInterfaceCard = nic;
        redundantNetworkInterfaceCard = redNic;
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
        startErrorWatcher();
        scanNetwork();
        metrics.init(processInterval_milliseconds);
    }

    //============= STOP FIELDBUS AND CLOSE NETWORK INTERFACE CARD ==============

    void terminate() {
        stop();
        Logger::debug("===== Closing EtherCAT Network Interface Card");
        b_networkOpen = false;
        stopErrorWatcher();
        if (errorWatcher.joinable()) errorWatcher.join();
        if (etherCatRuntime.joinable()) etherCatRuntime.join();
        if (slaveStateHandler.joinable()) slaveStateHandler.join();
        stopSlaveDetectionHandler();
        ec_close();
        Logger::info("===== Stopped EtherCAT fieldbus");
    }

    //================= START CYCLIC EXCHANGE =================

    void start() {
        if (!b_processStarting) {
            stopSlaveDetectionHandler();
            if (etherCatRuntime.joinable()) etherCatRuntime.join();
            if (slaveStateHandler.joinable()) slaveStateHandler.join();
            b_startupError = false;
            i_startupProgress = 0;
            sprintf(startupStatusString, "Starting Fieldbus Configuration");
            Logger::info("===== Starting Fieldbus Configuration");
            b_processStarting = true;
            std::thread etherCatProcessStarter([]() {
                if (scanNetwork() && configureSlaves()) startCyclicExchange();
                b_processStarting = false;
                });
            etherCatProcessStarter.detach();
        }
    }

    //================= STOP CYCLIC EXCHANGE ===================

    void stop() {
        if (b_processRunning) {
            Logger::debug("===== Stopping Cyclic Exchange...");
            for (auto slave : slaves) {
                if (slave->isOnline()) {
                    slave->resetData();
                    slave->pushEvent("Device Disconnected (Fieldbus Shutdown)", false);
                    slave->onDisconnection();
                }
                slave->identity->state = EC_STATE_NONE;
            }
            b_allOperational = false;
            b_processRunning = false;
            startSlaveDetectionHandler();
        }
    }


    //========== General Error Watcher Utility ============

    void startErrorWatcher() {
        b_errorWatcherRunning = true;
        errorWatcher = std::thread([]() {
            Logger::debug("===== Started EtherCAT Error Watchdog");
            while (b_errorWatcherRunning) {
                while (EtherCatError::hasError()) EtherCatError::logError();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            Logger::debug("===== Exited EtherCAT Error Watchdog");
            });
    }

    void stopErrorWatcher() {
        b_errorWatcherRunning = false;
        if (errorWatcher.joinable()) errorWatcher.join();
    }

    //============ Check Explicit ID compatibility and Read explicit ID of slave ============

    bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID) {
        int maxTries = 4;
        int tries = 0;
        while (tries < maxTries) {
            int wc = 0;
            uint16_t ALstatus;
            wc = ec_FPRD(configAddress, 0x130, 2, &ALstatus, EC_TIMEOUTSAFE);
            uint16_t ALcontrol = ALstatus |= 0x20;
            wc = ec_FPWR(configAddress, 0x120, 2, &ALcontrol, EC_TIMEOUTSAFE);
            wc = ec_FPRD(configAddress, 0x130, 2, &ALstatus, EC_TIMEOUTSAFE);
            bool supported = ALstatus & 0x20;
            if (supported) {
                uint16_t ALstatusCode;
                wc = ec_FPRD(configAddress, 0x134, 2, &ALstatusCode, EC_TIMEOUTSAFE);
                ID = ALstatusCode;
            }
            ALcontrol &= ~0x20;
            ec_FPWR(configAddress, 0x120, 2, &ALcontrol, EC_TIMEOUTSAFE);
            if (supported) return true;
            tries++;
        }
        return false;
    }


    //=================== DISCOVER ETHERCAT DEVICES ON THE NETWORK =====================

    bool scanNetwork() {

        i_startupProgress = 0;
        sprintf(startupStatusString, "Scanning Network");

        stopSlaveDetectionHandler();

        //when rescanning the network, all previous slaves are now considered to be offline before being detected again
        //for a slave to appear as offline, we set its identity object (ec_slavet) to nullptr
        for (auto slave : slaves) slave->identity = nullptr;
        //we clear the list of slaves, slaves that are in the node graph remain there
        slaves.clear();
        //we also clear the list of slaves that were not in the nodegraph
        slaves_unassigned.clear();

        //setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
        int workingCounter = ec_config_init(FALSE); //what is usetable??

        if (workingCounter > 0) {
            Logger::info("===== Found and Configured {} EtherCAT Slave{}", ec_slavecount, ((ec_slavecount == 1) ? ": " : "s: "));

            for (int i = 1; i <= ec_slavecount; i++) {
                ec_slavet& identity = ec_slave[i];                

                Logger::info("    = Slave {} : '{}'  Address: {}", i, identity.name, identity.configadr);

                uint16_t explicitDeviceID;
                bool explicitDeviceIdSupported = getExplicitDeviceID(identity.configadr, explicitDeviceID);
                if (explicitDeviceIdSupported) Logger::debug("    = Explicit Device ID: {}", explicitDeviceID);
                else Logger::debug("      Explicit Device ID is not supported");
                uint16_t stationAlias = identity.aliasadr;
                Logger::debug("      Station Alias: {}", stationAlias);
                
                std::shared_ptr<EtherCatSlave> slave = nullptr;

                for (auto environnementSlave : Environnement::getEtherCatSlaves()) {
                    if (strcmp(environnementSlave->getNodeName(), identity.name) != 0) continue;
                    switch (environnementSlave->identificationType) {
                        case EtherCatSlaveIdentification::Type::STATION_ALIAS:
                            if (environnementSlave->stationAlias == stationAlias) {
                                slave = environnementSlave;
                                Logger::info("      Matched Environnement Slave by Name & Station Alias");
                                break;
                            }
                            else continue;
                        case EtherCatSlaveIdentification::Type::EXPLICIT_DEVICE_ID:
                            if (environnementSlave->explicitDeviceID == explicitDeviceID) {
                                slave = environnementSlave;
                                Logger::info("      Matched Environnement Slave by Name & Explicit Device ID");
                                break;
                            }
                            else continue;
                    }
                }

                if (slave == nullptr) {
                    Logger::info("      Slave did not match any Environnement Slave");
                    slave = EtherCatDeviceFactory::getDeviceByName(identity.name);
                    slave->stationAlias = stationAlias;
                    slave->explicitDeviceID = explicitDeviceID;
                    char name[128];
                    if (explicitDeviceIdSupported && explicitDeviceID != 0) {
                        slave->identificationType = EtherCatSlaveIdentification::Type::EXPLICIT_DEVICE_ID;
                        sprintf(name, "%s (ID:%i)", slave->getNodeName(), slave->explicitDeviceID);
                    }
                    else {
                        slave->identificationType = EtherCatSlaveIdentification::Type::STATION_ALIAS;
                        sprintf(name, "%s (Alias:%i)", slave->getNodeName(), slave->stationAlias);
                    }
                    slave->setName(name);
                    slaves_unassigned.push_back(slave);
                }
                
                slave->identity = &identity;
                slave->slaveIndex = i;

                if (!slave->isSlaveKnown()) {
                    Logger::warn("Found Unknown Slave: {}", identity.name);
                }
               
                //add the slave to the list of slaves regardless of environnement presence
                slaves.push_back(slave);
            }

            startSlaveDetectionHandler();

            return true;
        }

        //if no device was found on the network
        b_startupError = true;
        sprintf(startupStatusString, "Found no EtherCAT slaves on the network");
        Logger::warn("===== No EtherCAT Slaves found...");
        return false;
    }

    //================ Slave Pinging Utility during no cyclic exchange ====================

    void startSlaveDetectionHandler() {
        if (b_detectionHandlerRunning) return;
        b_detectionHandlerRunning = true;
        slaveDetectionHandler = std::thread([]() {
            Logger::debug("Started EtherCAT Detection Handler");
            while (b_detectionHandlerRunning) {
                ec_readstate();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            Logger::debug("Exited EtherCAT Detection Handler");
        });
    }

    void stopSlaveDetectionHandler() {
        b_detectionHandlerRunning = false;
        if (slaveDetectionHandler.joinable()) slaveDetectionHandler.join();
    }

    //=========== Map Slave memory, Configure Distributed Clocks, Do Per slave configuration, Transition to Safe Operational ===============

    bool configureSlaves() {

        i_startupProgress = 1;
        sprintf(startupStatusString, "Configuring Distributed Clocks");

        Logger::debug("===== Configuring Distributed Clocks");
        if (!ec_configdc()) {
            b_startupError = true;
            sprintf(startupStatusString, "Could not configuredistributed clocks...");
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
                        sprintf(startupStatusString, "Configuring Slave #%i '%s'", slave->getSlaveIndex(), slave->getName());
                        i_startupProgress = slaveIndex + 1; //for progress bar display
                        Logger::debug("Configuring Slave '{}'", slave->getName());
                        if (slave->startupConfiguration()) Logger::debug("Successfully configured Slave '{}'", slave->getName());
                        else Logger::warn("Failed to configure slave '{}'", slave->getName());
                        break;
                    }
                }
                return 0;
            };
        }
        ioMapSize = ec_config_map(ioMap); //this function starts the configuration
        if (ioMapSize <= 0) {
            b_startupError = true;
            sprintf(startupStatusString, "Failed to Configure I/O Map");
            Logger::error("===== Failed To Configure I/O Map...");
            return false;
        }
        Logger::info("===== Finished Building I/O Map (Size : {} bytes)", ioMapSize);

        for (auto slave : slaves) {
            Logger::debug("   [{}] '{}' {} bytes ({} bits)",
                slave->getSlaveIndex(),
                slave->getNodeName(),
                slave->identity->Ibytes + slave->identity->Obytes,
                slave->identity->Ibits + slave->identity->Obits);
            Logger::debug("          Inputs: {} bytes ({} bits)", slave->identity->Ibytes, slave->identity->Ibits);
            Logger::debug("          Outputs: {} bytes ({} bits)", slave->identity->Obytes, slave->identity->Obits);
        }

        i_startupProgress = ec_slavecount + 2;
        sprintf(startupStatusString, "Checking For Safe-Operational State");

        Logger::debug("===== Checking For Safe-Operational State...");
        if (ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE) != EC_STATE_SAFE_OP) {
            b_startupError = true;
            sprintf(startupStatusString, "Not All Slaves Reached Safe-Operational State");
            Logger::error("===== Not all slaves have reached Safe-Operational State...");
            for (auto slave : slaves) {
                if (!slave->isStateSafeOperational() || !slave->isStateOperational() || slave->hasStateError()) {
                    Logger::warn("Slave '{}' has state {}", slave->getName(), slave->getEtherCatStateChar());
                }
            }
            return false;
        }
        Logger::info("===== All slaves are Safe-Operational");

        return true;
    }

    //========= START CYCLIC EXCHANGE ============

    void startCyclicExchange() {
        //don't allow the thread to start if it is already running
        if (b_processRunning) return;

        i_startupProgress = ec_slavecount + 3;
        sprintf(startupStatusString, "Starting Cyclic Exchange");
        Logger::debug("===== Starting Cyclic Process Data Exchange");

        b_processRunning = true;
        b_clockStable = false;

        metrics.init(processInterval_milliseconds);

        etherCatRuntime = std::thread([]() { cyclicExchange(); });
    }


    //======================== CYCLIC DATA EXCHANGE =========================

    void cyclicExchange() {
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

        i_startupProgress = ec_slavecount + 4;
        sprintf(startupStatusString, "Waiting For Clocks To Stabilize");

        //reset fieldbus timeout watchdog
        lastProcessDataFrameReturnTime = high_resolution_clock::now();

        //slaves are considered online when they are detected and actively exchanging data with the master
        //if we reached this state of the configuration, all slaves are detected and we are about to start exchanging data
        //we can set trigger the onConnection event of all slaves
        for (auto slave : slaves) {
            slave->resetData();
            slave->pushEvent("Device Connected", false);
            slave->onConnection();
        }

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


            high_resolution_clock::time_point now = high_resolution_clock::now();
            if (workingCounter <= 0) {
                //check timeout watchdog
                if (now - lastProcessDataFrameReturnTime > milliseconds((int)fieldbusTimeout_milliseconds)) {
                    Logger::critical("Fieldbus timed out...");
                    stop();
                    break; //breaks out of the main while loop
                }
                //adjust the copy of the reference clock in case no frame was received
                ec_DCtime += processInterval_nanoseconds;
            }
            else {
                //reset timeout watchdog
                lastProcessDataFrameReturnTime = now;
            }

            long long currentCycleDeltaT_nanoseconds = duration_cast<nanoseconds>(now - previousCycleTime).count();
            currentCycleDeltaT_seconds = (double)currentCycleDeltaT_nanoseconds / 1000000000.0;
            previousCycleTime = now;

            //don't update the devices while the fieldbus isn't fully started
            if (b_allOperational) {
                //interpret the data that was received for all slaves
                for (auto slave : slaves) if (slave->isStateOperational()) slave->readInputs();
                //update all nodes connected to ethercat slave nodes
                Environnement::nodeGraph.evaluate(DeviceType::ETHERCATSLAVE);
                //prepare the output data to be sent
                for (auto slave : slaves) if (slave->isStateOperational()) slave->prepareOutputs();
            }

            //========= HANDLE MASTER AND REFERENCE CLOCK DRIFT ===========

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
                sprintf(startupStatusString, "Setting All Slaves to Operational State");
                std::thread opStateHandler([]() { transitionToOperationalState(); });
                opStateHandler.detach();
            }

            //===== EtherCat Runtime End =====
        }
        b_processRunning = false;
        Logger::info("===== Cyclic Exchange Stopped !");
    }

    //============== TRANSITION ALL SLAVES TO OPERATIONAL AFTER REFERENCE CLOCK AND MASTER CLOCKS ALIGNED ===================

    void transitionToOperationalState() {
        Logger::debug("===== Clocks Stabilized, Setting All Slaves to Operational state...");
        //set all slaves to operational (by setting slave 0 to operational)
        ec_slave[0].state = EC_STATE_OPERATIONAL;
        ec_writestate(0);
        //wait for all slaves to reach OP state
        if (EC_STATE_OPERATIONAL == ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE)) {
            sprintf(startupStatusString, "Successfully Started EtherCAT Fieldbus");
            b_allOperational = true;
            Logger::info("===== All slaves are operational");
            Logger::info("===== Successfully started EtherCAT Fieldbus");
            //addition read state is required to set the individual state of each slave
            //statecheck on slave zero doesn't assign the state of each individual slave, only global slave 0
            ec_readstate();

            slaveStateHandler = std::thread([]() { handleStateTransitions(); });
        }
        else {
            b_startupError = true;
            sprintf(startupStatusString, "Not all slaves reached Operational State");
            Logger::error("===== Not all slaves reached operational state... ");
            for (auto slave : slaves) {
                if (!slave->isStateOperational() || slave->hasStateError()) {
                    Logger::error("Slave '{}' has state {}", slave->getName(), slave->getEtherCatStateChar());
                }
            }
            stop();
        }
    }

    //============== STATE HANDLING AND SLAVE RECOVERY =================

    void handleStateTransitions() {
        Logger::debug("Started Slave State Handler Thread");
        while (b_processRunning) {

            //save each slaves previous state
            for (auto slave : slaves) slave->previousState = slave->identity->state;

            //read the current state of each slave
            ec_readstate();

            //detect state changes by comparing the previous state with the current state
            for (auto slave : slaves) {
                if (slave->identity->state != slave->previousState) {
                    if (slave->previousState == EC_STATE_OPERATIONAL && !slave->isOnline()) {
                        slave->resetData();
                        slave->pushEvent("Device Disconnected", true);
                        slave->onDisconnection();
                        Logger::error("Slave '{}' Disconnected...", slave->getName());
                    }
                    else if (slave->previousState != EC_STATE_OPERATIONAL && slave->isStateOperational()) {
                        slave->resetData();
                        slave->pushEvent("Device Reconnected (in state change compare)", false);
                        slave->onConnection();
                        Logger::info("Slave '{}' is back in Operational State", slave->getName());
                    }
                    else if (slave->previousState == EC_STATE_NONE && slave->isOnline()) {
                        Logger::info("Slave '{}' Reconnected with state {}", slave->getName(), slave->getEtherCatStateChar());
                    }
                    else if (slave->isStateOperational() && !slave->hasStateError()) {
                        Logger::info("Slave '{}' transitionned to Operational State", slave->getName());
                    }
                    else Logger::warn("Slave '{}' transitionned to {} {}", slave->getName(), slave->getEtherCatStateChar(), slave->hasStateError() ? "(State Error)" : "");

                    ;
                }
            }

            //try to recover slaves that are not online or in operational state
            for (auto slave : slaves) {
                if (!slave->isOnline()) {
                    //recover is useful to detect a slave that has a power cycle and lost its configured address
                    //recover uses incremental addressing to detect if an offline slave pops up at the same place in the network
                    //if a slave responds at that address, the function verify it matches the previous slave at that address
                    //it then reattributes an configured address to the slave
                    //the function returns 1 if the slave was successfully recovered with its previous configured address
                    if (1 == ec_recover_slave(slave->getSlaveIndex(), EC_TIMEOUTRET3)) {
                        Logger::info("Recovered slave '{}' !", slave->getName());
                    }
                }
                else if (slave->isStateSafeOperational() && !slave->hasStateError()) {
                    //set the slave back to operational after reconfiguration
                    slave->identity->state = EC_STATE_OPERATIONAL;
                    ec_writestate(slave->getSlaveIndex());
                    if (EC_STATE_OPERATIONAL == ec_statecheck(slave->getSlaveIndex(), EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE)) {
                        slave->resetData();
                        slave->pushEvent("Device Reconnected (in recover section)", false);
                        slave->onConnection();
                        Logger::info("Slave '{}' is back in Operational State!", slave->getName());
                    }
                }
                else if (!slave->isStateOperational() || slave->hasStateError()) {
                    //reconfigure looks for a slave that still has the same configured address
                    //if no slave is found at the configured address, the function does nothing
                    //this mean the function cannot directly be used to reconfigure a slave that had a power cycle and lost its configured address
                    //reconfigure takes the slave back to init and reconfigures it all the way through safeoperational
                    //we then need to set it back to operational
                    //the ec_reconfig_slave function returns the status of the slave
                    if (EC_STATE_SAFE_OP == ec_reconfig_slave(slave->getSlaveIndex(), EC_TIMEOUTRET3))
                        Logger::info("Slave '{}' Successfully Reconfigured", slave->getName());
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        Logger::debug("Exited Slave State Handler Thread");
    }







    double getCurrentCycleDeltaT_seconds() {
        return currentCycleDeltaT_seconds;
    }

}