#pragma once

class Node;
class EtherCatDevice;

namespace NodeFactory {

	struct NodeGroup {
		char name[128];
		std::vector<Node*> nodes;
	};

	void load();
	void loadEtherCatNodes(std::vector<Node*>& nodeList);
	void loadMotionNodes(std::vector<Node*>& nodeList);
	void loadNetworkNodes(std::vector<Node*>& nodeList);
	void loadProcessorNodes(std::vector<Node*>& nodeList);

	std::shared_ptr<Node> getNodeBySaveName(const char* saveName);

	//EtherCAT
	std::shared_ptr<EtherCatDevice> getDeviceByEtherCatName(const char* etherCatName);
	std::shared_ptr<EtherCatDevice> getDeviceByIdCodes(uint32_t manufacturerCode, uint32_t identificationCode);
	const std::vector<NodeGroup>& getEtherCatDevicesByManufacturer();
	const std::vector<NodeGroup>& getEtherCatDevicesByCategory();

	//Motion
	std::vector<Node*>& getAllAxisNodes();
	std::vector<NodeGroup>& getMachinesByCategory();
	std::vector<Node*>& getAllSafetyNodes();
	std::vector<Node*>& getAllMotionUtilityNodes();

	//Network IO
	std::vector<Node*>& getAllNetworkNodes();

	//Processors
	std::vector<NodeGroup>& getProcessorNodesByCategory();
}
