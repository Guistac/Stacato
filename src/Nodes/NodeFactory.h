#pragma once

class Node;
class EtherCatDevice;

namespace NodeFactory {

	struct NodeGroup {
		std::string name;
		std::vector<std::shared_ptr<Node>> nodes;
	};

	void load();
	void loadEtherCatNodes(std::vector<std::shared_ptr<Node>>& nodeList);
	void loadMotionNodes(std::vector<std::shared_ptr<Node>>& nodeList);
	void loadNetworkNodes(std::vector<std::shared_ptr<Node>>& nodeList);
	void loadProcessorNodes(std::vector<std::shared_ptr<Node>>& nodeList);

	std::shared_ptr<Node> getNodeByClassName(std::string className);

	//EtherCAT
	std::shared_ptr<EtherCatDevice> getEtherCatDeviceByIdCodes(uint32_t manufacturerCode, uint32_t identificationCode);
	const std::vector<NodeGroup>& getEtherCatDevicesByManufacturer();
	const std::vector<NodeGroup>& getEtherCatDevicesByCategory();

	//Motion
	std::vector<std::shared_ptr<Node>>& getAllAxisNodes();
	std::vector<NodeGroup>& getMachinesByCategory();
	std::vector<std::shared_ptr<Node>>& getAllSafetyNodes();

	//Network IO
	std::vector<std::shared_ptr<Node>>& getAllNetworkNodes();

	//Processors
	std::vector<NodeGroup>& getProcessorNodesByCategory();
}
