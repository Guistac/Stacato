#include <pch.h>
#include "NodeGraph.h"

void NodeGraph::onConstruction() {
	Component::onConstruction();
	context = ax::NodeEditor::CreateEditor();
	
	nodeList = Legato::ListComponent<Node>::createInstance();
	nodeList->setSaveString("NodeList");
	nodeList->setEntrySaveString("Node");
	//nodelist entry constructor is set by setNodeConstructor();
	
	linkList = Legato::ListComponent<NodeLink>::createInstance();
	linkList->setSaveString("LinkList");
	linkList->setEntrySaveString("Link");
	linkList->setEntryConstructor([](Legato::Serializable& abstractEntry){ return NodeLink::createInstance(); });
	
}

void NodeGraph::onCopyFrom(std::shared_ptr<PrototypeBase> source) {
	Component::onCopyFrom(source);
}


bool NodeGraph::onSerialization(){
	Component::onSerialization(); //we don't need a save name
	
	//this needs to be called to satisfy the node editor library
	//if we never displayed the node editor, this means we never submitted any nodes to the library
	//and we can't retrieve information such as node positions from the library
	//we submit all node once here before saving so the library is happy
	//we can proceed as if the node editor had been displayed all along
	//if the node editor was displayed once this is useless but we submit the draw call anyways since it is offscreen
	ImGui::SetNextWindowPos(ImVec2(10000, 10000));
	ImGui::Begin("OffscreenNodeEditorFor1Frame", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);
	editorGui(ImVec2(0,0));
	ImGui::End();
	
	nodeList->serializeIntoParent(this);
	linkList->serializeIntoParent(this);
	
	return true;
}

bool NodeGraph::onDeserialization(){
	//Component::onDeserialization();
	
	std::shared_ptr<Legato::ListComponent<Node>> temporaryNodeList = Legato::ListComponent<Node>::createInstance();
	temporaryNodeList->setSaveString("NodeList");
	temporaryNodeList->setEntrySaveString("Node");
	temporaryNodeList->setEntryConstructor(nodeConstructor);
	
	//load all nodes
	temporaryNodeList->deserializeFromParent(this);
	
	//load all node pin
	for(auto node : temporaryNodeList->getEntries()){
		node->loadPins();
	}
	
	//add all nodes to nodegraph
	for(auto node : temporaryNodeList->getEntries()){
		addNode(node);
	}
	
	//load all links into a temporary list
	std::shared_ptr<Legato::ListComponent<NodeLink>> abstractLinkList = Legato::ListComponent<NodeLink>::createInstance();
	abstractLinkList->setSaveString("Links");
	abstractLinkList->setEntrySaveString("Link");
	abstractLinkList->setEntryConstructor([](Legato::Serializable& abstractEntry){ return NodeLink::createInstance(); });
	abstractLinkList->deserializeFromParent(this);

	//connect all links
	for(auto abstractLink : abstractLinkList->getEntries()){
		auto inputPin = abstractLink->getInputPin();
		auto outputPin = abstractLink->getInputPin();
		inputPin->connectTo(outputPin);
	}

	//let all nodes load data that is only relevant after all links are connected
	for(auto node : nodeList->getEntries()){
		node->loadAfterPinConnection();
	}
	
	/*
	 int largestUniqueID = 0;
	 if (nodeUniqueID > largestUniqueID) largestUniqueID = nodeUniqueID;
	 startCountingUniqueIDsFrom(largestUniqueID);
	 Logger::trace("Largest unique ID is {}", largestUniqueID);
	 Logger::info("Successfully loaded Node Graph");
	 */
	
	return true;
}


void NodeGraph::addNode(std::shared_ptr<Node> newNode) {
	if(newNode->uniqueID == -1) newNode->uniqueID = getNewUniqueID();
	newNode->nodeGraph = std::static_pointer_cast<NodeGraph>(shared_from_this());
	nodeList->addEntry(newNode);
	for (std::shared_ptr<NodePin> pin : newNode->getInputPins()) {
		pin->uniqueID = getNewUniqueID();
		pin->parentNode = newNode;
	}
	for (std::shared_ptr<NodePin> pin : newNode->getOutputPins()) {
		pin->uniqueID = getNewUniqueID();
		pin->parentNode = newNode;
	}
	newNode->onAddToNodeGraph();
	nodeAddCallback(newNode);
}

void NodeGraph::removeNode(std::shared_ptr<Node> removedNode) {
	removedNode->nodeGraph = nullptr;
	for (auto pin : removedNode->getInputPins()) {
		for (std::shared_ptr<NodeLink> link : pin->nodeLinks) link->disconnect();
	}
	for (auto pin : removedNode->getOutputPins()) {
		for (std::shared_ptr<NodeLink> link : pin->nodeLinks) link->disconnect();
	}
	nodeList->removeEntry(removedNode);

	for (int i = (int)selectedNodes.size() - 1; i >= 0; i--) {
		if (selectedNodes[i] == removedNode) {
			selectedNodes.erase(selectedNodes.begin() + i);
		}
	}
	removedNode->onRemoveFromNodeGraph();
	nodeRemoveCallback(removedNode);
}

void NodeGraph::addLink(std::shared_ptr<NodeLink> link){
	linkList->addEntry(link);
}

void NodeGraph::removeLink(std::shared_ptr<NodeLink> link){
	linkList->removeEntry(link);
}

std::shared_ptr<Node> NodeGraph::getNode(int Id) {
	for(auto node : nodeList->getEntries()){
		if (Id == node->uniqueID) return node;
	}
	return nullptr;
}


std::shared_ptr<NodePin> NodeGraph::getPin(int Id) {
	for(std::shared_ptr<Node> node : nodeList->getEntries()){
		for(auto pin : node->getInputPins()){
			if (Id == pin->uniqueID) return pin;
		}
		for(auto pin : node->getOutputPins()){
			if (Id == pin->uniqueID) return pin;
		}
	}
	return nullptr;
}

std::shared_ptr<NodeLink> NodeGraph::getLink(int Id) {
	for (std::shared_ptr<NodeLink> link : linkList->getEntries()) {
		if (Id == link->uniqueID) return link;
	}
	return nullptr;
}

