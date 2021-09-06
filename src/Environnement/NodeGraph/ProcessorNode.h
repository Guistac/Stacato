#pragma once

#include "ioNode.h"

#define DEFINE_PROCESSOR_NODE(nodeName, className)	public:																\
													virtual const char * getProcessorName() { return nodeName; }		\
													static const char * getProcessorNameStatic() { return nodeName; }	\
													className(){														\
														setName(nodeName);												\
														assignIoData();													\
													}																	\

#define RETURN_PROCESSOR_NODE_IF_MATCHING(name, className) if(strcmp(name, className::getProcessorNameStatic()) == 0) return std::make_shared<className>()

class ProcessorNode : public ioNode {

public:

	virtual NodeType getType() { 
		setName("test");
		return NodeType::PROCESSOR; }

	virtual const char* getProcessorName() { return "Base Processor Node"; }

	virtual void assignIoData() = 0;

};