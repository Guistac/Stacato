#pragma once

#include "nodeGraph/nodeGraph.h"
#include <tinyxml2.h>

class EtherCatDevice;
class Machine;

namespace Environnement{
	
	std::vector<std::shared_ptr<EtherCatDevice>> getEtherCatDevices();
	std::vector<std::shared_ptr<Machine>> getMachines();

	extern NodeGraph nodeGraph;

	bool save(const char* filePath);
	bool load(const char* filePath);

	void setName(const char*);
	const char* getName();

};