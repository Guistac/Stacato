#pragma once

//we include NodePin.h so classes can be derived by including the Node.h file
#include "NodeGraph/NodePin.h"
#include "NodeGraph/NodeLink.h"

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
	virtual void initialize();\

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
	
	virtual std::shared_ptr<Node> getNewInstance() = 0;
	virtual void initialize() = 0;
	virtual Type getType() = 0;
	
	virtual const char* getSaveName() = 0;
	virtual const char* getNodeCategory() = 0;
	void setName(const char* n) { strcpy(name, n); }
	const char* getName() { return name; }
	char* getNameBuffer(int& bufferSize) {
		bufferSize = 128;
		return name;
	}

	//pin handling
	void addNodePin(std::shared_ptr<NodePin> d);
	void removeIoData(std::shared_ptr<NodePin> d);
	std::vector<std::shared_ptr<NodePin>>& getInputPins() { return nodeInputPins; }
	std::vector<std::shared_ptr<NodePin>>& getOutputPins() { return nodeOutputPins; }
	bool hasInputs() { return !nodeInputPins.empty(); }
	bool hasOutputs() { return !nodeOutputPins.empty(); }
	void updatePin(std::shared_ptr<NodePin> pin){ onPinUpdate(pin); }
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin){}
	virtual void onPinConnection(std::shared_ptr<NodePin> pin){}
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin){}

	//nodegraph
	int getUniqueID() { return uniqueID; }
	bool isInNodeGraph() { return b_isInNodeGraph; }
	glm::vec2 getNodeGraphPosition();
	void restoreSavedPosition();
	
	//processing
	virtual void process(){}
	bool wasProcessed() { return b_wasProcessed; }
	bool areAllLinkedInputNodesProcessed();

	//gui
	virtual void nodeGui();
	virtual void propertiesGui(); //defined in nodegui.cpp
	virtual void nodeSpecificGui() {}
	float getTitleWidth();
	void titleGui();

	//splitting
	bool isSplit() { return b_isSplit; }
	void getSplitNodeGraphPosition(glm::vec2& inputNode, glm::vec2& outputNode);

	//saving & loading
	virtual bool load(tinyxml2::XMLElement* xml) { return true; }
	virtual bool save(tinyxml2::XMLElement* xml) { return true; }


	
	

	char name[128];

	bool b_isInNodeGraph = false;
	int uniqueID = -1;

	std::vector<std::shared_ptr<NodePin>> nodeInputPins;
	std::vector<std::shared_ptr<NodePin>> nodeOutputPins;

	//processing flags
	bool b_wasProcessed = false;
	bool b_circularDependencyFlag = false;

	//split node status
	bool b_isSplit = false;
	bool b_wasSplit = false;

	glm::vec2 savedPosition;
	glm::vec2 savedSplitPosition;
};



#define NodeTypeStrings \
	{Node::Type::IODEVICE,	"IODevice",		"IODevice"},\
	{Node::Type::PROCESSOR, "Processor",	"Processor"},\
	{Node::Type::CLOCK,		"Clock",		"Clock"},\
	{Node::Type::AXIS,		"Axis",			"Axis"},\
	{Node::Type::MACHINE,	"Machine",		"Machine"},\
	{Node::Type::CONTAINER, "Container",	"Container"}\

DEFINE_ENUMERATOR(Node::Type, NodeTypeStrings)
