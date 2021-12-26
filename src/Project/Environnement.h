#pragma once

class NodeGraph;
class Node;
class EtherCatDevice;
class Machine;
class Plot;

namespace Environnement{

	//project interface
	void createNew();
	bool save(const char* filePath);
	bool load(const char* filePath);
	void setName(const char*);
	const char* getName();

	//main environnement controls
	void setSimulation(bool sim);
	void start();
	void stop();
	void enableAllMachines();
	void disableAllMachines();

	//environnement status
	bool isReady();
	bool isStarting();
	bool isRunning();
	bool isSimulating();
	bool areAllMachinesEnabled();
	bool areNoMachinesEnabled();

	//update method driven by fieldbus
	void update();

	//nodegraph interface
	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices();
	std::vector<std::shared_ptr<Machine>>& getMachines();

	void addNode(std::shared_ptr<Node> node);
	void removeNode(std::shared_ptr<Node> node);

	extern std::shared_ptr<NodeGraph> nodeGraph;
	extern std::shared_ptr<Machine> selectedMachine;
	extern std::shared_ptr<EtherCatDevice> selectedEtherCatDevice;
};
