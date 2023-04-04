#include <pch.h>

#include <imgui.h>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>
#include <GLFW/glfw3.h>

#include "Environnement/NodeGraph/NodeGraph.h"

#include "Gui/Assets/Colors.h"



void NodeGraph::editorGui(ImVec2 size){
	
	ax::NodeEditor::SetCurrentEditor(context);
	ax::NodeEditor::Begin("Node Editor", size);
	
	//===== DRAW NODES =====

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2, 0));
	for (auto node : nodes) node->nodeGui();
	ImGui::PopStyleVar(2);

	
	
	//===== DRAW LINKS =====

	for (auto link : links){
		ax::NodeEditor::LinkId linkId = link->getUniqueID();
		ax::NodeEditor::PinId startPinId = link->getInputData()->getUniqueID();
		ax::NodeEditor::PinId endPinId = link->getOutputData()->getUniqueID();
		if(link->inputData->isBidirectional() && link->outputData->isBidirectional()){
			ax::NodeEditor::Link(linkId, startPinId, endPinId, ImColor(1.f, 1.f, .0f, 1.f), 3.0);
		}else{
			ax::NodeEditor::Link(linkId, startPinId, endPinId, ImColor(1.0f, 1.0f, 1.0f), 1.0);
		}
	}

	
	
	bool b_isLocked = false;
	if (!b_isLocked) {

		//===== link creation, returns true if editor want to create new link

		if (ax::NodeEditor::BeginCreate(ImVec4(1.0, 1.0, 1.0, 1.0), 1.0)) {
			ax::NodeEditor::PinId pin1Id, pin2Id;
			if (ax::NodeEditor::QueryNewLink(&pin1Id, &pin2Id)) {
				if (pin1Id && pin2Id) {
					std::shared_ptr<NodePin> pin1 = getPin(pin1Id.Get());
					std::shared_ptr<NodePin> pin2 = getPin(pin2Id.Get());
					if (pin1 && pin2 && pin1->isConnectionValid(pin2)) {
						if (ax::NodeEditor::AcceptNewItem(ImColor(1.0f, 1.0f, 1.0f), 3.0)) {
							auto link = pin1->connectTo(pin2);
							ax::NodeEditor::Link(link->getUniqueID(), pin1Id, pin2Id);
						}
					}
					else ax::NodeEditor::RejectNewItem(ImColor(1.0f, 0.0f, 0.0f), 3.0);
				}
			}
		}
		ax::NodeEditor::EndCreate();

		
		
		
		//===== link and node deletion action =====

		if (ax::NodeEditor::BeginDelete()) {

			ax::NodeEditor::LinkId deletedLinkId;
			while (ax::NodeEditor::QueryDeletedLink(&deletedLinkId)) {
				if (ax::NodeEditor::AcceptDeletedItem()) {
					std::shared_ptr<NodeLink> deletedLink = getLink(deletedLinkId.Get());
					if(deletedLink) deletedLink->disconnect();
				}
			}

			ax::NodeEditor::NodeId deletedNodeId;
			while (ax::NodeEditor::QueryDeletedNode(&deletedNodeId)) {
				std::shared_ptr<Node> deletedNode = getNode(deletedNodeId.Get());
				if (deletedNode && ax::NodeEditor::AcceptDeletedItem()) {
					removeNode(deletedNode);
				}
				else ax::NodeEditor::RejectDeletedItem();
			}
		}
		ax::NodeEditor::EndDelete();

		
		
		
		
		//===== Context Menus =====

		ax::NodeEditor::Suspend();

		static ax::NodeEditor::NodeId contextNodeId = 0;
		static ax::NodeEditor::LinkId contextLinkId = 0;
		static ax::NodeEditor::PinId  contextPinId = 0;

		if (ax::NodeEditor::ShowNodeContextMenu(&contextNodeId))		ImGui::OpenPopup("Node Context Menu");
		if (ax::NodeEditor::ShowPinContextMenu(&contextPinId))		ImGui::OpenPopup("Pin Context Menu");
		if (ax::NodeEditor::ShowLinkContextMenu(&contextLinkId))	ImGui::OpenPopup("Link Context Menu");
		if (ax::NodeEditor::ShowBackgroundContextMenu())           ImGui::OpenPopup("Background Context Menu");


		if (ImGui::BeginPopup("Node Context Menu")) {
			int nodeID = contextNodeId.Get();
			if(nodeID > INT_MAX / 2) nodeID = INT_MAX - nodeID;
			std::shared_ptr<Node> node = getNode(nodeID);
			ImGui::Text("Node : %s", node->getName().c_str());
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("(#%i)", node->getUniqueID());
			ImGui::PopStyleColor();
			switch (node->getType()) {
				case Node::Type::PROCESSOR:
				case Node::Type::CONTAINER:
				case Node::Type::CLOCK:
				case Node::Type::AXIS:
				case Node::Type::MACHINE:
					ImGui::Text("Type: %s", Enumerator::getDisplayString(node->getType()));
					break;
				case Node::Type::IODEVICE: {
					std::shared_ptr<Device> device = std::static_pointer_cast<Device>(node);
					ImGui::Text("Type: %s", getDeviceType(device->getDeviceType())->displayName);
					if (device->isConnected()) ImGui::Text("Status: Online");
					else if (device->isDetected()) ImGui::Text("Status: Detected");
					else ImGui::Text("Status: Offline");
				}
			}

			ImGui::Separator();
			if (node->getType() == Node::Type::IODEVICE) {
				ImGui::MenuItem("Split Inputs and Outputs", nullptr, &node->b_isSplit, true);
			}
			if (ImGui::MenuItem("Delete")) {
				ax::NodeEditor::DeleteNode(node->getUniqueID());
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Pin Context Menu")) {
			std::shared_ptr<NodePin> pin = getPin(contextPinId.Get());
			ImGui::Text("Pin : %s", pin->getDisplayString());
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
			ImGui::Text("(#%i)", pin->getUniqueID());
			ImGui::PopStyleColor();
			ImGui::Text("Type: %s", Enumerator::getDisplayString(pin->dataType));
			ImGui::Text("Value: %s", pin->getValueString());
			if (ImGui::BeginMenu("Connected Pins")) {
				for (auto connectedPin : pin->getConnectedPins()) {
					ImGui::Text("\"%s\" (%s #%i) on Node \"%s\" (#%i)",
						connectedPin->getDisplayString(),
						Enumerator::getDisplayString(connectedPin->dataType),
						connectedPin->getUniqueID(),
						connectedPin->parentNode->getName().c_str(),
						connectedPin->parentNode->getUniqueID());
				}
				ImGui::EndMenu();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Hide")) {
				pin->b_visible = false;
			}
			if (ImGui::MenuItem("Disconnect")) {
				pin->disconnectAllLinks();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginPopup("Link Context Menu")) {
			std::shared_ptr<NodeLink> link = getLink(contextLinkId.Get());
			ImGui::Text("Link #%i", link->getUniqueID());
			ImGui::Text("Input: \"%s\" on node \"%s\"", link->getInputData()->getDisplayString(), link->getInputData()->parentNode->getName().c_str());
			ImGui::Text("Output: \"%s\" on node \"%s\"", link->getOutputData()->getDisplayString(), link->getOutputData()->parentNode->getName().c_str());
			ImGui::Separator();
			if (ImGui::MenuItem("Disconnect")) link->disconnect();
			ImGui::EndPopup();
		}

		static glm::vec2 mouseRightClickPosition;
		if (ImGui::IsMouseClicked(1)) mouseRightClickPosition = ImGui::GetMousePos();
		if (ImGui::BeginPopup("Background Context Menu")) {
			if(std::shared_ptr<Node> newNode = editorContextMenuCallback()){
				addNode(newNode);
				ax::NodeEditor::SetNodePosition(newNode->getUniqueID(), ax::NodeEditor::ScreenToCanvas(mouseRightClickPosition));
				ax::NodeEditor::SelectNode(newNode->getUniqueID());
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		
		ax::NodeEditor::Resume();

	}


	//===== Update list of selected nodes =====

	static ax::NodeEditor::NodeId selectedNodeIds[1024];
	int selectedNodeCount = ax::NodeEditor::GetSelectedNodes(selectedNodeIds, 1024);
	getSelectedNodes().clear();
	if (selectedNodeCount > 0) {
		std::vector<int> selectedIds;
		for (int i = 0; i < selectedNodeCount; i++) {
			int selectedNodeId = selectedNodeIds[i].Get();
			//negative unique ids represent split node halves
			//if (selectedNodeId < 0) selectedNodeId = abs(selectedNodeId);
			if (selectedNodeId >= INT_MAX / 2) selectedNodeId = INT_MAX - selectedNodeId;
			//we don't add selected ids to the list twice
			//this can happen in case both parts of a split node are selected
			bool alreadyInSelectedIds = false;
			for (int id : selectedIds) {
				if (id == selectedNodeId) {
					alreadyInSelectedIds = true;
					break;
				}
			}
			if (!alreadyInSelectedIds) selectedIds.push_back(selectedNodeId);
		}
		for (int id : selectedIds) {
			auto selectedNode = getNode(id);
			if(selectedNode) getSelectedNodes().push_back(selectedNode);
		}
	}
	
	//===== Update list of selected links =====
	
	static ax::NodeEditor::LinkId selectedLinkIds[1024];
	int selectedLinkCount = ax::NodeEditor::GetSelectedLinks(selectedLinkIds, 1024);
	std::vector<std::shared_ptr<NodeLink>>& selectedLinks = getSelectedLinks();
	selectedLinks.clear();
	if(selectedLinkCount > 0){
		std::vector<int> selectedIds;
		for(int i = 0; i < selectedLinkCount; i++){
			int selectedLinkID = selectedLinkIds[i].Get();
			selectedLinks.push_back(getLink(selectedLinkID));
		}
	}

	ax::NodeEditor::End();

	//====== Accept new dragged nodes ======
	if (!b_isLocked) {
		
		
		
		if (ImGui::BeginDragDropTarget()) {
			glm::vec2 mousePosition = ImGui::GetMousePos();
			if(std::shared_ptr<Node> droppedNode = nodeDragDropTargetCallback()){
				addNode(droppedNode);
				ax::NodeEditor::SetNodePosition(droppedNode->getUniqueID(), ax::NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
			}
			ImGui::EndDragDropTarget();
		}
		
		
		
		
		
	
		if (ImGui::IsKeyPressed(GLFW_KEY_BACKSPACE)){
			//node editor is hovered and no other item is active (such as text input fields)
			if (!ImGui::IsAnyItemActive() && ImGui::IsItemHovered()) {
				if(!selectedNodes.empty()){
					for (auto& node : selectedNodes) ax::NodeEditor::DeleteNode(node->getUniqueID());
				}
				if(!selectedLinks.empty()){
					for(auto& link : selectedLinks) link->disconnect();
				}
			}
		}
	}


/*
	if (getWasJustLoaded()) {
		getWasJustLoaded() = false;
		for (auto node : getNodes()) node->restoreSavedPosition();
		centerView();
		showFlow();
	}
*/
 
}

void NodeGraph::centerView() {
	ax::NodeEditor::NavigateToContent();
}

void NodeGraph::showFlow() {
	for (auto link : links) ax::NodeEditor::Flow(link->getUniqueID());
}
