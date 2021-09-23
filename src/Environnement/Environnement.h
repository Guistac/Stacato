#pragma once

#include "nodeGraph/nodeGraph.h"
#include <tinyxml2.h>

class EtherCatSlave;

namespace Environnement{
	
	std::vector<std::shared_ptr<EtherCatSlave>> getEtherCatSlaves();

	extern NodeGraph nodeGraph;

	bool save(const char* filePath);
	bool load(const char* filePath);

	void setName(const char*);
	const char* getName();

};