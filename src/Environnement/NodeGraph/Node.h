#pragma once

//we include NodePin.h so classes can be derived by including the Node.h file
#include "NodePin.h"
#include "NodeLink.h"

#include "Legato/Editor/Component.h"
#include "Legato/Editor/ListComponent.h"

#define DEFINE_NODE(className, type, category) \
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(className)\
	public:\
	virtual std::string& getClassName() override {\
		static std::string className = #className;\
		return className;\
	}\
	virtual std::string getNodeCategory() override { return category; }\
	virtual Node::Type getType() override { return type; }\

namespace tinyxml2 { class XMLElement; }

class NodeGraph;

class Node : public virtual Legato::Component{
	
public:
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override;
	
	bool loadPins();
	virtual bool loadAfterPinConnection() { return true; }
	
public:

	enum class Type {
		IODEVICE,
		PROCESSOR,
		CLOCK,
		CONTAINER
	};
	
	virtual Type getType() = 0;
	virtual std::string& getClassName() = 0;
	virtual std::string getNodeCategory() = 0;

	//pin handling
	void addNodePin(std::shared_ptr<NodePin> d);
	void removeNodePin(std::shared_ptr<NodePin> d);
	std::vector<std::shared_ptr<NodePin>>& getInputPins() { return inputPins->getEntries(); }
	std::vector<std::shared_ptr<NodePin>>& getOutputPins() { return outputPins->getEntries(); }
	bool hasInputs() { return !inputPins->getEntries().empty(); }
	bool hasOutputs() { return !outputPins->getEntries().empty(); }
	void updatePin(std::shared_ptr<NodePin> pin){ onPinUpdate(pin); }
	virtual void onPinUpdate(std::shared_ptr<NodePin> pin){}
	virtual void onPinConnection(std::shared_ptr<NodePin> pin){}
	virtual void onPinDisconnection(std::shared_ptr<NodePin> pin){}
	virtual void onAddToNodeGraph(){}
	virtual void onRemoveFromNodeGraph(){}

	//nodegraph
	int getUniqueID() { return uniqueID; }
	glm::vec2 getNodeGraphPosition();
	virtual void restoreSavedPosition();
	std::shared_ptr<NodeGraph> nodeGraph = nullptr;
	
	//processing
	virtual bool prepareProcess(){ return true; }
	virtual void inputProcess(){}
	virtual void outputProcess(){}
	
	bool wasProcessed() { return b_wasProcessed; }
	
	virtual std::vector<std::shared_ptr<NodePin>> getUpdatedPinsAfterInputProcess(){ return outputPins->getEntries(); }
	virtual std::vector<std::shared_ptr<NodePin>> getUpdatedPinsAfterOutputProcess(){ return inputPins->getEntries(); }
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

	int uniqueID = -1;

	//std::vector<std::shared_ptr<NodePin>> nodeInputPins;
	//std::vector<std::shared_ptr<NodePin>> nodeOutputPins;
	
	std::shared_ptr<Legato::ListComponent<NodePin>> inputPins;
	std::shared_ptr<Legato::ListComponent<NodePin>> outputPins;

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
	{Node::Type::CONTAINER, "Container",	"Container"}\

DEFINE_ENUMERATOR(Node::Type, NodeTypeStrings)
