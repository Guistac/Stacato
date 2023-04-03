#pragma once

#include "Environnement/NodeGraph/DeviceNode.h"
#include "NodePin.h"
#include "NodeLink.h"

#include "Legato/Editor/Component.h"

namespace tinyxml2 { class XMLElement; }

struct NodeGraphProcess{
   enum class Direction{
	   INPUT_PROCESS,
	   OUTPUT_PROCESS
   };
   struct Instruction{
	   std::shared_ptr<Node> processedNode;
	   Direction processType;
	   static std::shared_ptr<Instruction> make(std::shared_ptr<Node> node, Direction direction){
		   auto instruction = std::make_shared<Instruction>();
		   instruction->processedNode = node;
		   instruction->processType = direction;
		   return instruction;
	   }
   };
   std::vector<std::shared_ptr<Instruction>> inputProcessInstructions;
   std::vector<std::shared_ptr<Instruction>> outputProcessInstructions;
   void log();
};

class NodeGraph : public Legato::Component{
	
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(NodeGraph)
 
	protected:
	 
	 virtual bool onSerialization() override {
		 Component::onSerialization();
		 return true;
	 }
	 
	 virtual bool onDeserialization() override {
		 Component::onDeserialization();
		 return true;
	 }
	 
	 virtual void onConstruction() override {
		 Component::onConstruction();
	 }
	 
	 virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		 Component::onCopyFrom(source);
	 }

public:
	
	std::vector<std::shared_ptr<Node>>& getNodes() { return nodes; }
	std::vector<std::shared_ptr<NodePin>>& getPins() { return pins; }
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return links; }
	std::vector<std::shared_ptr<Node>>& getSelectedNodes(){ return selectedNodes; }
	std::vector<std::shared_ptr<NodeLink>>& getSelectedLinks(){ return selectedLinks; }
	
	int getNewUniqueID(){
		uniqueIdCounter++;
		return uniqueIdCounter;
	}

	void startCountingUniqueIDsFrom(int largestUniqueID){
		uniqueIdCounter = largestUniqueID + 1;
	}

	void registerUniqueID(int id){
		if(id > uniqueIdCounter) uniqueIdCounter = id + 1;
	}
	
	void addNode(std::shared_ptr<Node>);
	void removeNode(std::shared_ptr<Node>);

	std::shared_ptr<Node> getNode(int uniqueID);
	std::shared_ptr<NodePin> getPin(int uniqueID);
	std::shared_ptr<NodeLink> getLink(int uniqueID);
	
	void setNodeAddCallback(std::function<void(std::shared_ptr<Node>)> cb){ nodeAddCallback = cb; }
	void setNodeRemoveCallback(std::function<void(std::shared_ptr<Node>)> cb){ nodeRemoveCallback = cb; }
	
private:
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<NodePin>> pins;
	std::vector<std::shared_ptr<NodeLink>> links;
	std::vector<std::shared_ptr<Node>> selectedNodes;
	std::vector<std::shared_ptr<NodeLink>> selectedLinks;
	
	std::function<void(std::shared_ptr<Node>)> nodeAddCallback;
	std::function<void(std::shared_ptr<Node>)> nodeRemoveCallback;
	
	//counter to add new nodes, pins and links
	//all items are odd numbers except for split node counterparts which are the an even number above the main node ID
	int uniqueIdCounter = 1;
	
	
	
public:

	std::shared_ptr<NodeGraphProcess> compileProcess(std::vector<std::shared_ptr<Node>>& startNodes);
	void executeInputProcess(std::shared_ptr<NodeGraphProcess> processProgram);
	void executeOutputProcess(std::shared_ptr<NodeGraphProcess> processProgram);

	bool load(tinyxml2::XMLElement* xml);
	bool save(tinyxml2::XMLElement* xml);

	void editorGui();
	void centerView();
	void showFlow();
	
private:
	
	std::vector<std::shared_ptr<NodeGraphProcess::Instruction>> compileOrderedInstructions(std::vector<std::shared_ptr<Node>>& startNodes, NodeGraphProcess::Direction processDirection);
	void executeInstructions(std::vector<std::shared_ptr<NodeGraphProcess::Instruction>>& instructions);
};


