#pragma once

//we include ioData.h so classes can be derived by including the ioNode.h file
#include "ioData.h"
#include "ioLink.h"

#define DEFINE_PROCESSOR_NODE(nodeName, className, category)	public:																							\
																virtual const char* getNodeName() { return nodeName; }											\
																virtual const char* getNodeCategory() { return category; }										\
																className(){ setName(nodeName); }																\
																virtual NodeType getType() { return NodeType::PROCESSOR; }										\
																virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

#define DEFINE_AXIS_NODE(nodeName, className)	public:																							\
												virtual const char* getNodeName() { return nodeName; }											\
												virtual const char* getNodeCategory() { return "Axis"; }										\
												className(){ setName(nodeName); }																\
												virtual NodeType getType() { return NodeType::AXIS; }											\
												virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

#define	DEFINE_CLOCK_NODE(nodeName, className)	public:																							\
												virtual const char* getNodeName() { return nodeName; }											\
												virtual const char* getNodeCategory() { return "Time"; }										\
												className(){ setName(nodeName); }																\
												virtual NodeType getType() { return NodeType::CLOCK; }											\
												virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

#define DEFINE_CONTAINER_NODE(nodeName, className, category)	public:																							\
																virtual const char * getNodeName() { return nodeName; }											\
																virtual const char* getNodeCategory() { return category; }										\
																className(){ setName(nodeName); }																\
																virtual NodeType getType() { return NodeType::CONTAINER; }										\
																virtual std::shared_ptr<ioNode> getNewNodeInstance() { return std::make_shared<className>(); }	\

class NodeGraph;

enum NodeType {
	IODEVICE,
	PROCESSOR,
	CLOCK,
	AXIS,
	CONTAINER
};

namespace tinyxml2 { class XMLElement; }

class ioNode {
public:

	virtual NodeType getType() = 0;
	virtual const char* getNodeName() = 0;
	virtual const char* getNodeCategory() = 0;
	virtual std::shared_ptr<ioNode> getNewNodeInstance() = 0;

	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }

	const char* getTypeString() {
		switch (getType()) {
			case IODEVICE: return "IODEVICE";
			case PROCESSOR: return "PROCESSOR";
			case CLOCK: return "CLOCK";
			case AXIS: return "AXIS";
			case CONTAINER: return "CONTAINER";
			default: return "";
		}
	}

	void addIoData(std::shared_ptr<ioData> d);
	void removeIoData(std::shared_ptr<ioData> d);
	std::vector<std::shared_ptr<ioData>>& getNodeInputData() { return nodeInputData; }
	std::vector<std::shared_ptr<ioData>>& getNodeOutputData() { return nodeOutputData; }
	bool hasInputs() { return !nodeInputData.empty(); }
	bool hasOutputs() { return !nodeOutputData.empty(); }
	virtual void assignIoData() = 0;

	int getUniqueID() { return uniqueID; }
	bool isInNodeGraph() { return b_isInNodeGraph; }

	virtual void process() = 0;
	bool wasProcessed() { return b_wasProcessed; }
	bool areAllLinkedInputNodesProcessed();

	//GUI STUFF

	virtual void nodeGui();
	virtual void propertiesGui(); //defined in nodegui.cpp
	virtual void nodeSpecificGui() {}

	glm::vec2 getNodeGraphPosition();
	void restoreSavedPosition();

	float getTitleWidth();
	void titleGui();

	//for split nodes
	bool isSplit() { return b_isSplit; }
	void getSplitNodeGraphPosition(glm::vec2& inputNode, glm::vec2& outputNode);

	//SAVING AND LOADING

	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }

private:

	friend class NodeGraph;
	friend class ioData;
	friend class ioLink;

	char name[128];

	NodeGraph* parentNodeGraph = nullptr;
	bool b_isInNodeGraph = false;
	int uniqueID = -1;

	std::vector<std::shared_ptr<ioData>> nodeInputData;
	std::vector<std::shared_ptr<ioData>> nodeOutputData;

	//processing flags
	bool b_wasProcessed = false;
	bool b_circularDependencyFlag = false;

	//split node status
	bool b_isSplit = false;
	bool b_wasSplit = false;

	glm::vec2 savedPosition;
	glm::vec2 savedSplitPosition;
};