#pragma once

class NodeGraph;
class Node;
class EtherCatDevice;
class Machine;
class Plot;
class NetworkDevice;

class Script;

namespace Environnement{

	//project interface
	void createNew();
	bool save(const char* filePath);
	bool load(const char* filePath);

	void initialize();
	void terminate();

	void open();
	void close();

	//project text information
	void setName(const char* _name);
	const char* getName();
	void updateName();
	const char* getNotes();
	void setNotes(const char* _notes);

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

	//environnement timing
	double getTime_seconds();
	long long int getTime_nanosecond();
	double getDeltaTime_seconds();
	long long int getDeltaTime_nanoseconds();

	//update method driven by fieldbus
	void updateEtherCatHardware();

	//nodegraph interface
	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices();
	std::vector<std::shared_ptr<Machine>>& getMachines();
	std::vector<std::shared_ptr<NetworkDevice>>& getNetworkDevices();

	void addNode(std::shared_ptr<Node> node);
	void removeNode(std::shared_ptr<Node> node);

	extern std::shared_ptr<Machine> selectedMachine;
	extern std::shared_ptr<EtherCatDevice> selectedEtherCatDevice;

	bool isEditorLocked();
	bool isEditorUnlockRequested();
	void lockEditor();
	void requestEditorUnlock();
	void confirmEditorUnlock();
	bool checkEditorPassword(const char* password);
};
