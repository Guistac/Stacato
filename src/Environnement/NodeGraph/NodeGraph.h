#pragma once

#include "Environnement/NodeGraph/DeviceNode.h"
#include "NodePin.h"
#include "NodeLink.h"

#include "Legato/Editor/ListComponent.h"

#include <imgui.h>
#include <imgui_node_editor.h>

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
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<Prototype> source) override;
	
public:
	
	std::vector<std::shared_ptr<Node>>& getNodes() { return *nodeList.get(); }
	std::vector<std::shared_ptr<NodeLink>>& getLinks() { return *linkList.get();  }
	std::vector<std::shared_ptr<Node>>& getSelectedNodes(){ return selectedNodes; }
	std::vector<std::shared_ptr<NodeLink>>& getSelectedLinks(){ return selectedLinks; }
	
	void addNode(std::shared_ptr<Node>);
	void removeNode(std::shared_ptr<Node>);
	void addLink(std::shared_ptr<NodeLink>);
	void removeLink(std::shared_ptr<NodeLink>);
	
	std::shared_ptr<Node> getNode(int uniqueID);
	std::shared_ptr<NodePin> getPin(int uniqueID);
	std::shared_ptr<NodeLink> getLink(int uniqueID);
	
	void setNodeConstructor(std::function<std::shared_ptr<Node>(Legato::Serializable&)> cst){ nodeConstructor = cst; }
	void setNodeAddCallback(std::function<void(std::shared_ptr<Node>)> cb){ nodeAddCallback = cb; }
	void setNodeRemoveCallback(std::function<void(std::shared_ptr<Node>)> cb){ nodeRemoveCallback = cb; }
	void setNodeEditorContextMenuCallback(std::function<std::shared_ptr<Node>()> cb){ editorContextMenuCallback = cb; }
	void setNodeDragDropTargetCallback(std::function<std::shared_ptr<Node>()> cb){ nodeDragDropTargetCallback = cb; }
	
	int getNewUniqueID(){
		uniqueIdCounter++;
		return uniqueIdCounter;
	}
	
	void registerUniqueID(int id){
		if(id > uniqueIdCounter) uniqueIdCounter = id + 1;
	}
	
	//gui
	void editorGui(ImVec2 size);
	void centerView();
	void showFlow();
	
	~NodeGraph(){ ax::NodeEditor::DestroyEditor(context); }
	
	
	void checkForUniqueIDConflicts();
	
private:
	
	std::shared_ptr<Legato::ListComponent<Node>> nodeList;
	std::shared_ptr<Legato::ListComponent<NodeLink>> linkList;
	
	std::vector<std::shared_ptr<Node>> selectedNodes;
	std::vector<std::shared_ptr<NodeLink>> selectedLinks;
	
	std::function<void(std::shared_ptr<Node>)> nodeAddCallback;
	std::function<void(std::shared_ptr<Node>)> nodeRemoveCallback;
	std::function<std::shared_ptr<Node>()> editorContextMenuCallback;
	std::function<std::shared_ptr<Node>()> nodeDragDropTargetCallback;
	std::function<std::shared_ptr<Node>(Legato::Serializable&)> nodeConstructor;
	
	ax::NodeEditor::EditorContext* context;
	bool b_justLoaded = false;
	
	//counter to add new nodes, pins and links
	//all items are odd numbers except for split node counterparts which are the an even number above the main node ID
	int uniqueIdCounter = 1;
	
public:

	std::shared_ptr<NodeGraphProcess> compileProcess(std::vector<std::shared_ptr<Node>>& startNodes);
	void executeInputProcess(std::shared_ptr<NodeGraphProcess> processProgram);
	void executeOutputProcess(std::shared_ptr<NodeGraphProcess> processProgram);

	
private:
	
	std::vector<std::shared_ptr<NodeGraphProcess::Instruction>> compileOrderedInstructions(std::vector<std::shared_ptr<Node>>& startNodes, NodeGraphProcess::Direction processDirection);
	void executeInstructions(std::vector<std::shared_ptr<NodeGraphProcess::Instruction>>& instructions);
};


