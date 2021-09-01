#include <pch.h>

#include "Environnement.h"

std::vector<std::shared_ptr<EtherCatSlave>> Environnement::etherCatSlaves;

NodeGraph Environnement::nodeGraph;
