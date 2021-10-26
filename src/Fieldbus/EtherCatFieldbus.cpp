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

    std::vector<std::shared_ptr<EtherCatDevice>> slaves;
    std::vector<std::shared_ptr<EtherCatDevice>> slaves_unassigned;
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
    double fieldbusTimeout_milliseconds = 100.0;

    bool b_detectionHandlerRunning = false;
    std::thread slaveDetectionHandler;
    std::thread etherCatRuntime;    //thread to read errors encountered by SOEM
    std::thread slaveStateHandler;  //thread to periodically check the state of all slaves and recover them if necessary
    bool b_errorWatcherRunning = false;
    std::thread errorWatcher;       //cyclic exchange thread (needs a full cpu core to axchieve precise timing)

    //====== non public functions ======

    void setup();

    //SOEM extension to read Explicit Device ID
    bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID);

    bool discoverDevices();
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

    bool init() {
        if (b_redundant) {
            bool foundPrimaryNic = false;
            bool foundRedundantNic = false;
            for (auto& nic : networkInterfaceCards) {
                if (strcmp(nic.description, networkInterfaceCard.description) == 0) {
                    networkInterfaceCard = nic;
                    foundPrimaryNic = true;
                }
                if (strcmp(nic.description, redundantNetworkInterfaceCard.description) == 0) {
                    redundantNetworkInterfaceCard = nic;
                    foundRedundantNic = true;
                }
            }
            if (foundPrimaryNic && foundRedundantNic) return init(networkInterfaceCard, redundantNetworkInterfaceCard);
            b_redundant = false;
        }
        else {
            for (auto& nic : networkInterfaceCards) {
                if (strcmp(nic.description, networkInterfaceCard.description) == 0) {
                    networkInterfaceCard = nic;
                    return init(networkInterfaceCard);
                    break;
                }
            }
        }
        if (!networkInterfaceCards.empty()) {
            Logger::warn("===== Could not find saved network interface card... Starting Fieldbus on default nic.");
            return init(networkInterfaceCards.front());
        }
        return Logger::warn("===== No Network interface cards present... Could not start EtherCAT Fieldbus");
    }

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
        if (etherCatRuntime.joinable()) etherCatRuntime.join();
        stopErrorWatcher();
        stopSlaveDetectionHandler();
        Logger::debug("===== Closing EtherCAT Network Interface Card");
        ec_close();
        b_networkOpen = false;
        Logger::info("===== Stopped EtherCAT fieldbus");
    }

    //================= SCAN REQUEST FROM GUI =================

    void scanNetwork() {
        stopSlaveDetectionHandler();
        discoverDevices();
        startSlaveDetectionHandler();
    }

    //================= START CYCLIC EXCHANGE =================

    void start() {
        if (!b_processStarting) {
            Logger::info("===== Starting Fieldbus Configuration");
            std::thread etherCatProcessStarter([]() {

                //join the cyclic echange thread if it was terminated previously
                if (etherCatRuntime.joinable()) etherCatRuntime.join();

                b_startupError = false;
                i_startupProgress = 0;
                sprintf(startupStatusString, "Starting Fieldbus Configuration");

                b_processStarting = true;
                stopSlaveDetectionHandler();

                i_startupProgress = 0;
                sprintf(startupStatusString, "Scanning Network");
                if (!discoverDevices()) {
                    b_startupError = true;
                    b_processStarting = false;
                    sprintf(startupStatusString, "Found no EtherCAT slaves on the network");
                    return;
                }

                if (!configureSlaves()) {
                    b_processStarting = false;
                    return;
                }

                startCyclicExchange();
                b_processStarting = false;
                });
            etherCatProcessStarter.detach();
        }
    }

    //================= STOP CYCLIC EXCHANGE ===================

    void stop() {
        if (b_processRunning) {
            Logger::debug("===== Stopping Cyclic Exchange...");
            b_processRunning = false;
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

    bool discoverDevices() {

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

                std::shared_ptr<EtherCatDevice> slave = nullptr;

                for (auto environnementSlave : Environnement::getEtherCatDevices()) {
                    //match the detected device name against the expected ethercat name of the environnement device
                    if (strcmp(environnementSlave->getEtherCatName(), identity.name) != 0) continue;
                    switch (environnementSlave->identificationType) {
                    case EtherCatDeviceIdentification::Type::STATION_ALIAS:
                        if (environnementSlave->stationAlias == stationAlias) {
                            slave = environnementSlave;
                            Logger::info("      Matched Environnement Slave by Name & Station Alias");
                            break;
                        }
                        else continue;
                    case EtherCatDeviceIdentification::Type::EXPLICIT_DEVICE_ID:
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
                    slave = EtherCatDeviceFactory::getDeviceByEtherCatName(identity.name);
                    slave->stationAlias = stationAlias;
                    slave->explicitDeviceID = explicitDeviceID;
                    char name[128];
                    if (explicitDeviceIdSupported && explicitDeviceID != 0) {
                        slave->identificationType = EtherCatDeviceIdentification::Type::EXPLICIT_DEVICE_ID;
                        sprintf(name, "%s (ID:%i)", slave->getSaveName(), slave->explicitDeviceID);
                    }
                    else {
                        slave->identificationType = EtherCatDeviceIdentification::Type::STATION_ALIAS;
                        sprintf(name, "%s (Alias:%i)", slave->getSaveName(), slave->stationAlias);
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

            return true;
        }

        Logger::warn("===== No EtherCAT Slaves found...");
        return false;
    }

    void removeFromUnassignedSlaves(std::shared_ptr<EtherCatDevice> removedDevice) {
        for (int i = 0; i < slaves_unassigned.size(); i++) {
            if (slaves_unassigned[i] == removedDevice) {
                slaves_unassigned.erase(slaves_unassigned.begin() + i);
                break;
            }
        }
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
                slave->getSaveName(),
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
        using namespace std::chrono;

        //thread timing variables
        uint64_t processInterval_nanoseconds = processInterval_milliseconds * 1000000.0L;
        uint64_t processDataTimeout_microseconds = processDataTimeout_milliseconds * 1000.0L;
        uint64_t systemTime_nanoseconds = Timing::getProgramTime_nanoseconds() + processInterval_nanoseconds;
        uint64_t cycleStartTime_nanoseconds = systemTime_nanoseconds + processInterval_nanoseconds;
        uint64_t previousCycleStartTime_nanoseconds = systemTime_nanoseconds;
        uint64_t fieldbusStartTime_nanoseconds = cycleStartTime_nanoseconds;
        uint64_t clockStableThreshold_nanoseconds = clockStableThreshold_milliseconds * 1000000.0;
        double averageDCTimeDelta_nanoseconds = (double)processInterval_nanoseconds / 2.0;
        int clockDriftCorrectionintegral = 0;
        std::chrono::high_resolution_clock::time_point lastProcessDataFrameReturnTime = high_resolution_clock::now();

        Logger::info("===== Waiting For clocks to stabilize before requesting Operational State...");
        i_startupProgress = ec_slavecount + 4;
        sprintf(startupStatusString, "Waiting For Clocks To Stabilize");

        //slaves are considered online when they are detected and actively exchanging data with the master
        //if we reached this state of the configuration, all slaves are detected and we are about to start exchanging data
        //we can trigger the onConnection event of all slaves
        for (auto slave : slaves) {
            slave->resetData();
            slave->pushEvent("Device Connected (Fieldbus Started)", false);
            slave->onConnection();
        }

        while (b_processRunning) {



            //======================= THREAD TIMING =========================

            //bruteforce timing precision by using 100% of CPU core
            //update and compare system time to next process 
            do { systemTime_nanoseconds = Timing::getProgramTime_nanoseconds(); } while (systemTime_nanoseconds < cycleStartTime_nanoseconds);

            //============= PROCESS DATA SENDING AND RECEIVING ==============

            ec_send_processdata();
            uint64_t frameSentTime_nanoseconds = Timing::getProgramTime_nanoseconds();
            int workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
            uint64_t frameReceivedTime_nanoseconds = Timing::getProgramTime_nanoseconds();

            //===================== TIMEOUT HANDLING ========================

            if (workingCounter <= 0) {
                if (high_resolution_clock::now() - lastProcessDataFrameReturnTime > milliseconds((int)fieldbusTimeout_milliseconds)) {
                    Logger::critical("Fieldbus timed out...");
                    //stop();
                    break; //breaks out of the main while loop
                }
                ec_DCtime += processInterval_nanoseconds; //adjust the copy of the reference clock in case no frame was received
            }
            else lastProcessDataFrameReturnTime = high_resolution_clock::now(); //reset timeout watchdog

            //===================== DEVICE IO HANDLING =======================

            if (b_allOperational) {
                //interpret all slaves input data if operational
                for (auto slave : slaves) if (slave->isStateOperational()) slave->readInputs();
                //update all nodes connected to ethercat slave nodes
                Environnement::nodeGraph.evaluate(Device::Type::ETHERCAT_DEVICE);
                //prepare all slaves output data if operational
                for (auto slave : slaves) if (slave->isStateOperational()) slave->prepareOutputs();
            }

            //=========== HANDLE MASTER AND REFERENCE CLOCK DRIFT ============

            //----- Adjust clock drift between the reference clock (ec_DCtime, time of process data receive at first slave) and the master clock ------
            //We do this by adjusting the time of next the next process cycle start.
            //This way the refresh rate of the current cyclic exchange loop is synchronized with the EtherCAT reference clock.
            //The target is to synchronize reception of the process frame with an integer multiple of the process interval.
            //For example, if the process interval is 100, the frame should be received by the reference clock slave at 0, 100, 200, 300, etc.
            //This is done by calculating the time error between the reference clock and the desired reception time (an integer multiple of the process interval)
            //We then use this dellta value in a simple control loop to produce a time offset that we will be added to the next cycle start time.
            //This effectively synchronises the cyclic exchange loop with the EtherCAT reference clock.
            int64_t referenceClockError_nanoseconds = ec_DCtime % processInterval_nanoseconds;
            if (referenceClockError_nanoseconds > processInterval_nanoseconds / 2) referenceClockError_nanoseconds -= processInterval_nanoseconds;
            averageDCTimeDelta_nanoseconds = averageDCTimeDelta_nanoseconds * 0.95 + (double)abs(referenceClockError_nanoseconds) * 0.05;
            if (referenceClockError_nanoseconds > 0) { clockDriftCorrectionintegral++; }
            if (referenceClockError_nanoseconds < 0) { clockDriftCorrectionintegral--; }
            int64_t masterClockCorrection_nanoseconds = -(referenceClockError_nanoseconds / 100) - (clockDriftCorrectionintegral / 20);
            previousCycleStartTime_nanoseconds = cycleStartTime_nanoseconds;
            cycleStartTime_nanoseconds += processInterval_nanoseconds + masterClockCorrection_nanoseconds;

            //======================== SYSTEM CLOCK ==========================

            //update a time value that will serve as a synchronisation reference for all ethercat devices
            //the time value is synchronous to the master system clock and is updated on each ethercat process cycle
            //we start by initializing the time value to the first cycle start time
            //we increment this time by the process interval time on each cycle
            //to avoid drift from the actual system clock, we calculate the error between the incremented time and the actual system time.
            //to avoid jitter we filter this error before applying it as a correction to the time value using a proportional gain weight.
            //this way we get a time reference that is synchronous with the system clock and
            //most importantly without high frequency jitter induced by corrections to keep the process cycle synchronized with the EtherCAT reference clock

            static int64_t systemTimeErrorSmoothed_nanoseconds = 0;
            static double systemTimeErrorFilter = 0.99;
            static int64_t systemTimeSmoothed_nanoseconds = cycleStartTime_nanoseconds;
            static double smoothedTimeCorrection_proportionalGain = 0.1;

            int64_t systemTimeError = systemTime_nanoseconds - systemTimeSmoothed_nanoseconds;
            systemTimeErrorSmoothed_nanoseconds = systemTimeErrorSmoothed_nanoseconds * systemTimeErrorFilter + (1.0 - systemTimeErrorFilter) * systemTimeError;
            currentCycleProgramTime_nanoseconds = systemTimeSmoothed_nanoseconds;
            currentCycleProgramTime_seconds = (double)systemTimeSmoothed_nanoseconds / 1000000000.0;
            uint64_t fieldbusTimeSmoothed_nanoseconds = systemTimeSmoothed_nanoseconds - fieldbusStartTime_nanoseconds;
            double fieldbusTimeSmoothed_seconds = (double)fieldbusTimeSmoothed_nanoseconds / 1000000000.0;
            systemTimeSmoothed_nanoseconds += processInterval_nanoseconds + systemTimeErrorSmoothed_nanoseconds * smoothedTimeCorrection_proportionalGain;

            //================= OPERATIONAL STATE TRANSITION ==================

            if (!b_clockStable && averageDCTimeDelta_nanoseconds < clockStableThreshold_nanoseconds) {
                b_clockStable = true;
                sprintf(startupStatusString, "Setting All Slaves to Operational State");
                std::thread opStateHandler([]() { transitionToOperationalState(); });
                opStateHandler.detach();
            }

            //==================== UPDATE FIELDBUS METRICS =====================

            metrics.fieldbusTime_seconds = fieldbusTimeSmoothed_seconds;
            metrics.averageDcTimeError_milliseconds = averageDCTimeDelta_nanoseconds / 1000000.0;       //used to display clock drift correction progress
            double frameSendDelay_milliseconds = (double)(frameSentTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
            double frameReceiveDelay_milliseconds = (double)(frameReceivedTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
            double timeoutDelay_milliseconds = frameSendDelay_milliseconds + processDataTimeout_milliseconds;
            double cycleLength_milliseconds = (double)(cycleStartTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
            metrics.dcTimeErrors.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, referenceClockError_nanoseconds / 1000000.0));
            metrics.averageDcTimeErrors.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, averageDCTimeDelta_nanoseconds / 1000000.0));
            metrics.sendDelays.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, frameSendDelay_milliseconds));
            metrics.receiveDelays.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, frameReceiveDelay_milliseconds));
            metrics.timeoutDelays.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, timeoutDelay_milliseconds));
            metrics.cycleLengths.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, cycleLength_milliseconds));;
            metrics.addWorkingCounter(workingCounter, fieldbusTimeSmoothed_seconds);
            if (workingCounter <= 0) metrics.timeouts.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, frameReceiveDelay_milliseconds));
            uint64_t processedTime_nanoseconds = Timing::getProgramTime_nanoseconds();
            double processDelay_milliseconds = (double)(processedTime_nanoseconds - previousCycleStartTime_nanoseconds) / 1000000.0L;
            metrics.processDelays.addPoint(glm::vec2(fieldbusTimeSmoothed_seconds, processDelay_milliseconds));
            metrics.cycleCounter++;

            //======================== RUNTIME LOOP END =========================
        }
        b_processRunning = false;
        b_allOperational = false;

        //send one last frame to all slaves to disable them
        //this way motors don't suddenly jerk to a stop when stopping the fieldbus in the middle of a movement
        for (auto slave : slaves) {
            slave->disable();
            slave->prepareOutputs();
        }
        ec_send_processdata();

        //terminate and disable all slaves
        for (auto slave : slaves) {
            if (slave->isDetected()) {
                slave->resetData();
                slave->pushEvent("Device Disconnected (Fieldbus Shutdown)", false);
                slave->onDisconnection();
            }
            slave->identity->state = EC_STATE_NONE;
        }
        //evaluate all nodes one last time to propagate the disconnection of devices
        Environnement::nodeGraph.evaluate(Device::Type::ETHERCAT_DEVICE);

        Logger::info("===== Cyclic Exchange Stopped !");

        //cleanup threads and relaunc slave detection handler
        if (slaveStateHandler.joinable()) slaveStateHandler.join();
        startSlaveDetectionHandler();
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
                    if (slave->isStateNone()) {
                        slave->resetData();
                        slave->pushEvent("Device Disconnected", true);
                        slave->onDisconnection();
                        Logger::error("Slave '{}' Disconnected...", slave->getName());
                    }
                    else if (slave->previousState == EC_STATE_NONE) {
                        slave->resetData();
                        char eventString[64];
                        sprintf(eventString, "Device Reconnected with state %s", slave->getEtherCatStateChar());
                        slave->pushEvent(eventString, false);
                        slave->onConnection();
                        Logger::info("Slave '{}' reconnected with state {}", slave->getName(), slave->getEtherCatStateChar());
                    }
                    else {
                        char eventString[64];
                        sprintf(eventString, "EtherCAT state changed to %s", slave->getEtherCatStateChar());
                        slave->pushEvent(eventString, false);
                        Logger::info("Slave '{}' state changed to {}", slave->getName(), slave->getEtherCatStateChar());
                    }
                }
            }

            //try to recover slaves that are not online or in operational state
            for (auto slave : slaves) {
                if (slave->isStateNone()) {
                    //recover is useful to detect a slave that has a power cycle and lost its configured address
                    //recover uses incremental addressing to detect if an offline slave pops up at the same place in the network
                    //if a slave responds at that address, the function verify it matches the previous slave at that address
                    //it then reattributes an configured address to the slave
                    //the function returns 1 if the slave was successfully recovered with its previous configured address
                    if (1 == ec_recover_slave(slave->getSlaveIndex(), EC_TIMEOUTRET3)) {
                        slave->resetData();
                        slave->pushEvent("Device Reconnected after power cycle", false);
                        slave->onConnection();
                        Logger::info("Recovered slave '{}' !", slave->getName());
                    }
                }
                else if (slave->isStateSafeOperational() && !slave->hasStateError()) {
                    //set the slave back to operational after reconfiguration
                    slave->identity->state = EC_STATE_OPERATIONAL;
                    ec_writestate(slave->getSlaveIndex());
                    if (EC_STATE_OPERATIONAL == ec_statecheck(slave->getSlaveIndex(), EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE)) {
                        slave->pushEvent("Device back in operational state", false);
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
                    if (EC_STATE_SAFE_OP == ec_reconfig_slave(slave->getSlaveIndex(), EC_TIMEOUTRET3)) {
                        slave->pushEvent("Device Reconfigured", false);
                        Logger::info("Slave '{}' Successfully Reconfigured", slave->getName());
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        Logger::debug("Exited Slave State Handler Thread");
    }

    double currentCycleProgramTime_seconds = 0.0;
    double getCycleProgramTime_seconds() {
        return currentCycleProgramTime_seconds;
    }
    
    long long int currentCycleProgramTime_nanoseconds = 0;
    long long int getCycleProgramTime_nanoseconds() {
        return currentCycleProgramTime_nanoseconds;
    }
}


//TODO: ALStatusCode reading and displaying on device error