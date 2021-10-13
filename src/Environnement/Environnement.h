#pragma once

#include "nodeGraph/nodeGraph.h"
#include <tinyxml2.h>

class EtherCatDevice;
class Machine;

namespace Environnement{
	
	extern NodeGraph nodeGraph;

	std::vector<std::shared_ptr<EtherCatDevice>>& getEtherCatDevices();
	std::vector<std::shared_ptr<Machine>>& getMachines();

	void addNode(std::shared_ptr<Node> node);
	void removeNode(std::shared_ptr<Node> node);

	bool save(const char* filePath);
	bool load(const char* filePath);

	void setName(const char*);
	const char* getName();

};