#pragma once

class NodeGraph;
class Node;
class EtherCatDevice;
class Machine;
class Plot;

namespace Environnement{

	extern std::shared_ptr<NodeGraph> nodeGraph;

	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices();
	std::vector<std::shared_ptr<Machine>>& getMachines();

	void enableAllMachines();
	void disableAllMachines();
	bool areAllMachinesEnabled();
	bool areNoMachinesEnabled();

	void addNode(std::shared_ptr<Node> node);
	void removeNode(std::shared_ptr<Node> node);

	void createNew();
	bool save(const char* filePath);
	bool load(const char* filePath);

	void setName(const char*);
	const char* getName();

	void gui();
	void machineManagerGui();
	void deviceManagerGui();
	void editorGui();
};