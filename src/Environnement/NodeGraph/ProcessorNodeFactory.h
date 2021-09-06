#pragma once

#include "Nodes/ClockNode.h"
#include "Nodes/DisplayNode.h"

namespace ProcessorNodeFactory {

	std::shared_ptr<ProcessorNode> getProcessorNodeByName(const char* name);

	std::vector<ProcessorNode*>& getDeviceList();

}