#pragma once

#include "nodeGraph/nodeGraph.h"

class EtherCatSlave;

class Environnement {
public:

	static bool add(std::shared_ptr<EtherCatSlave> slave);
	static bool hasEtherCatSlave(std::shared_ptr<EtherCatSlave> slave);
	static std::shared_ptr<EtherCatSlave> getMatchingEtherCatSlave(std::shared_ptr<EtherCatSlave>);
	static void setAllEtherCatSlavesOffline();

	static NodeGraph nodeGraph;
};