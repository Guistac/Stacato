#pragma once

#include "Legato/Editor/FileComponent.h"

namespace AnimationSystem{ class AnimatableRegistry; };

class NodeGraph;
class Node;
class EtherCatDevice;
class Machine;
class Plot;
class NetworkDevice;

class Script;
class Command;


class EnvironnementObject : public Legato::FileComponent{
public:
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(EnvironnementObject)
 
public:
	
	std::shared_ptr<NodeGraph> getNodeGraph(){ return nodeGraph; }
	std::shared_ptr<AnimationSystem::AnimatableRegistry> getAnimatableRegistry(){ return animatableRegistry; }
	
	protected:
	 
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	
public:

	void initialize();
	void terminate();

	//main environnement controls
	void start();
	void stop();
	void enableAllMachines();
	void disableAllMachines();

	//environnement status
	bool isReady();
	bool isStarting();
	bool isRunning();
	//bool isSimulating();
	bool areAllMachinesEnabled();
	bool areNoMachinesEnabled();

	//update method driven by fieldbus
	void updateEtherCatHardware();

	//nodegraph interface
	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices();
	std::vector<std::shared_ptr<Node>>& getEtherCatDeviceNodes();
	std::vector<std::shared_ptr<Machine>>& getMachines();
	std::vector<std::shared_ptr<NetworkDevice>>& getNetworkDevices();

	void addNode(std::shared_ptr<Node> node);
	void removeNode(std::shared_ptr<Node> node);

private:
	
	std::shared_ptr<NodeGraph> nodeGraph = nullptr;
	std::vector<std::shared_ptr<Machine>> machines = {};
	std::vector<std::shared_ptr<EtherCatDevice>> ethercatDevices = {};
	std::vector<std::shared_ptr<NetworkDevice>> networkDevices = {};
	
	std::shared_ptr<AnimationSystem::AnimatableRegistry> animatableRegistry = nullptr;
	
};
