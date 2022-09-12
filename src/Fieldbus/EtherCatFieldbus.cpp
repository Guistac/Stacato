#include <pch.h>

#include "EtherCatFieldbus.h"

#include "Fieldbus/EtherCatDevice.h"
#include "Utilities/EtherCatError.h"
#include "Environnement/Environnement.h"
#include "Environnement/NodeGraph/NodeGraph.h"
#include "Animation/Playback/Playback.h"
#include "Utilities/ProgressIndicator.h"
#include "Nodes/NodeFactory.h"
#include "config.h"

#define MAX_IO_MAP_SIZE 4096

namespace EtherCatFieldbus {

	//EXPERIMENTAL SETTINGS
	bool b_usePosixRealtimeThread = true;
	pthread_t rtThread;
	int stackSize = 65536;




    std::vector<std::shared_ptr<NetworkInterfaceCard>> networkInterfaceCards;
	std::vector<std::shared_ptr<NetworkInterfaceCard>>& getNetworksInterfaceCards(){ return networkInterfaceCards; }

	std::shared_ptr<NetworkInterfaceCard> primaryNetworkInterfaceCard;
	std::shared_ptr<NetworkInterfaceCard> redundantNetworkInterfaceCard;
	std::shared_ptr<NetworkInterfaceCard> getActiveNetworkInterfaceCard(){ return primaryNetworkInterfaceCard; }
	std::shared_ptr<NetworkInterfaceCard> getActiveRedundantNetworkInterfaceCard(){ return redundantNetworkInterfaceCard; }

	char defaultPrimaryNetworkInterfaceDescription[512];
	char defaultSecondaryNetworkInterfaceDescription[512];
	
	void setDefaultNetworkInterfaces(const char* primary, const char* secondary){
		if(primary != 0) strcpy(defaultPrimaryNetworkInterfaceDescription, primary);
		if(secondary != 0) strcpy(defaultSecondaryNetworkInterfaceDescription, secondary);
	}

    std::vector<std::shared_ptr<EtherCatDevice>> slaves;			//slaves discovered on the network
    std::vector<std::shared_ptr<EtherCatDevice>> slaves_unassigned; //slaves discovered on the network but not added to the environnement editor
	std::vector<std::shared_ptr<EtherCatDevice>>& getDevices(){ return slaves; }
	std::vector<std::shared_ptr<EtherCatDevice>>& getUnassignedDevices(){ return slaves_unassigned; }

	//contiguous storage for cyclic exchange data
	
    uint8_t ioMap[MAX_IO_MAP_SIZE];
    int ioMapSize = 0;


	bool b_detectionThreadRunning = false;
	std::thread slaveDetectionThread;

	bool b_cyclicExchangeRunning = false;
	std::thread cyclicExchangeThread;	//cyclic exchange thread (needs a full cpu core to avoid scheduler issues and achieve realtime performance)
	std::thread slaveStateHandler; 		//helper thread to periodically check the state of all slaves and recover them if necessary
	std::thread slaveErrorCounter;		//helper thread to periodically read all slaves error counters

	bool b_errorWatcherRunning = false;
	std::thread errorWatcherThread;       //thread to read errors encountered by SOEM

	int cyclicFrameTimeoutCounter = 0;
	int cyclicFrameErrorCounter = 0;

    bool b_networkOpen = false; //Network is initialized on one or two networks interface cards
	bool b_starting = false;	//while cyclic exchange is starting
	bool b_running = false;		//network is running

	bool hasNetworkInterface() { return b_networkOpen; }
	bool hasRedundantInterface() { return b_networkOpen && primaryNetworkInterfaceCard && redundantNetworkInterfaceCard; }
	bool hasDetectedDevices(){ return !slaves.empty(); }
	bool isStarting(){ return b_starting; }
	bool isRunning(){ return b_running; }
	bool canScan(){ return b_networkOpen && !b_cyclicExchangeRunning; }
	bool canStart(){ return true; } //should indicate if network permissions are set !
	bool canStop(){ return !b_starting || (b_starting && b_cyclicExchangeRunning); }

	ProgressIndicator startupProgress;
    EtherCatMetrics metrics;
	EtherCatMetrics& getMetrics(){ return metrics; }

    double processInterval_milliseconds = 3.0;
    double processDataTimeout_milliseconds = 1.5;
    double clockStableThreshold_milliseconds = 0.1;
    double fieldbusTimeout_milliseconds = 100.0;

	double currentCycleProgramTime_seconds = 0.0;
	long long int currentCycleProgramTime_nanoseconds = 0;
	double currentCycleDeltaT_seconds = 0.0;
	long long int currentCycleDeltaT_nanoseconds = 0;
	double getCycleProgramTime_seconds() { return currentCycleProgramTime_seconds; }
	long long int getCycleProgramTime_nanoseconds() { return currentCycleProgramTime_nanoseconds; }
	double getCycleTimeDelta_seconds() { return currentCycleDeltaT_seconds; }
	long long int getCycleTimeDelta_nanoseconds() { return currentCycleDeltaT_nanoseconds; }

    //====== non public functions ======

    void setup();

    //SOEM extension to read Explicit Device ID
    bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID);

	bool discoverDevices(bool logStartup);
    bool configureSlaves();
    void startCyclicExchange();
	void updateNetworkTopology();
    void cyclicExchange();
    void transitionToOperationalState();
    void handleStateTransitions();
	void countSlaveErrors();
	void updateErrorCounters();

    void startSlaveDetectionHandler();
    void stopSlaveDetectionHandler();
    void startErrorWatcher();
    void stopErrorWatcher();

    bool configureSlaves();
    void startCyclicExchange();

	void logDeviceStates();
	void logAlStatusCodes();

	//============= CHECK NETWORK PERMISSIONS ==============

	//this checks if low lewel network packet manipulation permissions are granted to libpcap by the operating system
	//on windows machine its unclear how these permissions are enabled (by some module installed by wireshark?)
	//on macos permissions are disabled by default and can be enabled by running the "Install ChmodBPF.pkg" Installer provided with the repository in the utilities folder
	//trying to execute pcap or ethercat functions without these permissions will result in a bad memory access and hard crash

	bool hasNetworkPermissions(){
		//this will only report a correct status after a network interface card was opened
		ecx_contextt* context = &ecx_context;
		ecx_portt* port = context->port;
		ec_stackT* stack = &port->stack;
		pcap_t** socket_ptr = stack->sock;
		pcap_t* socket = *socket_ptr;
		return socket != nullptr;
	}

    //============ LIST NETWORK INTERFACE CARDS ===============

    void updateNetworkInterfaceCardList() {
		if(isRunning()) return;
        Logger::info("===== Refreshing Network Interface Card List");
		
		std::vector<std::shared_ptr<NetworkInterfaceCard>> oldNics = networkInterfaceCards;
        networkInterfaceCards.clear();
		
        ec_adaptert* nics = ec_find_adapters();
        if (nics != nullptr) {
            while (nics != nullptr) {
				
#ifdef STACATO_MACOS
				//on macos, we can skip network interfaces that don't have a name with the format en
				if (strstr(nics->name, "en") == nullptr) {
					nics = nics->next;
					continue;
				}
#endif
				
				bool wasPresent = false;
				for(auto& oldNic : oldNics){
					if(strcmp(oldNic->name, nics->name) == 0 && strcmp(oldNic->description, nics->desc) == 0){
						networkInterfaceCards.push_back(oldNic);
						wasPresent = true;
						break;
					}
				}
				
				if(!wasPresent){
					std::shared_ptr<NetworkInterfaceCard> newNic = std::make_shared<NetworkInterfaceCard>();
					strcpy(newNic->name, nics->name);
					strcpy(newNic->description, nics->desc);
					networkInterfaceCards.push_back(newNic);
				}
				
                nics = nics->next;
            }
        }
        Logger::info("===== Found {} Network Interface Card{}", networkInterfaceCards.size(), networkInterfaceCards.size() == 1 ? "" : "s");
        for (auto& nic : networkInterfaceCards) Logger::debug("    = {} (ID: {})", nic->description, nic->name);
    }

    //============== INTIALIZE FIELDBUS WITH AND OPEN NETWORK INTERFACE CARD ==============

    bool init() {
		
		std::shared_ptr<NetworkInterfaceCard> primary = nullptr;
		std::shared_ptr<NetworkInterfaceCard> secondary = nullptr;
		
		if(defaultPrimaryNetworkInterfaceDescription){
			for(auto nic : getNetworksInterfaceCards()){
				if(strcmp(defaultPrimaryNetworkInterfaceDescription, nic->description) == 0){
					primary = nic;
					break;
				}
			}
			if(primary) Logger::info("Found default primary EtherCAT network interface");
			else Logger::info("Could not find default primary EtherCAT network interface");

			if(primary && defaultSecondaryNetworkInterfaceDescription){
				for(auto nic : getNetworksInterfaceCards()){
					if(strcmp(defaultSecondaryNetworkInterfaceDescription, nic->description) == 0){
						secondary = nic;
						break;
					}
				}
				if(secondary) Logger::info("Found default secondary EtherCAT network interface");
				else Logger::info("Could not find default secondary EtherCAT network interface");
			}
		}else Logger::warn("No Default EtherCAT network interface");
		
		if(primary && secondary) return init(primary, secondary);
		else if(primary) return init(primary);
		else if(!getNetworksInterfaceCards().empty()) return init(getNetworksInterfaceCards().front());
		else return false;
    }


    bool init(std::shared_ptr<NetworkInterfaceCard> nic) {
        if (b_networkOpen) terminate();
        primaryNetworkInterfaceCard = nic;
		redundantNetworkInterfaceCard = nullptr;
        Logger::debug("===== Initializing EtherCAT Fieldbus on Network Interface Card '{}'", primaryNetworkInterfaceCard->description);
        if (ec_init(primaryNetworkInterfaceCard->name) < 0) {
            Logger::error("===== Failed to initialize network interface card ...");
            b_networkOpen = false;
            return false;
        }
        b_networkOpen = true;
        Logger::info("===== Initialized network interface card '{}'", primaryNetworkInterfaceCard->description);
        setup();
        return true;
    }

    bool init(std::shared_ptr<NetworkInterfaceCard> nic, std::shared_ptr<NetworkInterfaceCard> redNic) {
        if (b_networkOpen) terminate();
		primaryNetworkInterfaceCard = nic;
        redundantNetworkInterfaceCard = redNic;
        Logger::debug("===== Initializing EtherCAT Fieldbus on Network Interface Card '{}' with redundancy on '{}'",
					  primaryNetworkInterfaceCard->description,
					  redundantNetworkInterfaceCard->description);
        if (ec_init_redundant(primaryNetworkInterfaceCard->name, redundantNetworkInterfaceCard->name) < 0) {
            Logger::error("===== Failed to initialize network interface cards ...");
            b_networkOpen = false;
            return false;
        }
        b_networkOpen = true;
        Logger::info("===== Initialized network interface cards '{}' & '{}'",
					 primaryNetworkInterfaceCard->description,
					 redundantNetworkInterfaceCard->description);
        setup();
        return true;
    }

	bool b_autoInit = false;

	void autoInit(){
		if(isAutoInitRunning()) return;
		std::thread autoInitHandler([](){
			pthread_setname_np("EtherCAT Auto Init Handler Thread");
			b_autoInit = true;
			updateNetworkInterfaceCardList();
			for(auto& nic : networkInterfaceCards){
				if(!init(nic)) continue;
				if(slaves.empty()) terminate();
				else break;
			}
			b_autoInit = false;
		});
		autoInitHandler.detach();
	}

	bool isAutoInitRunning(){
		return b_autoInit;
	}

    void setup() {
        startErrorWatcher();
        scanNetwork();
        metrics.init(processInterval_milliseconds);
    }

    //============= STOP FIELDBUS AND CLOSE NETWORK INTERFACE CARD ==============

    void terminate() {
        stop();
		if(b_networkOpen){
			stopErrorWatcher();
			stopSlaveDetectionHandler();
			Logger::debug("===== Closing EtherCAT Network Interface Card");
			ec_close();
			b_networkOpen = false;
			Logger::info("===== Stopped EtherCAT fieldbus");
		}
		primaryNetworkInterfaceCard = nullptr;
		redundantNetworkInterfaceCard = nullptr;
    }

    //================= SCAN REQUEST FROM GUI =================

    void scanNetwork() {
        stopSlaveDetectionHandler();
		
		if(!hasNetworkPermissions()){
			Logger::critical("Could not start EtherCAT Fieldbus because low level network packet manipulation is not allowed on the system.");
			Logger::critical("Please install ChmodBPF.pkg and restart Stacato.");
			return;
		}
		
        discoverDevices(false);
        startSlaveDetectionHandler();
    }

    //================= START CYCLIC EXCHANGE =================

    void start() {
        if (!b_starting) {
			b_starting = true;

			startupProgress.setStart("Starting Fieldbus Configuration");
			Logger::info("===== Starting Fieldbus Configuration");
						
			std::thread etherCatProcessStarter([]() {
				pthread_setname_np("EtherCAT Process Starter Thread");
								
                stopSlaveDetectionHandler();
							
				if(!hasDetectedDevices() || !hasNetworkInterface()) {
					updateNetworkInterfaceCardList();
					startupProgress.setProgress(0.01, "Scanning Network Interfaces");
					for(auto& nic : networkInterfaceCards){
						std::string nicScanString = "Scanning Network Interface " + std::string(nic->description);
						startupProgress.setProgress(0.02, nicScanString.c_str());
						if(!init(nic)) continue;
						if(slaves.empty()) terminate();
						else break;
					}
					if(!hasNetworkInterface()){
						b_starting = false;
						startupProgress.setFailure("No EtherCAT devices found on any network interface.");
						return;
					}
				}
				
                if (!discoverDevices(true)) {
					b_starting = false;
					logDeviceStates();
					logAlStatusCodes();
                    return;
                }

                if (!configureSlaves()) {
					b_starting = false;
					logDeviceStates();
					logAlStatusCodes();
                    return;
                }
				
				updateNetworkTopology();

                startCyclicExchange();
			});
            etherCatProcessStarter.detach();
        }
    }

    //================= STOP CYCLIC EXCHANGE ===================

    void stop() {
        if (b_cyclicExchangeRunning) {
            Logger::debug("===== Stopping Cyclic Exchange...");
			b_cyclicExchangeRunning = false;
			b_starting = false;
			if(cyclicExchangeThread.joinable()) cyclicExchangeThread.join();
        }
    }


    //========== General Error Watcher Utility ============

    void startErrorWatcher() {
        b_errorWatcherRunning = true;
		errorWatcherThread = std::thread([]() {
			pthread_setname_np("EtherCAT Error Watcher Thread");
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
        if (errorWatcherThread.joinable()) errorWatcherThread.join();
    }

    //============ Check Explicit ID compatibility and Read explicit ID of slave ============

    bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID) {
		
        int maxTries = 8;
		bool success = false;
		int tries = 0;
		bool foundID = false;
		
		//read AL Status (state machine, error, and ID status)
		uint16_t ALstatus;
		while(tries < maxTries && !success){
			int wc = ec_FPRD(configAddress, 0x130, 2, &ALstatus, EC_TIMEOUTSAFE);
			if(wc == 1) success = true;
			tries++;
		}
		
		success = false;
		tries = 0;
		
		//write AL Control to request device identification (bit 5 high)
		uint16_t ALcontrol = ALstatus |= 0x20;
		while(tries < maxTries && !success){
			int wc = ec_FPWR(configAddress, 0x120, 2, &ALcontrol, EC_TIMEOUTSAFE);
			if(wc == 1) success = true;
			tries++;
		}
		
		success = false;
		tries = 0;
		
		//read AL status to see if device id is active (bit 5)
		//this takes a considerable amount of time, a single read might be too soon, so we read multiple time
		while(tries < maxTries && !success){
			int wc = ec_FPRD(configAddress, 0x130, 2, &ALstatus, EC_TIMEOUTSAFE);
			if(wc == 1 && ALstatus & 0x20) success = true;
			tries++;
		}
		
		if (success) {
			
			success = false;
			tries = 0;
			
			//if it is supported read AL Status code (will contain explicit device ID)
			uint16_t ALstatusCode;
			
			while(tries < maxTries && !success){
				int wc = ec_FPRD(configAddress, 0x134, 2, &ALstatusCode, EC_TIMEOUTSAFE);
				if(wc == 1) success = true;
				tries++;
			}
			
			if(success) {
				ID = ALstatusCode;
				foundID = true;
			}else{
				foundID = false;
			}
			
		}else{
			foundID = false;
		}
		
		
		success = false;
		tries = 0;
		
		//write to AL Control to disable ID Display
		ALcontrol &= ~0x20;
		while(tries < maxTries && !success){
			int wc = ec_FPWR(configAddress, 0x120, 2, &ALcontrol, EC_TIMEOUTSAFE);
			if(wc == 1) success = true;
			tries++;
		}
		
		if(foundID) return true;
		
		ID = 0;
        return false;
    }


    //=================== DISCOVER ETHERCAT DEVICES ON THE NETWORK =====================

    bool discoverDevices(bool logStartup) {

		if(logStartup) startupProgress.setProgress(0.01, "Scanning Network");
		
        //when rescanning the network, all previous slaves are now considered to be offline before being detected again
        //for a slave to appear as offline, we set its identity object (ec_slavet) to nullptr
        for (auto slave : slaves) slave->identity = nullptr;
        //we clear the list of slaves, slaves that are in the node graph remain there
        slaves.clear();
        //we also clear the list of slaves that were not in the nodegraph
        slaves_unassigned.clear();

        //setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
		//BUG: we need to do this twice: when swapping etherCAT connections, sometimes the aliasAdress doesn't read on the first config call (probably SOEM bug)
        int workingCounter = ec_config_init(false);
		workingCounter = ec_config_init(false);

        if (workingCounter > 0) {

            //wait and check if all slaves have reached Pre Operational State like requested by ec_config_init()
            if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE) != EC_STATE_PRE_OP) {
				if(logStartup) startupProgress.setFailure("Not all slaves reached Pre-Operational State. Check the Log for more detailed errors.");
                Logger::error("Not All Slaves Reached Pre-Operational State...");
				return false;
            }

			if(logStartup) startupProgress.setProgress(0.05, "Identifying Devices");
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
                    //match the detected device name against the manufacturer and identification codes
					if(environnementSlave->getManufacturerCode() != identity.eep_man || environnementSlave->getIdentificationCode() != identity.eep_id) continue;
                    switch (environnementSlave->identificationType) {
						case EtherCatDevice::IdentificationType::STATION_ALIAS:
							if (environnementSlave->stationAlias == stationAlias) {
								slave = environnementSlave;
								Logger::info("      Matched Environnement Slave by Name & Station Alias");
							}
							break;
						case EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID:
							if (explicitDeviceIdSupported && environnementSlave->explicitDeviceID == explicitDeviceID) {
								slave = environnementSlave;
								Logger::info("      Matched Environnement Slave by Name & Explicit Device ID");
							}
							break;
                    }
					if(slave != nullptr) break;
                }

                if (slave == nullptr) {
                    Logger::info("      Slave did not match any Environnement Slave");
					slave = NodeFactory::getEtherCatDeviceByIdCodes(identity.eep_man, identity.eep_id);
                    slave->stationAlias = stationAlias;
                    slave->explicitDeviceID = explicitDeviceID;
                    char name[128];
                    if (explicitDeviceIdSupported && explicitDeviceID != 0) {
                        slave->identificationType = EtherCatDevice::IdentificationType::EXPLICIT_DEVICE_ID;
                        sprintf(name, "%s (ID:%i)", slave->getName(), slave->explicitDeviceID);
                    }
                    else {
                        slave->identificationType = EtherCatDevice::IdentificationType::STATION_ALIAS;
                        sprintf(name, "%s (Alias:%i)", slave->getName(), slave->stationAlias);
                    }
                    slave->setName(name);
                    slaves_unassigned.push_back(slave);
                }

                slave->identity = &identity;
                slave->slaveIndex = i;

                if (!slave->isEtherCatDeviceKnown()) {
                    Logger::warn("Found Unknown Slave: {}", identity.name);
                }

                //add the slave to the list of slaves regardless of environnement presence
                slaves.push_back(slave);
            }

            return true;
        }

		if(logStartup) startupProgress.setFailure("No EtherCAT slaves found");
        Logger::warn("===== No EtherCAT Slaves found...");
        return false;
    }

    void removeUnassignedDevice(std::shared_ptr<EtherCatDevice> removedDevice) {
        for (int i = 0; i < slaves_unassigned.size(); i++) {
            if (slaves_unassigned[i] == removedDevice) {
                slaves_unassigned.erase(slaves_unassigned.begin() + i);
                break;
            }
        }
    }

    //================ Slave Pinging Utility during no cyclic exchange ====================

    void startSlaveDetectionHandler() {
        if (b_detectionThreadRunning) return;
		b_detectionThreadRunning = true;
		slaveDetectionThread = std::thread([]() {
			pthread_setname_np("EtherCAT Detection Handler Thread");
            Logger::debug("Started EtherCAT Detection Handler");
            while (b_detectionThreadRunning) {
                ec_readstate();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            Logger::debug("Exited EtherCAT Detection Handler");
		});
    }

    void stopSlaveDetectionHandler() {
		b_detectionThreadRunning = false;
        if (slaveDetectionThread.joinable()) slaveDetectionThread.join();
    }

    //=========== Map Slave memory, Configure Distributed Clocks, Do Per slave configuration, Transition to Safe Operational ===============

    bool configureSlaves() {

		startupProgress.setProgress(0.075, "Configuring Distributed Clocks");
        Logger::debug("===== Configuring Distributed Clocks");
		
        if (!ec_configdc()) {
			startupProgress.setFailure("Could not configure distributed clocks. Check the Log for more detailed errors.");
            Logger::error("===== Could not configure distributed clocks ...");
            return false;
        }

        //assign slave startup hooks
        for (int i = 1; i <= ec_slavecount; i++) {
            //we don't use the PO2SOconfigx hook since it isn't supported by ec_reconfig_slave()
            ec_slave[i].PO2SOconfig = [](uint16_t slaveIndex) -> int {
                for (auto slave : slaves) {
                    if (slave->getSlaveIndex() == slaveIndex) {
						
						int slaveCount = slaves.size();
						float confProgStart = 0.1;
						float confProgEnd = 0.5;
						float confIncrement = (confProgEnd - confProgStart) / slaveCount;
						float configurationProgress = slave->getSlaveIndex() * confIncrement + confProgStart;
						
						static char slaveConfigurationString[256];
						sprintf(slaveConfigurationString, "Configuring %s", slave->getName());
						startupProgress.setProgress(configurationProgress, slaveConfigurationString);
						
						Logger::debug("Configuring Slave '{}'", slave->getName());
                        bool configurationResult = slave->startupConfiguration();
                        if (configurationResult) {
                            Logger::debug("Successfully configured Slave '{}'", slave->getName());
                            return 1;
                        }
                        else {
                            Logger::warn("Failed to configure slave '{}'", slave->getName());
                            return 0;
                        }
                    }
                }
                return 0;
            };
        }

        //build ioMap for PDO data, configure FMMU and SyncManager, request SAFE-OP state for all slaves
		startupProgress.setProgress(0.01, "Configuring Devices");
        Logger::debug("===== Beginning Device Configuration and IOMap Building...");
        ioMapSize = ec_config_map(ioMap); //this function starts the configuration
        if (ioMapSize <= 0) {
			startupProgress.setFailure("Failed to configure devices");
            Logger::error("===== Failed To Configure Devices. Check the Log for more detailed errors.", ioMapSize);
            return false;
		}else if(ioMapSize >= MAX_IO_MAP_SIZE){
			Logger::critical("IoMap size is exceeded !");
		}
        Logger::info("===== Finished Configuring Devices  (IOMap size : {} bytes)", ioMapSize);

		
        for (auto slave : slaves) {
            Logger::debug("   [{}] '{}' {} bytes ({} bits)",
                slave->getSlaveIndex(),
                slave->getSaveName(),
                slave->identity->Ibytes + slave->identity->Obytes,
                slave->identity->Ibits + slave->identity->Obits);
            Logger::debug("          Inputs: {} bytes ({} bits)", slave->identity->Ibytes, slave->identity->Ibits);
            Logger::debug("          Outputs: {} bytes ({} bits)", slave->identity->Obytes, slave->identity->Obits);
        }

		
		startupProgress.setProgress(0.55, "Waiting for Safe-Operational State");
        Logger::debug("===== Checking For Safe-Operational State...");
        if (ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE) != EC_STATE_SAFE_OP) {
			startupProgress.setFailure("Not all slaves reached Safe-Operational State. Check the Log for more detailed errors.");
            Logger::error("===== Not all slaves have reached Safe-Operational State...");
            return false;
        }
        Logger::info("===== All slaves are Safe-Operational");

        return true;
    }

    //========= START CYCLIC EXCHANGE ============

	//EXPERIMENTAL
	void pthreadCyclicExchange(void* data){
		pthread_setname_np("EtherCAT Cyclic Exchange Thread (osal rtThread)");
		cyclicExchange();
	}

    void startCyclicExchange() {
        //don't allow the thread to start if it is already running
        if (b_cyclicExchangeRunning) return;

		startupProgress.setProgress(0.6, "Starting Cyclic Exchange");
        Logger::debug("===== Starting Cyclic Process Data Exchange");

        metrics.init(processInterval_milliseconds);
		
		if(b_usePosixRealtimeThread){
			
			//EXPERIMENTAL
			b_cyclicExchangeRunning = true;
			osal_thread_create_rt(&rtThread, stackSize, (void*)&pthreadCyclicExchange, nullptr);
			pthread_detach(rtThread);
			
		}else{
		
			//join the cyclic echange thread if it was terminated previously
			if (cyclicExchangeThread.joinable()) cyclicExchangeThread.join();
			
			b_cyclicExchangeRunning = true;
			cyclicExchangeThread = std::thread([]() {
				pthread_setname_np("EtherCAT Cyclic Exchange Thread (std::thread)");
				cyclicExchange();
			});
		}
    }


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
		startupProgress.setProgress(0.65, "Waiting for clocks to stabilize");

        //slaves are considered online when they are detected and actively exchanging data with the master
        //if we reached this state of the configuration, all slaves are detected and we are about to start exchanging data
        //we can trigger the onConnection event of all slaves
        for (auto slave : slaves) {
            slave->pushEvent("Device Connected (Fieldbus Started)", false);
            slave->onConnection();
        }

        int64_t systemTimeErrorSmoothed_nanoseconds = 0;
        int64_t systemTimeSmoothed_nanoseconds = cycleStartTime_nanoseconds;
		bool b_clockStable = false;

        while (b_cyclicExchangeRunning) {

            //======================= THREAD TIMING =========================

            //bruteforce timing precision by using 100% of CPU core
            //update and compare system time to next process 
            do { systemTime_nanoseconds = Timing::getProgramTime_nanoseconds(); } while (systemTime_nanoseconds < cycleStartTime_nanoseconds);
			
            //sleep timing instead of bruteforce timing
            /*
            systemTime_nanoseconds = Timing::getProgramTime_nanoseconds();
            long long sleepTime_nanoseconds = cycleStartTime_nanoseconds - systemTime_nanoseconds;
            uint32_t sleepTime_microseconds = sleepTime_nanoseconds / 1000;
            if(sleepTime_microseconds > 0) osal_usleep(sleepTime_microseconds);
            systemTime_nanoseconds = Timing::getProgramTime_nanoseconds();
             */
		
            //============= PROCESS DATA SENDING AND RECEIVING ==============

            ec_send_processdata();
            uint64_t frameSentTime_nanoseconds = Timing::getProgramTime_nanoseconds();
            int workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
            uint64_t frameReceivedTime_nanoseconds = Timing::getProgramTime_nanoseconds();
			
			switch(workingCounter){
				case EC_NOFRAME:
				case EC_TIMEOUT:
					cyclicFrameTimeoutCounter++;
					break;
				case EC_OTHERFRAME:
				case EC_ERROR:
				case EC_SLAVECOUNTEXCEEDED:
					cyclicFrameErrorCounter++;
					break;
			}

            //===================== TIMEOUT HANDLING ========================

            if (workingCounter <= 0) {
                if (high_resolution_clock::now() - lastProcessDataFrameReturnTime > milliseconds((int)fieldbusTimeout_milliseconds)) {
                    Logger::critical("Fieldbus timed out...");
                    break; //breaks out of the main while loop (which stops the fieldbus?)
                }
				//adjust the copy of the reference clock in case no frame was received
                ec_DCtime += processInterval_nanoseconds;
            }
            else lastProcessDataFrameReturnTime = high_resolution_clock::now(); //reset timeout watchdog

            //===================== ENVIRONNEMENT UPDATE =======================

			if (b_running) Environnement::updateEtherCatHardware();

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
            else if (referenceClockError_nanoseconds < 0) { clockDriftCorrectionintegral--; }
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
            //most importantly without high frequency jitter induced by corrections that keep the process cycle synchronized with the EtherCAT reference clock

            static double systemTimeErrorFilter = 0.99; //smoothing of the error to get a get a stable value
            static double smoothedTimeCorrection_proportionalGain = 0.002; //strength of the correction applied to the time value (tested and working for cycle times between 1ms and 20 ms)
            
			//calculate the current time error
			int64_t systemTimeError = systemTime_nanoseconds - systemTimeSmoothed_nanoseconds;
			//smoothe the time error
            systemTimeErrorSmoothed_nanoseconds = systemTimeErrorSmoothed_nanoseconds * systemTimeErrorFilter + (1.0 - systemTimeErrorFilter) * systemTimeError;
			//apply a weighted error correction to the system time
			systemTimeSmoothed_nanoseconds += processInterval_nanoseconds + systemTimeErrorSmoothed_nanoseconds * smoothedTimeCorrection_proportionalGain;
			
			//time values in reference to fieldbus start
			uint64_t fieldbusTimeSmoothed_nanoseconds = systemTimeSmoothed_nanoseconds - fieldbusStartTime_nanoseconds;
			double fieldbusTimeSmoothed_seconds = (double)fieldbusTimeSmoothed_nanoseconds / 1000000000.0;
			
			//save previous time values (in reference to fieldbus start)
			uint64_t previousCycleProgramTime_nanoseconds = currentCycleProgramTime_nanoseconds;
			double previousCycleProgramTime_seconds = currentCycleProgramTime_seconds;
			
			//assign new current time values (in reference to fieldbus start)
            currentCycleProgramTime_nanoseconds = fieldbusTimeSmoothed_nanoseconds;
            currentCycleProgramTime_seconds = (double)fieldbusTimeSmoothed_nanoseconds / 1000000000.0;
            
			//calculate current delta time values
			currentCycleDeltaT_seconds = currentCycleProgramTime_seconds - previousCycleProgramTime_seconds;
			currentCycleDeltaT_nanoseconds = currentCycleProgramTime_nanoseconds - previousCycleProgramTime_nanoseconds;
			
            //================= OPERATIONAL STATE TRANSITION ==================

            if (!b_clockStable && averageDCTimeDelta_nanoseconds < clockStableThreshold_nanoseconds) {
				//detect clock stabilisation and request operational state
				b_clockStable = true;
				startupProgress.setProgress(0.95, "Requesting Operational State");
                std::thread opStateHandler([]() {
					pthread_setname_np("EtherCAT Operational State Transition Handler");
					transitionToOperationalState();
				});
                opStateHandler.detach();
			}else if(!b_clockStable){
				//while the clocks are stabilizing, update the startup progress bar
				float from = 0.65;
				float to = 1.0;
				float maxValue = processInterval_milliseconds / 2.0;
				float minValue = clockStableThreshold_milliseconds;
				float value = metrics.averageDcTimeError_milliseconds;
				float percentage = 1.0 - ((value - minValue) / (maxValue - minValue));
				percentage = std::min(1.0f, percentage);
				percentage = std::max(0.0f, percentage);
				float progress = from + (to - from) * percentage;
				startupProgress.progress = progress;
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
		b_cyclicExchangeRunning = false; //set this in case we broke out of the main loop
        b_running = false;

		
		
		
		
        //send one last frame to all slaves to disable them
        //this way motors don't suddenly jerk to a stop when stopping the fieldbus in the middle of a movement
        for (auto slave : slaves) {
            slave->onDisconnection();
            slave->writeOutputs();
			slave->pushEvent("Device Disconnected (Fieldbus Shutdown)", false);
			slave->identity->state = EC_STATE_NONE;
        }
        ec_send_processdata();
        
        Environnement::updateEtherCatHardware();

        Logger::info("===== Cyclic Exchange Stopped !");

        //cleanup threads and relaunch slave detection handler
        if (slaveStateHandler.joinable()) slaveStateHandler.join();
		if(slaveErrorCounter.joinable()) slaveErrorCounter.join();
        startSlaveDetectionHandler();
		
		Environnement::stop();
    }

    //============== TRANSITION ALL SLAVES TO OPERATIONAL AFTER REFERENCE CLOCK AND MASTER CLOCKS ALIGNED ===================

    void transitionToOperationalState() {
        Logger::debug("===== Clocks Stabilized, Setting All Slaves to Operational state...");
        //set all slaves to operational (by setting slave 0 to operational)
        ec_slave[0].state = EC_STATE_OPERATIONAL;
        ec_writestate(0);
        //wait for all slaves to reach OP state
        if (EC_STATE_OPERATIONAL == ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE)) {
			startupProgress.setCompletion("Successfully Started EtherCAT Fieldbus");
            b_running = true;
            Logger::info("===== All slaves are operational");
            Logger::info("===== Successfully started EtherCAT Fieldbus");
            //addition read state is required to set the individual state of each slave
            //statecheck on slave zero doesn't assign the state of each individual slave, only global slave 0
            ec_readstate();
			handleStateTransitions();
			countSlaveErrors();
        }
        else {
			startupProgress.setFailure("Not all slaves reached Operational State. Check the Log for more detailed errors.");
            Logger::error("===== Not all slaves reached operational state... ");
			logDeviceStates();
			logAlStatusCodes();
            for (auto slave : slaves) {
                if (!slave->isStateOperational() || slave->hasStateError()) {
                    Logger::error("Slave '{}' has state {}", slave->getName(), slave->getEtherCatStateChar());
                }
            }
            stop();
        }
		b_starting = false;
    }

    //============== STATE HANDLING AND SLAVE RECOVERY =================

    void handleStateTransitions() {
		slaveStateHandler = std::thread([]() {
			pthread_setname_np("EtherCAT State Transition Handler Thread");
			Logger::debug("Started Slave State Handler Thread");
			while (b_cyclicExchangeRunning) {

				//save each slaves previous state
				for (auto slave : slaves) slave->previousState = slave->identity->state;

				//read the current state of each slave
				ec_readstate();

				//detect state changes by comparing the previous state with the current state
				for (auto slave : slaves) {
					if (slave->identity->state != slave->previousState) {
						if (slave->isStateNone()) {
							slave->pushEvent("Device Disconnected", true);
							slave->onDisconnection();
							Logger::error("Slave '{}' Disconnected...", slave->getName());
						}
						else if (slave->previousState == EC_STATE_NONE) {
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
		});
    }

	void countSlaveErrors(){
		slaveErrorCounter = std::thread([]() {
			pthread_setname_np("EtherCAT Slave Error Counter Thread");
			Logger::debug("Started Slave Error Counter Thread");
			while (b_cyclicExchangeRunning) {
				updateErrorCounters();
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			Logger::debug("Exited Slave Error Counter Thread");
		});
	}

	//========= DEVICE STATE LOGGING ========

	void logDeviceStates(){
		for (auto& slave : slaves) {
			Logger::warn("{} has state {}", slave->getName(), slave->getEtherCatStateChar());
		}
	}

	void logAlStatusCodes(){
		for(auto& slave : slaves){
			uint16_t AlStatusCode = 0x0;
			int wc = ec_FPRD(slave->getAssignedAddress(), 0x134, 2, &AlStatusCode, EC_TIMEOUTSAFE);
			if(wc != 1) Logger::error("{} : Could not read AL Status Code", AlStatusCode);
			else if(AlStatusCode != 0x0) Logger::error("{} : AL Status Code 0x{:X} ({})", slave->getName(), AlStatusCode, ec_ALstatuscode2string(AlStatusCode));
		}
	}








std::vector<std::shared_ptr<DeviceConnection>> networkTopology;
std::vector<std::shared_ptr<WrongConnection>> wrongConnections;

std::vector<std::shared_ptr<DeviceConnection>>& getNetworkTopology(){ return networkTopology; }
std::vector<std::shared_ptr<WrongConnection>>& getWrongConnections(){ return wrongConnections; }


void updateNetworkTopology(){
	
	//remove all existing connections
	networkTopology.clear();
	wrongConnections.clear();
	for(auto& device : slaves) device->connections.clear();
	
	
	
	for(auto device : slaves){
		if(device->identity->entryport != 0) {
			auto wrongConnection = std::make_shared<WrongConnection>();
			wrongConnection->device = device;
			wrongConnection->port = device->identity->entryport;
			wrongConnections.push_back(wrongConnection);
		}
	}
	if(!wrongConnections.empty()) return;
	
	
	
	
	//find the first device after the master on the network chain
	std::shared_ptr<EtherCatDevice> firstDeviceConnectedToMaster = nullptr;
	for(auto& device : slaves){
		if(device->identity->parent == 0) {
			firstDeviceConnectedToMaster = device;
			break;
		}
	}
	if(!firstDeviceConnectedToMaster) return;
	
	//add a special connection between the master and the first device
	auto masterConnection = std::make_shared<DeviceConnection>();
	masterConnection->b_parentIsMaster = true;
	masterConnection->childDevice = firstDeviceConnectedToMaster;
	masterConnection->childDevicePort = firstDeviceConnectedToMaster->identity->entryport;
	networkTopology.push_back(masterConnection);
	
	
	std::shared_ptr<EtherCatDevice> parentDevice = firstDeviceConnectedToMaster;
	std::vector<std::shared_ptr<EtherCatDevice>> devicesWithUnregisteredConnections;
	
	
	auto getNextChildDeviceOnPort = [](int port, std::shared_ptr<EtherCatDevice> parent, std::vector<std::shared_ptr<EtherCatDevice>>& children) -> std::shared_ptr<EtherCatDevice> {
		for(auto& connection : parent->connections){
			if(connection->parentDevice == parent && connection->parentDevicePort == port) return nullptr;
		}
		for(auto& child : children){
			if(child->identity->parentport == port) return child;
		}
		return nullptr;
	};
	
	
	auto getNextChildDeviceOf = [&getNextChildDeviceOnPort](std::shared_ptr<EtherCatDevice> parent) -> std::shared_ptr<EtherCatDevice> {
		int parentDeviceIndex = parent->getSlaveIndex();
		
		//get a list of devices that have the parent as parent
		std::vector<std::shared_ptr<EtherCatDevice>> childDevices;
		for(auto& device : slaves){
			if(device->identity->parent == parentDeviceIndex) childDevices.push_back(device);
		}
		
		//check the next port for an unregistered conneciont (order is 3 1 2 0)
		if(auto nextChild = getNextChildDeviceOnPort(3, parent, childDevices)) {
			return nextChild;
		}
		if(auto nextChild = getNextChildDeviceOnPort(1, parent, childDevices)) {
			return nextChild;
		}
		if(auto nextChild = getNextChildDeviceOnPort(2, parent, childDevices)) {
			return nextChild;
		}
		if(auto nextChild = getNextChildDeviceOnPort(0, parent, childDevices)) {
			return nextChild;
		}
		return nullptr;
	};
	
	
	auto hasUnregisteredConnections = [&](std::shared_ptr<EtherCatDevice> device) -> bool {
		
		int deviceIndex = device->getSlaveIndex();
		std::vector<std::shared_ptr<EtherCatDevice>> childDevices;
		for(auto& childDevice : slaves){
			if(childDevice->identity->parent == deviceIndex) childDevices.push_back(childDevice);
		}
		
		//check each child device of the queried device
		for(auto& childDevice : childDevices){
			
			bool b_connectionAlreadyRegistered = false;
			
			//for each connection that is registered by the device
			for(auto& connection : device->connections){
				//if a connection exists for between the queried device and one of its child device, that connection is already registered
				if(connection->parentDevice == device && connection->childDevice == childDevice) {
					b_connectionAlreadyRegistered = true;
					break;
				}
			}
			
			if(!b_connectionAlreadyRegistered) {
				return true;
			}
			
		}
		
		return false;
	};
	
	
	
	while(parentDevice){
		
		//get the next child with an unregistered connection to the parent device
		auto childDevice = getNextChildDeviceOf(parentDevice);
		
		//if there is a child with unregistered connection, register that connection with the parent and child
		if(childDevice){
			auto connection = std::make_shared<DeviceConnection>();
			connection->parentDevice = parentDevice;
			connection->parentDevicePort = childDevice->identity->parentport;
			connection->childDevice = childDevice;
			connection->childDevicePort = childDevice->identity->entryport;
			parentDevice->connections.push_back(connection);
			childDevice->connections.push_back(connection);
			networkTopology.push_back(connection);
		}
		
		//check if the parent device has unregistered connections
		if(hasUnregisteredConnections(parentDevice)){
			devicesWithUnregisteredConnections.push_back(parentDevice);
		}
		
		if(childDevice) parentDevice = childDevice;
		else {
	
			
			parentDevice = nullptr;
	
			//go back to the last device with unscanned ports and find its children
			while(!devicesWithUnregisteredConnections.empty()){
				
				//check if the last device with unregistered connections still has unregistered connections
				auto lastDeviceWithUnregisteredConnections = devicesWithUnregisteredConnections.back();
				if(hasUnregisteredConnections(lastDeviceWithUnregisteredConnections)){
					//if it has unregistered connections, get the next unregistered child
					parentDevice = lastDeviceWithUnregisteredConnections;
					break;
				}else{
					//if that device has no unregistered connections, remove it from the list of devices with unregistered connections
					devicesWithUnregisteredConnections.pop_back();
				}
					
			}
			
		}
	}
	
}

void updateErrorCounters(){
	for(auto& device : slaves) device->downloadErrorCounters();
	for(auto& connection : networkTopology){
		int largestErrorCount = 0;
		if(connection->b_parentIsMaster) {
			auto& childPortErrors = connection->childDevice->errorCounters.portErrors[connection->childDevicePort];
			largestErrorCount = std::max(largestErrorCount, cyclicFrameTimeoutCounter);
			largestErrorCount = std::max(largestErrorCount, cyclicFrameErrorCounter);
			largestErrorCount = std::max(largestErrorCount, (int)childPortErrors.frameRxErrors);
			largestErrorCount = std::max(largestErrorCount, (int)childPortErrors.physicalRxErrors);
			connection->b_wasDisconnected = childPortErrors.lostLinks > 0;
		}else{
			auto& parentPortErrors = connection->parentDevice->errorCounters.portErrors[connection->parentDevicePort];
			auto& childPortErrors = connection->childDevice->errorCounters.portErrors[connection->childDevicePort];
			connection->b_wasDisconnected = parentPortErrors.lostLinks > 0 || childPortErrors.lostLinks > 0;
			largestErrorCount = std::max(largestErrorCount, (int)parentPortErrors.frameRxErrors);
			largestErrorCount = std::max(largestErrorCount, (int)parentPortErrors.physicalRxErrors);
			largestErrorCount = std::max(largestErrorCount, (int)childPortErrors.frameRxErrors);
			largestErrorCount = std::max(largestErrorCount, (int)childPortErrors.physicalRxErrors);
		}
		connection->instability = (float)largestErrorCount / 255.0;
	}
}

void resetErrorCounters(){
	std::thread errorCounterResetter([](){
		cyclicFrameTimeoutCounter = 0;
		cyclicFrameErrorCounter = 0;
		for(auto& device : slaves) device->resetErrorCounters();
		updateErrorCounters();
	});
	errorCounterResetter.detach();
}

int getCyclicFrameTimeoutCounter(){ return cyclicFrameTimeoutCounter; }
int getCyclicFrameErrorCounter(){ return cyclicFrameErrorCounter; }




}
