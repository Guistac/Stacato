#pragma once

//we include NodePin.h so classes can be derived by including the Node.h file
#include "NodePin.h"
#include "NodeLink.h"

#define DEFINE_NODE(className, nodeName, saveName, type, category) \
	virtual const char* getSaveName() { return saveName; }\
	virtual const char* getNodeCategory() { return category; }\
	className(){ setName(nodeName); }\
	virtual Node::Type getType() { return type; }\
	virtual std::shared_ptr<Node> getNewInstance(){\
		std::shared_ptr<Node> newInstance = std::make_shared<className>();\
		newInstance->initialize();\
		return newInstance;\
	}\
	virtual void initialize();

class NodeGraph;
namespace tinyxml2 { class XMLElement; }

class Node : public std::enable_shared_from_this<Node>{
public:

	enum class Type {
		IODEVICE,
		PROCESSOR,
		CLOCK,
		AXIS,
		MACHINE,
		CONTAINER
	};
	
	virtual Type getType() = 0;
	virtual const char* getSaveName() = 0;
	virtual const char* getNodeCategory() = 0;

	virtual std::shared_ptr<Node> getNewInstance() = 0;
	virtual void initialize() = 0;
	
	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	char* getNameBuffer(int& bufferSize) {
		bufferSize = 128;
		return name;
	}

	void addIoData(std::shared_ptr<NodePin> d);
	void removeIoData(std::shared_ptr<NodePin> d);
	std::vector<std::shared_ptr<NodePin>>& getNodeInputData() { return nodeInputData; }
	std::vector<std::shared_ptr<NodePin>>& getNodeOutputData() { return nodeOutputData; }
	bool hasInputs() { return !nodeInputData.empty(); }
	bool hasOutputs() { return !nodeOutputData.empty(); }

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
	friend class NodePin;
	friend class NodeLink;

	char name[128];

	NodeGraph* parentNodeGraph = nullptr;
	bool b_isInNodeGraph = false;
	int uniqueID = -1;

	std::vector<std::shared_ptr<NodePin>> nodeInputData;
	std::vector<std::shared_ptr<NodePin>> nodeOutputData;

	//processing flags
	bool b_wasProcessed = false;
	bool b_circularDependencyFlag = false;

	//split node status
	bool b_isSplit = false;
	bool b_wasSplit = false;

	glm::vec2 savedPosition;
	glm::vec2 savedSplitPosition;
};



struct NodeType {
	Node::Type type;
	const char displayName[64];
	const char saveName[64];
};
NodeType* getNodeType(Node::Type t);
NodeType* getNodeType(const char* saveName);
