#include <pch.h>

#include "ProcessorNodeFactory.h"

namespace ProcessorNodeFactory {

	std::shared_ptr<ProcessorNode> getProcessorNodeByName(const char* name) {
		RETURN_PROCESSOR_NODE_IF_MATCHING(name, ClockNode);
		RETURN_PROCESSOR_NODE_IF_MATCHING(name, DisplayNode);
		return nullptr;
	}

	std::vector<ProcessorNode*> processorNodeList = {
		//new ClockNode(),
		//new DisplayNode()
	};

	std::vector<ProcessorNode*>& getDeviceList() {
		return processorNodeList;
	}
}