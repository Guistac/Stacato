#pragma once

#include "Utilities/EtherCatMetrics.h"

class EtherCatDevice;

struct NetworkInterfaceCard {
    char description[128];
    char name[128];
};

namespace tinyxml2 { class XMLElement; }

struct ProgressIndicator;

namespace EtherCatFieldbus {

	//EXPERIMENTAL SETTINGS
	extern bool b_usePosixRealtimeThread;
    extern bool b_skipCycles;
    extern int maxSkippedFrames;


	//Network interface
	std::shared_ptr<NetworkInterfaceCard> getActiveNetworkInterfaceCard();

	//Discovered Devices
	std::vector<std::shared_ptr<EtherCatDevice>>& getDevices();             //all slaves discovered on the network
	std::vector<std::shared_ptr<EtherCatDevice>>& getUnmatchedDevices();	//discovered slaves that are not in the environnement nodegraph
	void removeUnmatchedDevice(std::shared_ptr<EtherCatDevice> removedDevice);

	void initialize();
	void terminate();

	//Network Control
	void scan();
    void start();
	void stop();
	
	//Network State
	bool isScanning();
	bool isInitialized();
	bool isStarting();
	bool isRunning();
	bool canScan();
	bool canStart();
	bool canStop();

	extern ProgressIndicator startupProgress;

	EtherCatMetrics& getMetrics();

	//cyclic exchange timing
	double getCycleProgramTime_seconds();
	long long getCycleProgramTime_nanoseconds();
	double getCycleTimeDelta_seconds();
	long long getCycleTimeDelta_nanoseconds();

	//Cyclic Echange Timing Settings
	extern int processInterval_milliseconds;
	extern double processDataTimeout_milliseconds;
	extern double clockStableThreshold_milliseconds;
	extern double fieldbusTimeout_milliseconds;

    //Save and load EtherCAT settings
    bool save(tinyxml2::XMLElement* xml);
    bool load(tinyxml2::XMLElement* xml);



	struct DeviceConnection{
		bool b_parentIsMaster = false;
		std::shared_ptr<EtherCatDevice> parentDevice;
		uint8_t parentDevicePort;
		std::shared_ptr<EtherCatDevice> childDevice;
		uint8_t childDevicePort;
		float instability = 1.0;
		bool b_wasDisconnected = false;
	};
	struct WrongConnection{
		std::shared_ptr<EtherCatDevice> device;
		uint8_t port;
	};
	std::vector<std::shared_ptr<DeviceConnection>>& getNetworkTopology();
	std::vector<std::shared_ptr<WrongConnection>>& getWrongConnections();
	
	void resetErrorCounters();

	int getCyclicFrameTimeoutCounter();
	int getCyclicFrameErrorCounter();

	double getConfigurationProgramStartTime_seconds();

};

