#include <pch.h>

#include "Environnement.h"


#include "NodeGraph/NodeGraph.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Motion/Machine/Machine.h"

#include "Gui/ApplicationWindow/ApplicationWindow.h"

#include "Motion/Playback.h"

#include "Networking/Network.h"
#include "Networking/NetworkDevice.h"

namespace Environnement {

bool b_isStarting = false;
bool b_isRunning = false;
bool b_isSimulating = false;

double simulationStartTime_seconds = 0.0;
long long int simulationStartTime_nanoseconds = 0;
double simulationTime_seconds = 0.0;
long long int simulationTime_nanoseconds = 0;
double simulationTimeDelta_seconds = 0.0;
long long int simulationTimeDelta_nanoseconds = 0;

void startSimulation();
void stopSimulation();
void startHardware();
void stopHardware();

void updateSimulation();
std::thread environnementSimulator;

void initialize(){
	if(isRunning()) stop();
	EtherCatFieldbus::updateNetworkInterfaceCardList();
	Network::init();
}

void terminate(){
	if(isRunning()) stop();
	EtherCatFieldbus::terminate();
	Network::terminate();
}

void open(){
	EtherCatFieldbus::init();
}

void close(){
	EtherCatFieldbus::terminate();
}

void start(){
	if(b_isSimulating) startSimulation();
	else startHardware();
}

void stop(){
	if(b_isSimulating) stopSimulation();
	else stopHardware();
}

bool isReady(){
	if(b_isSimulating) return true;
	else return EtherCatFieldbus::isNetworkInitialized();
}

bool isStarting(){
	return b_isStarting;
}

bool isRunning(){
	return b_isRunning;
}

bool isSimulating(){
	return b_isSimulating;
}

void setSimulation(bool b_sim){
	if(!isRunning()) b_isSimulating = b_sim;
}




void startSimulation(){
	b_isRunning = true;
	Logger::info("Starting Environnement Simulation");
	
	simulationStartTime_seconds = Timing::getProgramTime_seconds();
	simulationStartTime_nanoseconds = Timing::getProgramTime_nanoseconds();
	
	environnementSimulator = std::thread([](){
		while(b_isRunning){
			updateSimulation();
			//run simulation at 100Hz and free the cpu core in between processing cycles
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		Logger::info("Stopped Environnement Simulation");
	});
	
	for(auto& networkDevice : getNetworkDevices()) networkDevice->connect();
	
	Logger::info("Started Environnement Simulation");
}

void stopSimulation(){
	disableAllMachines();
	for(auto& networkDevice : getNetworkDevices()) networkDevice->disconnect();
	b_isRunning = false;
	if(environnementSimulator.joinable()) environnementSimulator.join();
}

void startHardware(){
	b_isStarting = true;
	Logger::info("Starting Environnement Hardware");
	EtherCatFieldbus::start();
	std::thread environnementHardwareStarter([](){
		//first sleep, to allow the fieldbus some time to start
		//then check if the fieldbus is starting until it isn't anymore
		do{ std::this_thread::sleep_for(std::chrono::milliseconds(20));
		} while(EtherCatFieldbus::isStarting());
		if(!EtherCatFieldbus::isRunning()) {
			b_isRunning = false;
			b_isStarting = false;
			EtherCatFieldbus::stop();
			Logger::warn("Failed to Start Environnement Hardware");
			return;
		}
		
		for(auto& networkDevice : getNetworkDevices()) networkDevice->connect();
		
		Logger::info("Started Environnement Hardware");
		b_isRunning = true;
		b_isStarting = false;
	});
	environnementHardwareStarter.detach();
}


void stopHardware(){
	for(auto& machine : getMachines()) machine->disable();
	EtherCatFieldbus::stop();
	for(auto& networkDevice : getNetworkDevices()) networkDevice->disconnect();
	b_isRunning = false;
}


void updateEtherCatHardware(){
	//interpret all slaves input data if operational
	for (auto slave : EtherCatFieldbus::slaves) if (slave->isStateOperational()) slave->readInputs();
	
	//increments the playback position of all active manoeuvres
	//if a manoeuvre finishes playback, this sets its playback position to the exact end of the manoeuvre
	Playback::incrementPlaybackPosition();
	
	//update all nodes connected to ethercat slave nodes
	Environnement::nodeGraph->evaluate(Device::Type::ETHERCAT_DEVICE);
	
	//ends playback of finished manoeuvres and rapids
	//triggers the onParameterPlaybackEnd() method of machines
	Playback::updateActiveManoeuvreState();
	
	//prepare all slaves output data if operational
	for (auto slave : EtherCatFieldbus::slaves) if (slave->isStateOperational()) slave->prepareOutputs();
}

void updateSimulation(){
	
	//get current time
	double currentSimulationTime_seconds = Timing::getProgramTime_seconds() - simulationStartTime_seconds;
	long long int currentSimulationTime_nanoseconds = Timing::getProgramTime_nanoseconds() - simulationStartTime_nanoseconds;
	
	//update time deltas
	simulationTimeDelta_seconds = currentSimulationTime_seconds - simulationTime_seconds;
	simulationTimeDelta_nanoseconds = currentSimulationTime_nanoseconds - simulationTime_nanoseconds;
	
	//update current time
	simulationTime_seconds = currentSimulationTime_seconds;
	simulationTime_nanoseconds = currentSimulationTime_nanoseconds;
	
	Playback::incrementPlaybackPosition();
	
	for(auto& machine : getMachines()) machine->simulateProcess();
	
	Playback::updateActiveManoeuvreState();
}


double getTime_seconds(){
	if(b_isSimulating) return simulationTime_seconds;
	else return EtherCatFieldbus::getCycleProgramTime_seconds();
}

long long int getTime_nanoseconds(){
	if(b_isSimulating) return simulationTime_nanoseconds;
	else return EtherCatFieldbus::getCycleProgramTime_nanoseconds();
}

double getDeltaTime_seconds(){
	if(b_isSimulating) return simulationTimeDelta_seconds;
	else return EtherCatFieldbus::getCycleTimeDelta_seconds();
}

long long int getDeltaTime_nanoseconds(){
	if(b_isSimulating) return simulationTimeDelta_nanoseconds;
	else return EtherCatFieldbus::getCycleTimeDelta_nanoseconds();
}




std::shared_ptr<NodeGraph> nodeGraph = std::make_shared<NodeGraph>();
std::vector<std::shared_ptr<EtherCatDevice>> etherCatDevices;
std::vector<std::shared_ptr<Machine>> machines;
std::vector<std::shared_ptr<NetworkDevice>> networkDevices;

std::shared_ptr<Machine> selectedMachine;
std::shared_ptr<EtherCatDevice> selectedEtherCatDevice;

std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices() {
	return etherCatDevices;
}

std::vector<std::shared_ptr<Machine>>& getMachines() {
	return machines;
}

std::vector<std::shared_ptr<NetworkDevice>>& getNetworkDevices(){
	return networkDevices;
}

void enableAllMachines() {
	for (auto machine : machines) machine->enable();
}

void disableAllMachines() {
	for (auto machine : machines) machine->disable();
}

bool areAllMachinesEnabled() {
	if (machines.empty()) return false;
	for (auto machine : machines) {
		if (!machine->isEnabled()) return false;
	}
	return true;
}

bool areNoMachinesEnabled() {
	for(auto machine : machines) {
		if (machine->isEnabled()) return false;
	}
	return true;
}

void addNode(std::shared_ptr<Node> node) {
	switch (node->getType()) {
		case Node::Type::MACHINE:
			machines.push_back(std::dynamic_pointer_cast<Machine>(node));
			break;
		case Node::Type::IODEVICE:{
			std::shared_ptr<Device> deviceNode = std::dynamic_pointer_cast<Device>(node);
			switch (deviceNode->getDeviceType()) {
				case Device::Type::ETHERCAT_DEVICE:
					etherCatDevices.push_back(std::dynamic_pointer_cast<EtherCatDevice>(deviceNode));
					break;
				case Device::Type::NETWORK_DEVICE:
					networkDevices.push_back(std::dynamic_pointer_cast<NetworkDevice>(deviceNode));
					break;
				case Device::Type::USB_DEVICE:
					break;
			}
		}break;
		default:
			break;
	}
}

void removeNode(std::shared_ptr<Node> node){
	switch (node->getType()) {
		case Node::Type::MACHINE:{
			std::shared_ptr<Machine> machineNode = std::dynamic_pointer_cast<Machine>(node);
			for (int i = 0; i < machines.size(); i++) {
				if (machines[i] == machineNode) {
					machines.erase(machines.begin() + i);
					break;
				}
			}
		}break;
		case Node::Type::IODEVICE:{
			std::shared_ptr<Device> deviceNode = std::dynamic_pointer_cast<Device>(node);
			switch (deviceNode->getDeviceType()) {
			case Device::Type::ETHERCAT_DEVICE: {
				std::shared_ptr<EtherCatDevice> etherCatDeviceNode = std::dynamic_pointer_cast<EtherCatDevice>(deviceNode);
				for (int i = 0; i < etherCatDevices.size(); i++) {
					if (etherCatDevices[i] == etherCatDeviceNode) {
						etherCatDevices.erase(etherCatDevices.begin() + i);
						break;
					}
				}
			}break;
			case Device::Type::NETWORK_DEVICE: {
				std::shared_ptr<NetworkDevice> networkDeviceNode = std::dynamic_pointer_cast<NetworkDevice>(deviceNode);
				for (int i = 0; i < networkDevices.size(); i++) {
					if (networkDevices[i] == networkDeviceNode) {
						networkDevices.erase(networkDevices.begin() + i);
						break;
					}
				}
			}break;
			case Device::Type::USB_DEVICE: {
			}break;
			}
		}break;
		default:
			break;
	}
}

char name[256] = "Default Environnement";
char notes[65536] = "";

void setName(const char* newName) {
	strcpy(name, newName);
	updateName();
}

void updateName(){
	ApplicationWindow::setWindowName(name);
}

const char* getName() { return name; }

const char* getNotes() { return notes; }

void setNotes(const char* _notes){
	sprintf(notes, "%s", _notes);
}

}
