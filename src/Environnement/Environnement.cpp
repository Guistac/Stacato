#include <pch.h>

#include "Environnement.h"

#include "Environnement/NodeGraph/NodeGraph.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Machine/Machine.h"

#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include "Animation/Playback/Playback.h"

#include "Networking/Network.h"
#include "Networking/NetworkDevice.h"

#include "Visualizer/Visualizer.h"

#include "Gui/Environnement/EnvironnementGui.h"

#include "Project/Editor/CommandHistory.h"

#include "EnvironnementScript.h"

#include "Gui/Fieldbus/EtherCatGui.h"

#include "Animation/ManualControlChannel.h"

namespace Environnement {

	std::shared_ptr<ManualControlChannel> manualControlChannel = nullptr;

	std::shared_ptr<ManualControlChannel> getManualControlChannel(){ return manualControlChannel; }

	void enableManualControlOfMachineIndex(int index){
		if(index < 0 || index >= getMachines().size()) return;
		auto machine = getMachines()[index];
		if(machine->getAnimatables().empty()) return;
		if(auto activePreset = manualControlChannel->getActiveChannelPreset()){
			auto animatable = machine->getAnimatables().front();
			activePreset->addAnimatable(animatable);
		}
	}
	void disableManualControlOfMachineIndex(int index){
		if(index < 0 || index >= getMachines().size()) return;
		auto machine = getMachines()[index];
		if(machine->getAnimatables().empty()) return;
		if(auto activePreset = manualControlChannel->getActiveChannelPreset()){
			auto animatable = machine->getAnimatables().front();
			activePreset->removeAnimatable(animatable);
		}
	}
	void toggleManualControlOfMachineIndex(int index){
		if(index < 0 || index >= getMachines().size()) return;
		auto machine = getMachines()[index];
		if(machine->getAnimatables().empty()) return;
		if(auto activePreset = manualControlChannel->getActiveChannelPreset()){
			auto animatable = machine->getAnimatables().front();
			if(activePreset->hasAnimatable(animatable)) activePreset->removeAnimatable(animatable);
			else activePreset->addAnimatable(animatable);
		}
	}
	bool isManualControlEnabledForMachineIndex(int index){
		if(index < 0 || index >= getMachines().size()) return;
		auto machine = getMachines()[index];
		if(machine->getAnimatables().empty()) return;
		if(auto activePreset = manualControlChannel->getActiveChannelPreset()){
			auto animatable = machine->getAnimatables().front();
			return activePreset->hasAnimatable(animatable);
		}
	}






	std::recursive_mutex mutex;
	std::recursive_mutex* getMutex(){ return &mutex; }

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
		Network::init();
		EtherCatFieldbus::initialize();
		manualControlChannel = std::make_shared<ManualControlChannel>();
	}

	void terminate(){
		//if(isRunning()) stop();
		EtherCatFieldbus::terminate();
		Network::terminate();
	}

	void start(){
		if(isStarting() || isRunning()) return;
		if(b_isSimulating) startSimulation();
		else startHardware();
	}

	void stop(){
		if(!isStarting() && !isRunning()) return;
		if(b_isSimulating) stopSimulation();
		else stopHardware();
	}

	bool isReady(){
		if(b_isSimulating) return true;
		else return EtherCatFieldbus::canStart();
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



	//—————————————— SIMULATION —————————————————

	void startSimulation(){
		b_isRunning = true;
		Logger::info("Starting Environnement Simulation");
		
		for(auto node : NodeGraph::getNodes()){
			if(!node->prepareProcess()){
				Logger::error("[Environnement] Could not start environnement : Node {} could not start", node->getName());
				return;
			}
		}
		
		Logger::debug("Compiling: EtherCAT Process Program: ");
		NodeGraph::compileProcess(getEtherCatDeviceNodes())->log();
		
		for(auto machine : getMachines()){
			for(auto animatable : machine->getAnimatables()){
				animatable->clearConstraints();
			}
			machine->addConstraints();
		}
		
		Script::start();
		if(!Script::isRunning()){
			b_isRunning = false;
			Logger::error("Environnement Script could not be started");
			return;
		}
		
		simulationStartTime_seconds = Timing::getProgramTime_seconds();
		simulationStartTime_nanoseconds = Timing::getProgramTime_nanoseconds();
		
		environnementSimulator = std::thread([](){
			pthread_setname_np("Environnement Simulation Thread");
			while(b_isRunning){
				updateSimulation();
				//run simulation at 100Hz and free the cpu core in between processing cycles
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		});
		
		for(auto& networkDevice : getNetworkDevices()) networkDevice->connect();
				
		Environnement::StageVisualizer::start();
		
		Logger::info("Started Environnement Simulation");
	}


	void updateSimulation(){
		mutex.lock();

		//update simulation time
		double currentSimulationTime_seconds = Timing::getProgramTime_seconds() - simulationStartTime_seconds;
		long long int currentSimulationTime_nanoseconds = Timing::getProgramTime_nanoseconds() - simulationStartTime_nanoseconds;
		simulationTimeDelta_seconds = currentSimulationTime_seconds - simulationTime_seconds;
		simulationTimeDelta_nanoseconds = currentSimulationTime_nanoseconds - simulationTime_nanoseconds;
		simulationTime_seconds = currentSimulationTime_seconds;
		simulationTime_nanoseconds = currentSimulationTime_nanoseconds;

		for(auto& machine : getMachines()) machine->simulateInputProcess();

		//auto start = std::chrono::high_resolution_clock::now();
		Script::update();
		//auto end = std::chrono::high_resolution_clock::now();
		//Logger::warn("scrip processing duration {}µs", std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		
		PlaybackManager::update();

		for(auto& machine : getMachines()) machine->simulateOutputProcess();

		mutex.unlock();
		
		if(!Script::isRunning()) {
            Logger::warn("Environnement Script Stopped, Stopping Environnement");
			stop();
		}
	}


	void stopSimulation(){
		std::thread simulationStopper = std::thread([](){
			disableAllMachines();
			
			for(auto& networkDevice : getNetworkDevices()) networkDevice->disconnect();
			b_isRunning = false;
			if(environnementSimulator.joinable()) environnementSimulator.join();
			
            Script::stop();
            
			for(auto machine : getMachines()){
				machine->state = DeviceState::OFFLINE;
			}
			
			Environnement::StageVisualizer::stop();
			
			Logger::info("Stopped Environnement Simulation");
		});
		simulationStopper.detach();
	}


	//—————————————— HARDWARE RUNTIME —————————————————

	std::shared_ptr<NodeGraph::CompiledProcess> ethercatDeviceProcess;

	void startHardware(){
		
		
		
		b_isStarting = true;
		Logger::info("Starting Environnement Hardware");
		
		for(auto machine : getMachines()){
			for(auto animatable : machine->getAnimatables()){
				animatable->clearConstraints();
			}
			machine->addConstraints();
		}
		
		Script::start();
		if(!Script::isRunning()){
			Logger::error("Could not start Environnement Script, stopping environnement");
			b_isStarting = false;
			return;
		}
		
		
		for(auto node : NodeGraph::getNodes()){
			if(!node->prepareProcess()){
				Logger::error("[Environnement] Could not start environnement : Node {} could not start", node->getName());
				b_isStarting = false;
				return;
			}
		}
		
		ethercatDeviceProcess = NodeGraph::compileProcess(getEtherCatDeviceNodes());
		Logger::info("Compiled EtherCAT Process: ");
		ethercatDeviceProcess->log();
		
		EtherCatStartPopup::get()->open();
		EtherCatFieldbus::start();
		std::thread environnementHardwareStarter([](){
			//wait while the fieldbus is starting
			while(EtherCatFieldbus::isStarting()) std::this_thread::sleep_for(std::chrono::milliseconds(20));
			
			//when done starting, check if start succeded
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
		
		Environnement::StageVisualizer::start();
	}

	void updateEtherCatHardware(){
		mutex.lock();
		
		//interpret all slaves input data if operational
		for (auto ethercatDevice : EtherCatFieldbus::getDevices()) if (ethercatDevice->isStateOperational()) ethercatDevice->readInputs();
				
		//read inputs from devices and propagate them into the node graph
		NodeGraph::executeInputProcess(ethercatDeviceProcess);
		
		Script::update();
		
		//increments the playback position of all active manoeuvres
		PlaybackManager::update();
		
		//take nodegraph outputs and propagate them to the devices
		NodeGraph::executeOutputProcess(ethercatDeviceProcess);
		
		//prepare all slaves output data if operational
		for (auto ethercatDevice : EtherCatFieldbus::getDevices()) if (ethercatDevice->isStateOperational()) ethercatDevice->writeOutputs();
		
		mutex.unlock();
		
		if(!Script::isRunning()){
			Logger::warn("Environnement Script Stopped, Stopping Environnement");
			stop();
		}
	}

	void stopHardware(){
		std::thread hardwareStopper = std::thread([](){
			b_isRunning = false;
						
			for(auto& machine : getMachines()) machine->disable();
			EtherCatFieldbus::stop();
			for(auto& networkDevice : getNetworkDevices()) networkDevice->disconnect();
			
			//execute the input process one last time to propagate disconnection of ethercat devices
			NodeGraph::executeInputProcess(ethercatDeviceProcess);
			
			Script::stop();
			Environnement::StageVisualizer::stop();
			
			Logger::info("Stopped Environnement.");
		});
		hardwareStopper.detach();
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



	std::vector<std::shared_ptr<EtherCatDevice>> etherCatDevices;
	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices() { return etherCatDevices; }
	std::vector<std::shared_ptr<Node>> etherCatDeviceNodes;
	std::vector<std::shared_ptr<Node>>& getEtherCatDeviceNodes(){ return etherCatDeviceNodes; }

	std::vector<std::shared_ptr<Machine>> machines;
	std::vector<std::shared_ptr<Machine>>& getMachines() { return machines; }
	
	std::vector<std::shared_ptr<NetworkDevice>> networkDevices;
	std::vector<std::shared_ptr<NetworkDevice>>& getNetworkDevices(){ return networkDevices; }

	std::shared_ptr<Machine> selectedMachine;
	std::shared_ptr<EtherCatDevice> selectedEtherCatDevice;

	void enableAllMachines() {
        if(!Environnement::isRunning()) return;
		for (auto machine : machines) machine->enable();
	}
	void disableAllMachines() {
        if(!Environnement::isRunning()) return;
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
				machines.push_back(std::static_pointer_cast<Machine>(node));
				break;
			case Node::Type::IODEVICE:{
				std::shared_ptr<Device> deviceNode = std::static_pointer_cast<Device>(node);
				switch (deviceNode->getDeviceType()) {
					case Device::Type::ETHERCAT_DEVICE:
						etherCatDevices.push_back(std::static_pointer_cast<EtherCatDevice>(deviceNode));
						etherCatDeviceNodes.push_back(node);
						break;
					case Device::Type::NETWORK_DEVICE:
						networkDevices.push_back(std::static_pointer_cast<NetworkDevice>(deviceNode));
						break;
					case Device::Type::USB_DEVICE:
						break;
				}
			}break;
			default:
				break;
		}
		NodeGraph::addNode(node);
	}

	void removeNode(std::shared_ptr<Node> node){
		switch (node->getType()) {
			case Node::Type::MACHINE:{
				std::shared_ptr<Machine> machineNode = std::static_pointer_cast<Machine>(node);
				for (int i = 0; i < machines.size(); i++) {
					if (machines[i] == machineNode) {
						machines.erase(machines.begin() + i);
						break;
					}
				}
			}break;
			case Node::Type::IODEVICE:{
				std::shared_ptr<Device> deviceNode = std::static_pointer_cast<Device>(node);
				switch (deviceNode->getDeviceType()) {
				case Device::Type::ETHERCAT_DEVICE: {
					std::shared_ptr<EtherCatDevice> etherCatDeviceNode = std::static_pointer_cast<EtherCatDevice>(deviceNode);
					for (int i = 0; i < etherCatDevices.size(); i++) {
						if (etherCatDevices[i] == etherCatDeviceNode) {
							etherCatDevices.erase(etherCatDevices.begin() + i);
							break;
						}
					}
					for (int i = 0; i < etherCatDeviceNodes.size(); i++) {
						if (etherCatDeviceNodes[i] == node) {
							etherCatDeviceNodes.erase(etherCatDeviceNodes.begin() + i);
							break;
						}
					}
				}break;
				case Device::Type::NETWORK_DEVICE: {
					std::shared_ptr<NetworkDevice> networkDeviceNode = std::static_pointer_cast<NetworkDevice>(deviceNode);
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
		NodeGraph::removeNode(node);
	}

	char name[256] = "Default Environnement";
	char notes[65536] = "";

	void setName(const char* newName) {
		strcpy(name, newName);
		updateName();
	}

	void updateName(){
		//ApplicationWindow::setWindowName(name);
	}

	const char* getName() { return name; }

	const char* getNotes() { return notes; }

	void setNotes(const char* _notes){
		sprintf(notes, "%s", _notes);
	}

	void createNew() {
		NodeGraph::reset();
		NodeGraph::Gui::reset();
		getEtherCatDevices().clear();
		getMachines().clear();
		StageVisualizer::reset();
		Environnement::Script::reset();
	}

}
