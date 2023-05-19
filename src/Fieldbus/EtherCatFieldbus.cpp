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

    bool b_skipCycles = false;
    int maxSkippedFrames = 2;

    bool b_skippingFrames = false;
    int skippedFrames = 0;


	std::shared_ptr<NetworkInterfaceCard> activeNetworkInterfaceCard = nullptr;
	std::shared_ptr<NetworkInterfaceCard> getActiveNetworkInterfaceCard(){ return activeNetworkInterfaceCard; }

    std::vector<std::shared_ptr<EtherCatDevice>> discoveredDevices;			//slaves discovered on the network
    std::vector<std::shared_ptr<EtherCatDevice>> discoveredDevicesUnmatched; //slaves discovered on the network but not added to the environnement editor
	std::vector<std::shared_ptr<EtherCatDevice>>& getDevices(){ return discoveredDevices; }
	std::vector<std::shared_ptr<EtherCatDevice>>& getUnmatchedDevices(){ return discoveredDevicesUnmatched; }
	void removeUnmatchedDevice(std::shared_ptr<EtherCatDevice> removedDevice) {
		for(int i = 0; i < discoveredDevicesUnmatched.size(); i++){
			if(discoveredDevicesUnmatched[i] == removedDevice){
				discoveredDevicesUnmatched.erase(discoveredDevicesUnmatched.begin() + i);
				break;
			}
		}
	}


	//contiguous storage for cyclic exchange data
    uint8_t ioMap[MAX_IO_MAP_SIZE];
    int ioMapSize = 0;

	int cyclicFrameTimeoutCounter = 0;
	int cyclicFrameErrorCounter = 0;
	int getCyclicFrameTimeoutCounter(){ return cyclicFrameTimeoutCounter; }
	int getCyclicFrameErrorCounter(){ return cyclicFrameErrorCounter; }

	ProgressIndicator startupProgress;
    EtherCatMetrics metrics;
	EtherCatMetrics& getMetrics(){ return metrics; }

	//parameters
    int processInterval_milliseconds = 4.0;
    double processDataTimeout_milliseconds = 1.5;
    double clockStableThreshold_milliseconds = 0.1;
    double fieldbusTimeout_milliseconds = 100.0;

	//timing
	double currentCycleProgramTime_seconds = 0.0;
	long long int currentCycleProgramTime_nanoseconds = 0;
	double currentCycleDeltaT_seconds = 0.0;
	long long int currentCycleDeltaT_nanoseconds = 0;
	double getCycleProgramTime_seconds() { return currentCycleProgramTime_seconds; }
	long long int getCycleProgramTime_nanoseconds() { return currentCycleProgramTime_nanoseconds; }
	double getCycleTimeDelta_seconds() { return currentCycleDeltaT_seconds; }
	long long int getCycleTimeDelta_nanoseconds() { return currentCycleDeltaT_nanoseconds; }

    //====== non public functions ======

	bool initializeNetwork();
	void closeNetwork();

	bool identifyDevices();
	bool configureDevices();
	void updateNetworkTopology();

	void startCyclicExchange();
	void cycle();
	void transitionToOperationalState();
	void stopCyclicExchange();

	void startPollingNetworkInterfaceCards();
	void stopPollingNetworkInterfaceCards();

	void startPollingDiscoveredDevices();
	void stopPollingDiscoveredDevices();

	void startHandlingStateTransitions();
	void stopHandlingStateTransitions();

	void startCountingTransmissionErrors();
	void stopCountingTransmissionErrors();

	void startWatchingForErrors();
	void stopWatchingForErrors();

	void logDeviceStates();
	void logAlStatusCodes();

	bool getExplicitDeviceID(uint16_t configAddress, uint16_t& ID);



	//========== Network Control Methods

	bool b_networkInitializing = false;
	bool b_networkInitialized = false;
	bool b_networkStarting = false;
	bool b_networkRunning = false;

	bool canScan(){ return !b_networkInitializing && !b_networkStarting && !b_networkRunning; }
	bool canStart(){ return !b_networkInitializing && !b_networkStarting && !b_networkRunning; }
	bool canStop(){ return b_networkRunning; }

	bool isScanning(){ return b_networkInitializing; }
	bool isInitialized(){ return b_networkInitialized; }
	bool isStarting(){ return b_networkStarting; }
	bool isRunning(){ return b_networkRunning; }

	//============== public functions ==============

	void initialize(){
		scan();
		startPollingNetworkInterfaceCards();
	}
	void terminate(){
		stopPollingNetworkInterfaceCards();
		closeNetwork();
	}

	//non blocking
	void scan(){
		std::thread networkScanner([](){
			initializeNetwork();
		});
		networkScanner.detach();
	}

	//non blocking
	void start() {
		
		if(b_networkRunning){
			Logger::warn("Cannot start network while it is running");
			return;
		}
		else if(b_networkStarting){
			Logger::warn("Cannot start network while it is starting");
			return;
		}
		else if(b_networkInitializing){
			Logger::warn("Cannot start network while it is initializing");
			return;
		}
		
		b_networkStarting = true;

		startupProgress.setStart("Starting Fieldbus");
		Logger::info("===== Starting Fieldbus");
					
		std::thread ethercatFieldbusStarter([]() {
			pthread_setname_np("EtherCAT Process Starter Thread");
			
			if(!initializeNetwork()){
				Logger::error("failed to initialize network");
				b_networkStarting = false;
				return;
			}
			
			stopPollingDiscoveredDevices(); //because intializeNetwork() started polling devices
			
			if(!configureDevices()){
				logAlStatusCodes();
				Logger::error("Failed to start fieldbus.");
				b_networkStarting = false;
				return;
			}
			
			updateNetworkTopology();
			
			startCyclicExchange();

		});
		ethercatFieldbusStarter.detach();
		
	}

	void stop(){
		stopCyclicExchange();
	}

	//============= Check Network Permissions ==============

	bool hasNetworkPermissions(){
		//this checks if low lewel network packet manipulation permissions are granted to libpcap by the operating system
		//on windows machine its unclear how these permissions are enabled (by some module installed by wireshark?)
		//on macos permissions are disabled by default and can be enabled by running the "Install ChmodBPF.pkg" Installer provided with the repository in the utilities folder
		//trying to execute pcap or ethercat functions without these permissions will result in a bad memory access and hard crash
		//this will only report a correct status after a network interface card was opened
		ecx_contextt* context = &ecx_context;
		ecx_portt* port = context->port;
		ec_stackT* stack = &port->stack;
		pcap_t** socket_ptr = stack->sock;
		pcap_t* socket = *socket_ptr;
		return socket != nullptr;
	}

    //============== Update Network interface card list

    void getDetectedNetworkInterfaceCards(std::vector<std::shared_ptr<NetworkInterfaceCard>>& output) {
		
		output.clear();
		
		if (ec_adaptert* nics = ec_find_adapters()){
            while (nics != nullptr) {
				
				#ifdef STACATO_MACOS
				//on macos, we can skip network interfaces that don't have a name with the format en
				if (strstr(nics->name, "en") == nullptr) {
					nics = nics->next;
					continue;
				}
				#endif
				
				std::shared_ptr<NetworkInterfaceCard> nic = std::make_shared<NetworkInterfaceCard>();
				strcpy(nic->name, nics->name);
				strcpy(nic->description, nics->desc);
				output.push_back(nic);
				
                nics = nics->next;
            }
        }
    }

	bool b_nicCheckerRunning = false;
	std::thread nicChecker;

	std::vector<std::shared_ptr<NetworkInterfaceCard>> previousDetectedNics;

	void startPollingNetworkInterfaceCards(){
		b_nicCheckerRunning = true;
		nicChecker = std::thread([](){
			
			getDetectedNetworkInterfaceCards(previousDetectedNics);
			
			while(b_nicCheckerRunning){
				
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				if(b_networkRunning) continue;
				
				std::vector<std::shared_ptr<NetworkInterfaceCard>> newDetectedNics;
				std::vector<std::shared_ptr<NetworkInterfaceCard>> processedNics;
				getDetectedNetworkInterfaceCards(newDetectedNics);
				
				for(auto oldNic : previousDetectedNics){
					bool oldNicStillConnected = false;
					for(int i = newDetectedNics.size() - 1; i >= 0; i--){
						if(oldNic->matches(newDetectedNics[i])){
							//old network interface is still there, remove it from the new ones
							oldNicStillConnected = true;
							processedNics.push_back(newDetectedNics[i]);
							newDetectedNics.erase(newDetectedNics.begin() + i);
						}
					}
					if(!oldNicStillConnected) Logger::info("Network interface '{}' was disconnected", oldNic->name);
				}
				
				for(auto newNic : newDetectedNics){
					Logger::info("Network interface '{}' was connected", newNic->name);
					processedNics.push_back(newNic);
				}
				
				previousDetectedNics = processedNics;
				
				
				
				if(b_networkInitialized){
					
					auto currentNic = activeNetworkInterfaceCard;
					std::vector<std::shared_ptr<NetworkInterfaceCard>> detectedNics;
					getDetectedNetworkInterfaceCards(detectedNics);
					
					bool currentNicDetected = false;
					for(auto nic : detectedNics){
						if(strcmp(currentNic->name, nic->name) == 0 && strcmp(currentNic->description, nic->description) == 0){
							currentNicDetected = true;
							break;
						}
					}
					
					if(!currentNicDetected){
						Logger::critical("Active network interface '{}' was disconnected.", currentNic->name);
						closeNetwork();
					}
						
				}
				
			}
		});
	}
	
	void stopPollingNetworkInterfaceCards(){
		if(b_nicCheckerRunning){
			b_nicCheckerRunning = false;
			if(nicChecker.joinable()) nicChecker.join();
		}
	}

    //============== Search all NICs for slaves, initialize network and identify slaves

	bool initializeNetwork(){
		if(b_networkInitializing){
			Logger::error("Can't initialize the network while it is initializing");
			return false;
		}
		else if(b_networkRunning) {
			Logger::error("Can't initialize the network while it is running");
			return false;
		}
		else if(b_networkInitialized) {
			stopWatchingForErrors();
			stopPollingDiscoveredDevices();
			ec_close();
		}
		
		b_networkInitializing = true;
		
		Logger::info("Initializing EtherCAT Network and searching for devices.");
		startupProgress.setProgress(0.01, "Scanning Network for devices");
		
		b_networkInitialized = false;
		
		std::vector<std::shared_ptr<NetworkInterfaceCard>> detectedNics;
		getDetectedNetworkInterfaceCards(detectedNics);
		
		Logger::info("Found {} Network Interface Card{}", detectedNics.size(), detectedNics.size() == 1 ? "" : "s");
		for (auto& nic : detectedNics) Logger::debug("    = {} (ID: {})", nic->description, nic->name);
		
		//check all nics for EtherCAT slave responses
		Logger::info("Scanning all Network Interface Cards for EtherCAT slaves.");
		std::vector<std::shared_ptr<NetworkInterfaceCard>> nicsWithDetectedSlaves = {};
		for(auto nic : detectedNics){
			if(ec_init(nic->name) > 0){
				
				activeNetworkInterfaceCard = nic;
				
				//check for network permissions after opening a network interface
				if(!hasNetworkPermissions()){
					Logger::critical("Could not start EtherCAT Fieldbus because low level network packet manipulation is not allowed on the system.");
					Logger::critical("Please install ChmodBPF.pkg and restart Stacato.");
					return false;
				}
				
				uint16_t w;
				int workingCounter = ec_BRD(0x0000, ECT_REG_TYPE, sizeof(w), &w, EC_TIMEOUTSAFE);
				if(workingCounter > 0){
					nicsWithDetectedSlaves.push_back(nic);
					Logger::info("{} : Found {} EtherCAT slaves", nic->name, workingCounter);
				}else{
					Logger::trace("{} : Found 0 EtherCAT slaves", nic->name);
				}
				ec_close();
				activeNetworkInterfaceCard = nullptr;
				
			}else Logger::error("Failed to initialize Network Interface Card {}", nic->name);
		}
		
		if(nicsWithDetectedSlaves.empty()){
			//no slave on any nic
			Logger::warn("No Slaves detected on any Network Interface Card");
			startupProgress.setFailure("No EtherCAT slaves found.");
			b_networkInitializing = false;
			return false;
		}
		else{
			//one network interface card with slaves, or more than two
			std::shared_ptr<NetworkInterfaceCard> nic = nicsWithDetectedSlaves[0];
			
			if(nicsWithDetectedSlaves.size() != 1)
				Logger::info("Found EtherCAT slaves on more than two Network Interface Cards.");
			
			Logger::info("Starting EtherCAT on Network Interface {}", nic->name);
			
			if(ec_init(nic->name) <= 0){
				Logger::error("Failed to initialize EtherCAT on Network Interface {}", nic->name);
				startupProgress.setFailure("Failed to initialize network interface.");
				b_networkInitializing = false;
				return false;
			}
			
			activeNetworkInterfaceCard = nic;
			
			//discover and setup all slaves, get slave count and info in ec_slave, setup mailboxes, request state PRE-OP for all slaves
			//BUG: we need to do this twice: when swapping etherCAT connections, sometimes the aliasAdress doesn't read on the first config call (probably SOEM bug)
			if(ec_config_init(false) <= 0){
				ec_close();
				Logger::error("Failed to configure EtherCAT on Network Interface {}", nic->name);
				startupProgress.setFailure("Failed to configure network interface.");
				b_networkInitializing = false;
				return false;
			}
			
			b_networkInitialized = true;
			Logger::info("Initialized EtherCAT Network with {} devices on Network Interface {}", ec_slavecount, nic->name);
			
			startWatchingForErrors();
			
			identifyDevices();
			
			startPollingDiscoveredDevices();
			
			b_networkInitializing = false;
			return true;
		}
	}

	void closeNetwork(){
		if(b_networkInitialized){
			
			stopCyclicExchange();
			
			stopPollingDiscoveredDevices();
			stopWatchingForErrors();
			
			ec_close();
			
			b_networkInitialized = false;
		}
	}

    //============== Identify devices on the network

    bool identifyDevices() {
		
        //when rescanning the network, all previous slaves are now considered to be offline before being detected again
        //for a slave to appear as offline, we set its identity object (ec_slavet) to nullptr
        for (auto device : discoveredDevices) device->identity = nullptr;
        //we clear the list of slaves, slaves that are in the node graph remain there
        discoveredDevices.clear();
        //we also clear the list of slaves that were not in the nodegraph
        discoveredDevicesUnmatched.clear();

		//wait and check if all slaves have reached Pre Operational State like requested by ec_config_init()
		if (ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE) != EC_STATE_PRE_OP) {
			Logger::error("Not All Slaves Reached Pre-Operational State...");
			startupProgress.setFailure("Not all slaves reached Pre-Operational State. Check the Log for more detailed errors.");
			return false;
		}else Logger::info("All devices reached Pre-Operational State.");
		
		Logger::info("Identifying Devices.");
		startupProgress.setProgress(0.05, "Identifying Devices");

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
				discoveredDevicesUnmatched.push_back(slave);
			}

			slave->identity = &identity;
			slave->slaveIndex = i;

			if (!slave->isEtherCatDeviceKnown()) {
				Logger::warn("Found Unknown Slave: {}", identity.name);
			}
			
			slave->onDetection();

			//add the slave to the list of slaves regardless of environnement presence
			discoveredDevices.push_back(slave);
		}
		
		return true;
    }

    //============== Map Slave memory, Configure Distributed Clocks, Do Per slave configuration, Transition to Safe Operational ===============

	double configurationStartProgramTime_seconds = 0.0;

	double getConfigurationProgramStartTime_seconds(){
		return configurationStartProgramTime_seconds;
	}

    bool configureDevices() {

		startupProgress.setProgress(0.075, "Configuring Distributed Clocks");
        Logger::debug("===== Configuring Distributed Clocks");
		
        if (!ec_configdc()) {
			startupProgress.setFailure("Could not configure distributed clocks. Check the Log for more detailed errors.");
            Logger::error("===== Could not configure distributed clocks ...");
            //return false;
        }

		for(auto device : discoveredDevices) device->b_configurationSucceeded = false;
		
        //assign slave startup hooks
        for (int i = 1; i <= ec_slavecount; i++) {
            //we don't use the PO2SOconfigx hook since it isn't supported by ec_reconfig_slave()
            ec_slave[i].PO2SOconfig = [](uint16_t slaveIndex) -> int {
                for (auto device : discoveredDevices) {
                    if (device->getSlaveIndex() == slaveIndex) {
						
						int slaveCount = discoveredDevices.size();
						float confProgStart = 0.1;
						float confProgEnd = 0.5;
						float confIncrement = (confProgEnd - confProgStart) / slaveCount;
						float configurationProgress = device->getSlaveIndex() * confIncrement + confProgStart;
						
						static char slaveConfigurationString[256];
						sprintf(slaveConfigurationString, "Configuring %s", device->getName());
						startupProgress.setProgress(configurationProgress, slaveConfigurationString);
						
						Logger::debug("Configuring Slave '{}'", device->getName());
                        
                        if (device->startupConfiguration()) {
							device->b_configurationSucceeded = true;
                            Logger::info("Successfully configured Slave '{}'", device->getName());
                            return 1;
                        }
                        else {
							device->b_configurationSucceeded = false;
                            Logger::error("Failed to configure slave '{}'", device->getName());
                            return 0;
                        }
                    }
                }
                return 0;
            };
        }
		
		configurationStartProgramTime_seconds = Timing::getProgramTime_seconds();

        //build ioMap for PDO data, configure FMMU and SyncManager, request SAFE-OP state for all slaves
		startupProgress.setProgress(0.01, "Configuring Devices");
        Logger::debug("===== Beginning Device Configuration and IOMap Building...");
        ioMapSize = ec_config_map(ioMap); //this function starts the configuration
		
		for(auto device : discoveredDevices) {
			if(!device->b_configurationSucceeded){
				startupProgress.setFailure("Failed to configure devices");
				return Logger::warn("Failed to configure all devices");
			}
		}
		if (ioMapSize <= 0) return Logger::error("EtherCAT ioMap size is zero");
		else if(ioMapSize >= MAX_IO_MAP_SIZE) return Logger::critical("IoMap size is exceeded !");
		
        Logger::info("===== Finished Configuring Devices  (IOMap size : {} bytes)", ioMapSize);
		
        for (auto device : discoveredDevices) {
            Logger::debug("   [{}] '{}' {} bytes ({} bits)",
						  device->getSlaveIndex(),
						  device->getSaveName(),
						  device->identity->Ibytes + device->identity->Obytes,
						  device->identity->Ibits + device->identity->Obits);
            Logger::debug("          Inputs: {} bytes ({} bits)", device->identity->Ibytes, device->identity->Ibits);
            Logger::debug("          Outputs: {} bytes ({} bits)", device->identity->Obytes, device->identity->Obits);
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

	//============== Network Topology

	std::vector<std::shared_ptr<DeviceConnection>> networkTopology;
	std::vector<std::shared_ptr<WrongConnection>> wrongConnections;
	std::vector<std::shared_ptr<DeviceConnection>>& getNetworkTopology(){ return networkTopology; }
	std::vector<std::shared_ptr<WrongConnection>>& getWrongConnections(){ return wrongConnections; }

	void updateNetworkTopology(){
		
		//remove all existing connections
		networkTopology.clear();
		wrongConnections.clear();
		for(auto& device : discoveredDevices) device->connections.clear();
		
		
		
		for(auto device : discoveredDevices){
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
		for(auto& device : discoveredDevices){
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
			for(auto& device : discoveredDevices){
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
			for(auto& childDevice : discoveredDevices){
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

    //============== Cyclic Exchange

	//cyclic exchange timing variables
	uint64_t processInterval_nanoseconds;
	uint64_t processDataTimeout_microseconds;
	uint64_t systemTime_nanoseconds;
	uint64_t cycleStartTime_nanoseconds;
	uint64_t previousCycleStartTime_nanoseconds;
	uint64_t fieldbusStartTime_nanoseconds;
	uint64_t clockStableThreshold_nanoseconds;
	double averageDCTimeDelta_nanoseconds;
	int clockDriftCorrectionintegral;
	uint64_t lastProcessDataFrameReturnTime_nanoseconds;
	int64_t systemTimeErrorSmoothed_nanoseconds;
	int64_t systemTimeSmoothed_nanoseconds;
	bool b_clockStable;

	bool b_cyclicExchangeThreadRunning = false;
	bool b_cyclicExchangeTimedOut;

    void cyclicExchange(void* data) {
		
		b_cyclicExchangeThreadRunning = true;
		b_cyclicExchangeTimedOut = false;
		
		pthread_setname_np("EtherCAT Cyclic Exchange Thread (osal rtThread)");

		metrics.init(processInterval_milliseconds);
		
		//slaves are considered online when they are detected and actively exchanging data with the master
		//if we reached this state of the configuration, all slaves are detected and we are about to start exchanging data
		//we can trigger the onConnection event of all slaves
		for (auto device : discoveredDevices) {
			device->pushEvent("Device Connected (Fieldbus Started)", false);
			device->onConnection();
		}
		
        //thread timing variables
        processInterval_nanoseconds = processInterval_milliseconds * 1000000.0L;
        processDataTimeout_microseconds = processDataTimeout_milliseconds * 1000.0L;
        systemTime_nanoseconds = Timing::getProgramTime_nanoseconds() + processInterval_nanoseconds;
        cycleStartTime_nanoseconds = systemTime_nanoseconds + processInterval_nanoseconds;
        previousCycleStartTime_nanoseconds = systemTime_nanoseconds;
        fieldbusStartTime_nanoseconds = cycleStartTime_nanoseconds;
        clockStableThreshold_nanoseconds = clockStableThreshold_milliseconds * 1000000.0;
        averageDCTimeDelta_nanoseconds = (double)processInterval_nanoseconds / 2.0;
        clockDriftCorrectionintegral = 0;
		lastProcessDataFrameReturnTime_nanoseconds = Timing::getProgramTime_nanoseconds();
		systemTimeErrorSmoothed_nanoseconds = 0;
		systemTimeSmoothed_nanoseconds = cycleStartTime_nanoseconds;
		b_clockStable = false;
		
        Logger::info("===== Waiting For clocks to stabilize before requesting Operational State...");
		startupProgress.setProgress(0.65, "Waiting for clocks to stabilize");
		
		//========= CYCLIC EXCHANGE ============
		while (b_cyclicExchangeThreadRunning && !b_cyclicExchangeTimedOut) cycle();
		//======================================

        //send one last frame to all slaves to disable them
        //this way motors don't suddenly jerk to a stop when stopping the fieldbus in the middle of a movement
		//TODO: is the previous still valid ?
        for (auto device : discoveredDevices) {
			device->onDisconnection();
			device->writeOutputs();
			device->pushEvent("Device Disconnected (Fieldbus Shutdown)", false);
			device->identity->state = EC_STATE_NONE;
        }
        ec_send_processdata();
        
        Environnement::updateEtherCatHardware();

        Logger::info("===== Cyclic Exchange Stopped !");

        //cleanup threads and relaunch slave detection handler
		
		stopHandlingStateTransitions();
		stopCountingTransmissionErrors();
		
		startPollingDiscoveredDevices();
		
		Environnement::stop();
		
		b_cyclicExchangeThreadRunning = false;
		b_networkRunning = false;
		b_networkStarting = false; //we set this in case we canceled network starting during clock stabilisation
    }

	void cycle(){
		using namespace std::chrono;
		
		//======================= THREAD TIMING =========================

		//bruteforce timing precision by using 100% of CPU core
		//update and compare system time to next process
		do { systemTime_nanoseconds = Timing::getProgramTime_nanoseconds(); } while (systemTime_nanoseconds < cycleStartTime_nanoseconds);

		//============= PROCESS DATA SENDING AND RECEIVING ==============

		//EXPERIMENTAL
		if(b_skipCycles){
			b_skipCycles = false;
			b_skippingFrames = true;
			skippedFrames = 0;
			Logger::warn("Skipping {} EtherCAT Frames", maxSkippedFrames);
		}else if(b_skippingFrames){
			skippedFrames++;
			Logger::warn("Skipping frame {}/{}", skippedFrames, maxSkippedFrames);
			if(skippedFrames >= maxSkippedFrames) b_skippingFrames = false;
		}
		
		int workingCounter;
		uint64_t frameSentTime_nanoseconds = systemTime_nanoseconds;
		uint64_t frameReceivedTime_nanoseconds = systemTime_nanoseconds;
		
		if(!b_skippingFrames){
			ec_send_processdata();
			frameSentTime_nanoseconds = Timing::getProgramTime_nanoseconds();
			workingCounter = ec_receive_processdata(processDataTimeout_microseconds);
			frameReceivedTime_nanoseconds = Timing::getProgramTime_nanoseconds();
			if(frameSentTime_nanoseconds < cycleStartTime_nanoseconds){
				//Logger::error("Weird frame send time... {} {}", frameSentTime_nanoseconds, cycleStartTime_nanoseconds);
				frameSentTime_nanoseconds = cycleStartTime_nanoseconds;
			}
		}
			
		metrics.frameCount++;
		
		switch(workingCounter){
			case EC_NOFRAME:
			case EC_TIMEOUT:
				cyclicFrameTimeoutCounter++;
				metrics.droppedFrameCount++;
				break;
			case EC_OTHERFRAME:
			case EC_ERROR:
			case EC_SLAVECOUNTEXCEEDED:
				cyclicFrameErrorCounter++;
				metrics.droppedFrameCount++;
				break;
		}

		//===================== TIMEOUT HANDLING ========================

		if (workingCounter <= 0) {
			if (Timing::getProgramTime_nanoseconds() - lastProcessDataFrameReturnTime_nanoseconds > fieldbusTimeout_milliseconds * 1000000) {
				Logger::critical("Fieldbus timed out...");
				b_cyclicExchangeTimedOut = true;
				return; //breaks out of the main while loop (which stops the fieldbus?)
			}
			//adjust the copy of the reference clock in case no frame was received
			ec_DCtime += processInterval_nanoseconds;
		}
		else lastProcessDataFrameReturnTime_nanoseconds = Timing::getProgramTime_nanoseconds(); //reset timeout watchdog

		//===================== ENVIRONNEMENT UPDATE =======================

		if (b_networkRunning) Environnement::updateEtherCatHardware();

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
			transitionToOperationalState();
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

	void transitionToOperationalState() {
		std::thread opStateHandler([]() {
			pthread_setname_np("EtherCAT Operational State Transition Handler");
			Logger::debug("===== Clocks Stabilized, Setting All Slaves to Operational state...");
			//set all slaves to operational (by setting slave 0 to operational)
			ec_slave[0].state = EC_STATE_OPERATIONAL;
			ec_writestate(0);
			//wait for all slaves to reach OP state
			if (EC_STATE_OPERATIONAL == ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE)) {
				
				//addition read state is required to set the individual state of each slave
				//statecheck on slave zero doesn't assign the state of each individual slave, only global slave 0
				ec_readstate();
				b_networkRunning = true;

				startHandlingStateTransitions();
				startCountingTransmissionErrors();
				
				startupProgress.setCompletion("Successfully Started EtherCAT Fieldbus");
				Logger::info("===== All slaves are operational");
				Logger::info("===== Successfully started EtherCAT Fieldbus");

			}
			else {
				startupProgress.setFailure("Not all slaves reached Operational State. Check the Log for more detailed errors.");
				Logger::error("===== Not all slaves reached operational state... ");
				logDeviceStates();
				logAlStatusCodes();
				for (auto device : discoveredDevices) {
					if (!device->isStateOperational() || device->hasStateError()) {
						Logger::error("Device '{}' has state {}", device->getName(), device->getEtherCatStateChar());
					}
				}
				stop();
			}
			b_networkStarting = false;
		});
		opStateHandler.detach();
	}

	pthread_t cyclicExchangeThread;
	int stackSize = 65536;

	void startCyclicExchange() {
		
		//if the cyclic exchange thread was not terminate manually (because of fieldbus timeout)
		//it still needs to be joined (or maybe we should just detach it)
		pthread_join(cyclicExchangeThread, nullptr);
		
		if (b_cyclicExchangeThreadRunning){
			Logger::error("Can't start Cyclic exchange while it is running");
			return;
		}
		
		startupProgress.setProgress(0.6, "Starting Cyclic Exchange");
		Logger::debug("===== Starting Cyclic Process Data Exchange");
			
		osal_thread_create_rt(&cyclicExchangeThread, stackSize, (void*)&cyclicExchange, nullptr);

	}

	void stopCyclicExchange(){
		if(b_cyclicExchangeThreadRunning){
			b_cyclicExchangeThreadRunning = false;
		}
		pthread_join(cyclicExchangeThread, nullptr);
	}




    //============== STATE HANDLING AND SLAVE RECOVERY =================

	std::thread slaveStateHandler;
	bool b_slaveStateHandlerRunning = false;

    void startHandlingStateTransitions() {
		
		if(b_slaveStateHandlerRunning){
			Logger::error("Can't start slave state transition handler while it is running.");
			return;
		}
		
		slaveStateHandler = std::thread([]() {
			b_slaveStateHandlerRunning = true;
			
			pthread_setname_np("EtherCAT State Transition Handler Thread");
			Logger::debug("Started Slave State Handler Thread");
			while (b_slaveStateHandlerRunning) {

				//save each slaves previous state
				for (auto device : discoveredDevices) device->previousState = device->identity->state;

				//read the current state of each slave
				ec_readstate();

				//detect state changes by comparing the previous state with the current state
				for (auto device : discoveredDevices) {
					if (device->identity->state != device->previousState) {
						if (device->isStateNone()) {
							device->pushEvent("Device Disconnected", true);
							device->onDisconnection();
							Logger::error("Slave '{}' Disconnected...", device->getName());
						}
						else if (device->previousState == EC_STATE_NONE) {
							char eventString[64];
							sprintf(eventString, "Device Reconnected with state %s", device->getEtherCatStateChar());
							device->pushEvent(eventString, false);
							device->onConnection();
							Logger::info("Slave '{}' reconnected with state {}", device->getName(), device->getEtherCatStateChar());
						}
						else {
							char eventString[64];
							sprintf(eventString, "EtherCAT state changed to %s", device->getEtherCatStateChar());
							device->pushEvent(eventString, false);
							Logger::info("Slave '{}' state changed to {}", device->getName(), device->getEtherCatStateChar());
						}
					}
				}

				//try to recover slaves that are not online or in operational state
				for (auto device : discoveredDevices) {
					if (device->isStateNone()) {
						//recover is useful to detect a slave that has a power cycle and lost its configured address
						//recover uses incremental addressing to detect if an offline slave pops up at the same place in the network
						//if a slave responds at that address, the function verify it matches the previous slave at that address
						//it then reattributes an configured address to the slave
						//the function returns 1 if the slave was successfully recovered with its previous configured address
						if (1 == ec_recover_slave(device->getSlaveIndex(), EC_TIMEOUTRET3)) {
							device->pushEvent("Device Reconnected after power cycle", false);
							device->onConnection();
							Logger::info("Recovered Device '{}' !", device->getName());
						}
					}
					else if (device->isStateSafeOperational() && !device->hasStateError()) {
						//set the slave back to operational after reconfiguration
						device->identity->state = EC_STATE_OPERATIONAL;
						ec_writestate(device->getSlaveIndex());
						if (EC_STATE_OPERATIONAL == ec_statecheck(device->getSlaveIndex(), EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE)) {
							device->pushEvent("Device back in operational state", false);
							Logger::info("Slave '{}' is back in Operational State!", device->getName());
						}
					}
					else if (!device->isStateOperational() || device->hasStateError()) {
						//reconfigure looks for a slave that still has the same configured address
						//if no slave is found at the configured address, the function does nothing
						//this mean the function cannot directly be used to reconfigure a slave that had a power cycle and lost its configured address
						//reconfigure takes the slave back to init and reconfigures it all the way through safeoperational
						//we then need to set it back to operational
						//the ec_reconfig_slave function returns the status of the slave
						if (EC_STATE_SAFE_OP == ec_reconfig_slave(device->getSlaveIndex(), EC_TIMEOUTRET3)) {
							device->pushEvent("Device Reconfigured", false);
							Logger::info("Slave '{}' Successfully Reconfigured", device->getName());
						}
					}
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			Logger::debug("Exited Slave State Handler Thread");
		});
    }

	void stopHandlingStateTransitions(){
		if(b_slaveStateHandlerRunning){
			b_slaveStateHandlerRunning = false;
			if(slaveStateHandler.joinable()) slaveStateHandler.join();
		}
	}


	//============== TRANSMISSION ERROR COUNTER =================

	std::thread transmissionErrorCounterThread;
	bool b_transmissionErrorCounterRunning = false;

	void updateTransmissionErrorCounters(){
		for(auto& device : discoveredDevices) device->downloadErrorCounters();
		for(auto& connection : networkTopology){
			int largestErrorCount = 0;
			if(connection->b_parentIsMaster) {
				auto& childPortErrors = connection->childDevice->errorCounters.portErrors[connection->childDevicePort];
				double masterStability_f = 100.0 * 255.0 * (double)EtherCatFieldbus::getMetrics().droppedFrameCount / (double)EtherCatFieldbus::getMetrics().frameCount;
				int masterStability_i = std::clamp((int)masterStability_f, 0, 255);
				largestErrorCount = std::max(largestErrorCount, masterStability_i);
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

	void startCountingTransmissionErrors(){
		if(b_transmissionErrorCounterRunning){
			Logger::error("Can't start transmission error counter while it is running");
			return;
		}
		transmissionErrorCounterThread = std::thread([]() {
			b_transmissionErrorCounterRunning = true;
			pthread_setname_np("EtherCAT Transmission Error Counter");
			Logger::debug("Started Transmission Error Counter");
			while (b_transmissionErrorCounterRunning) {
				updateTransmissionErrorCounters();
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
			Logger::debug("Exited Transmission Error Counter");
		});
	}

	void stopCountingTransmissionErrors(){
		if(b_transmissionErrorCounterRunning){
			b_transmissionErrorCounterRunning = false;
			if(transmissionErrorCounterThread.joinable()) transmissionErrorCounterThread.join();
		}
	}

	void resetErrorCounters(){
		std::thread errorCounterResetter([](){
			cyclicFrameTimeoutCounter = 0;
			cyclicFrameErrorCounter = 0;
			for(auto& device : discoveredDevices) device->resetErrorCounters();
			updateTransmissionErrorCounters();
		});
		errorCounterResetter.detach();
	}

	//========== GENERAL ERROR WATCHING ============

	bool b_errorWatcherRunning = false;
	std::thread errorWatcherThread;

	void startWatchingForErrors() {
		if(b_errorWatcherRunning){
			Logger::error("Can't start error watcher while it is runnning");
			return;
		}
		
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

	void stopWatchingForErrors() {
		if(b_errorWatcherRunning){
			b_errorWatcherRunning = false;
			errorWatcherThread.join();
		}
	}


	//============ DISCOVERED DEVICE DETECTION ==============

	bool b_detectionThreadRunning = false;
	std::thread slaveDetectionThread;

	void startPollingDiscoveredDevices() {
		return;
		if (b_detectionThreadRunning) {
			Logger::error("Can't start Discovered Device Detection while it is running");
			return;
		}
		b_detectionThreadRunning = true;
		slaveDetectionThread = std::thread([]() {
			pthread_setname_np("EtherCAT Detection Handler Thread");
			Logger::debug("Started EtherCAT Detection Handler");
			while (b_detectionThreadRunning) {
				if(b_networkInitialized) ec_readstate();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				
			}
			Logger::debug("Exited EtherCAT Detection Handler");
		});
	}

	void stopPollingDiscoveredDevices() {
		return;
		if(b_detectionThreadRunning){
			b_detectionThreadRunning = false;
			slaveDetectionThread.join();
		}
	}



	//========= DEVICE STATE LOGGING ========

	void logDeviceStates(){
		for (auto& device : discoveredDevices) {
			Logger::warn("Device {} has state {}", device->getName(), device->getEtherCatStateChar());
		}
	}

	void logAlStatusCodes(){
		for(auto& device : discoveredDevices){
			uint16_t AlStatusCode = 0x0;
			int wc = ec_FPRD(device->getAssignedAddress(), 0x134, 2, &AlStatusCode, EC_TIMEOUTSAFE);
			if(wc != 1) Logger::error("{} : Could not read AL Status Code", AlStatusCode);
			else if(AlStatusCode != 0x0) Logger::error("{} : AL Status Code 0x{:X} ({})", device->getName(), AlStatusCode, ec_ALstatuscode2string(AlStatusCode));
		}
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

}
