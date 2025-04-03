#pragma once

class NodeGraph;
class Node;
class EtherCatDevice;
class Machine;
class Plot;
class NetworkDevice;

class Script;
class Command;

class ManualControlChannel;

#include "Legato/Editor/LegatoFile.h"

class EnvironnementFile : public Legato::File{
	COMPONENT_IMPLEMENTATION(EnvironnementFile);
public:
	
};

namespace Environnement{

	//project interface
	void createNew();
	bool save(const char* filePath);
	bool load(const char* filePath);

	void initialize();
	void terminate();

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
	long long int getTime_nanoseconds();
	double getDeltaTime_seconds();
	long long int getDeltaTime_nanoseconds();

	//update method driven by fieldbus
	void updateEtherCatHardware();

	//nodegraph interface
	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices();
	std::vector<std::shared_ptr<Node>>& getEtherCatDeviceNodes();
	std::vector<std::shared_ptr<Machine>>& getMachines();
	std::vector<std::shared_ptr<NetworkDevice>>& getNetworkDevices();

	void addNode(std::shared_ptr<Node> node);
	void removeNode(std::shared_ptr<Node> node);

	//bool isEditorLocked();
	//void lockEditor();
	//void unlockEditor();
	//bool checkEditorPassword(const char* password);

/*
	std::shared_ptr<ManualControlChannel> getManualControlChannel();
	void enableManualControlOfMachineIndex(int index);
	void disableManualControlOfMachineIndex(int index);
	void toggleManualControlOfMachineIndex(int index);
	bool isManualControlEnabledForMachineIndex(int index);
*/

	extern std::shared_ptr<ManualControlChannel> controlChannel1;
	extern std::shared_ptr<ManualControlChannel> controlChannel2;

	std::recursive_mutex* getMutex();

	bool canHomeAllMachines();
	void homeAllMachines();

};
