#include <pch.h>

#include "Environnement.h"


#include "NodeGraph/NodeGraph.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Motion/Machine/Machine.h"

#include "Motion/Playback.h"

namespace Environnement {


bool b_isRunning = false;
bool b_isSimulating = false;

void startSimulation();
void updateSimulation();
std::thread environnementSimulator;


void start(){
	if(b_isSimulating){
		startSimulation();
	}else{
		//EtherCatFieldbus::start();
	}
}


void stop(){
	if(b_isSimulating){
		b_isRunning = false;
		environnementSimulator.join();
	}else{
		//EtherCatFieldbus::stop();
	}
}


bool isRunning(){
	return b_isRunning;
}


bool isSimulating(){
	return b_isSimulating;
}

void setSimulation(bool sim){
	if(!isRunning()) b_isSimulating = sim;
}



void update(){
	//interpret all slaves input data if operational
	for (auto slave : EtherCatFieldbus::slaves) if (slave->isStateOperational()) slave->readInputs();
	
	//increments the playback position of all active manoeuvres
	Playback::incrementPlaybackPosition();
	
	//update all nodes connected to ethercat slave nodes
	Environnement::nodeGraph->evaluate(Device::Type::ETHERCAT_DEVICE);
	
	//ends playback of finished manoeuvres and rapids
	Playback::updateActiveManoeuvreState();
	
	//prepare all slaves output data if operational
	for (auto slave : EtherCatFieldbus::slaves) if (slave->isStateOperational()) slave->prepareOutputs();
}

void startSimulation(){
	b_isRunning = true;
	
	environnementSimulator = std::thread([](){
		Logger::info("Started Environnement Simulation");
		
		while(b_isRunning){
			updateSimulation();
			//run simulation at 100Hz and free the cpu core in between processing cycles
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
		Logger::info("Stopped Environnement Simulation");
	});
}

void updateSimulation(){
	Playback::incrementPlaybackPosition();
	
	for(auto& machine : getMachines()){
		//TODO: simulate machine motion
		//machine->updateSimulation();
	}
	
	Playback::updateActiveManoeuvreState();
}

double getTime_seconds(){
	if(b_isSimulating) return 0.0;
	else return EtherCatFieldbus::getCycleProgramTime_seconds();
}

long long int getTime_nanoseconds(){
	if(b_isSimulating) return 0;
	else return EtherCatFieldbus::getCycleProgramTime_nanoseconds();
}

double getDeltaTime_seconds(){
	if(b_isSimulating) return 0.0;
	else return EtherCatFieldbus::getCycleTimeDelta_seconds();
}

long long int getDeltaTime_nanoseconds(){
	if(b_isSimulating) return 0;
	else return 0;
}




std::shared_ptr<NodeGraph> nodeGraph = std::make_shared<NodeGraph>();
std::vector<std::shared_ptr<EtherCatDevice>> etherCatDevices;
std::vector<std::shared_ptr<Machine>> machines;

std::shared_ptr<Machine> selectedMachine;
std::shared_ptr<EtherCatDevice> selectedEtherCatDevice;

std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices() {
	return etherCatDevices;
}

std::vector<std::shared_ptr<Machine>>& getMachines() {
	return machines;
}

void enableAllMachines() {
	for (auto machine : machines) {
		machine->enable();
	}
}

void disableAllMachines() {
	for (auto machine : machines) {
		machine->disable();
	}
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

void setName(const char* newName) {
	strcpy(name, newName);
	//TODO: change title of window
}

const char* getName() { return name; }

}
