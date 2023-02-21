#pragma once

//we include NodePin.h so classes can be derived by including the Node.h file
#include "NodePin.h"
#include "NodeLink.h"

#define DEFINE_NODE(className, nodeName, saveName, type, category) \
	virtual const char* getSaveName() override { return saveName; }\
	virtual const char* getNodeCategory() override { return category; }\
	className(){ setName(nodeName); }\
	virtual Node::Type getType() override { return type; }\
	virtual std::shared_ptr<Node> getNewInstance() override {\
		std::shared_ptr<Node> newInstance = std::make_shared<className>();\
		newInstance->initialize();\
		return newInstance;\
	}\
	virtual void initialize() override;\

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
	virtual void onAddToNodeGraph(){}
	virtual void onRemoveFromNodeGraph(){}

	//nodegraph
	int getUniqueID() { return uniqueID; }
	bool isInNodeGraph() { return b_isInNodeGraph; }
	glm::vec2 getNodeGraphPosition();
	virtual void restoreSavedPosition();
	
	//processing
	virtual bool prepareProcess(){ return true; }
	virtual void inputProcess(){}
	virtual void outputProcess(){}
	
	bool wasProcessed() { return b_wasProcessed; }
	
	virtual std::vector<std::shared_ptr<NodePin>> getUpdatedPinsAfterInputProcess(){ return nodeOutputPins; }
	virtual std::vector<std::shared_ptr<NodePin>> getUpdatedPinsAfterOutputProcess(){ return nodeInputPins; }
	virtual bool needsOutputProcess(){ return false; }
	
	//—————————— INPUT VS OUTPUT PROCESSES ——————————
	//each node has an input process and can have an output process
	//the input process gets triggered by the left input pins, and generally propagates updates through the right output pins
	//the output process gets triggered by the right output pins, and generally propagates updates through the left input pins
	//while regular pins can only trigger the input process
	//bidirectional pins can trigger input and output processes
	//if a node has no bidirectional output pin, it does not have an output process
	//if a node has no bidirectional intput pin, it cannot trigger another nodes output process
	//each nodes gets to decice which pins will propagate updates after each process
	//for the input process, all output pins generally trigger the input process on connected nodes
	//for the output process, all bidirectional input pins generally trigger the output process on connected nodes
	//but as an exeption to the rule: a node could decide that its output process could update output pins and cause input processes to be executed downstream
	
	bool areAllLinkedInputNodesProcessed();
	bool areAllLinkedBidirectionalOutputNodesProcessed();

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
