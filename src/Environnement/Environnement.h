#pragma once

#include "nodeGraph/nodeGraph.h"
#include <tinyxml2.h>

class EtherCatSlave;

namespace Environnement{
	
	bool hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave);
	std::shared_ptr<EtherCatSlave> getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave>);
	void setAllEtherCatSlavesOffline();

	extern NodeGraph nodeGraph;

	bool save(const char* filePath);
	bool load(const char* filePath);

	void setName(const char*);
	const char* getName();

};