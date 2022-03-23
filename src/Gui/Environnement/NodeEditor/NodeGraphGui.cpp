#include <pch.h>


#include "NodeEditorGui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <GLFW/glfw3.h>

#include "Environnement/NodeGraph.h"
#include "Project/Project.h"
#include "Environnement/Environnement.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "Environnement/NodeFactory.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

namespace NodeEditor = ax::NodeEditor;

namespace ImGuiNodeEditor {
	ax::NodeEditor::EditorContext* nodeEditorContext;

	void CreateContext() {
		nodeEditorContext = ax::NodeEditor::CreateEditor();
	}

	void DestroyContext() {
		ax::NodeEditor::DestroyEditor(nodeEditorContext);
	}
}




namespace Environnement::NodeGraph::Gui{

	void editor() {
		
		NodeEditor::SetCurrentEditor(ImGuiNodeEditor::nodeEditorContext);
		NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

		//===== DRAW NODES =====

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2, 0));
		for (auto node : Environnement::NodeGraph::getNodes()) node->nodeGui();
		ImGui::PopStyleVar(2);

		//===== DRAW LINKS =====

		for (auto link : Environnement::NodeGraph::getLinks())
			NodeEditor::Link(link->getUniqueID(),
				link->getInputData()->getUniqueID(),
				link->getOutputData()->getUniqueID(),
				ImColor(1.0f, 1.0f, 1.0f),
				1.0);

		if (Project::isEditingAllowed()) {

			//===== Handle link creation action, returns true if editor want to create new link

			if (NodeEditor::BeginCreate(ImVec4(1.0, 1.0, 1.0, 1.0), 1.0)) {
				NodeEditor::PinId pin1Id, pin2Id;
				if (NodeEditor::QueryNewLink(&pin1Id, &pin2Id)) {
					if (pin1Id && pin2Id) {
						std::shared_ptr<NodePin> pin1 = getPin(pin1Id.Get());
						std::shared_ptr<NodePin> pin2 = getPin(pin2Id.Get());
						if (pin1 && pin2 && isConnectionValid(pin1, pin2)) {
							if (NodeEditor::AcceptNewItem(ImColor(1.0f, 1.0f, 1.0f), 3.0)) {
								std::shared_ptr<NodeLink> link = connect(pin1, pin2);
								NodeEditor::Link(link->getUniqueID(), pin1Id, pin2Id);
							}
						}
						else NodeEditor::RejectNewItem(ImColor(1.0f, 0.0f, 0.0f), 3.0);
					}
				}
			}
			NodeEditor::EndCreate();

			//===== Handle link and node deletion action =====

			if (NodeEditor::BeginDelete()) {

				NodeEditor::LinkId deletedLinkId;
				while (NodeEditor::QueryDeletedLink(&deletedLinkId)) {
					if (NodeEditor::AcceptDeletedItem()) {
						std::shared_ptr<NodeLink> deletedLink = getLink(deletedLinkId.Get());
						if (deletedLink) disconnect(deletedLink);
					}
				}

				NodeEditor::NodeId deletedNodeId;
				while (NodeEditor::QueryDeletedNode(&deletedNodeId)) {
					std::shared_ptr<Node> deletedNode = getNode(deletedNodeId.Get());
					if (deletedNode && NodeEditor::AcceptDeletedItem()) {
						Environnement::NodeGraph::removeNode(deletedNode); //TODO: why are we removing twice ?
						Environnement::removeNode(deletedNode);
					}
					else NodeEditor::RejectDeletedItem();
				}
			}
			NodeEditor::EndDelete();

			//===== Handle Context Menus =====

			NodeEditor::Suspend();

			static NodeEditor::NodeId contextNodeId = 0;
			static NodeEditor::LinkId contextLinkId = 0;
			static NodeEditor::PinId  contextPinId = 0;

			if (NodeEditor::ShowNodeContextMenu(&contextNodeId))		ImGui::OpenPopup("Node Context Menu");
			if (NodeEditor::ShowPinContextMenu(&contextPinId))		ImGui::OpenPopup("Pin Context Menu");
			if (NodeEditor::ShowLinkContextMenu(&contextLinkId))	ImGui::OpenPopup("Link Context Menu");
			if (NodeEditor::ShowBackgroundContextMenu())           ImGui::OpenPopup("Background Context Menu");


			if (ImGui::BeginPopup("Node Context Menu")) {
				std::shared_ptr<Node> node = getNode(contextNodeId.Get());
				ImGui::Text("Node : %s", node->getName());
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
						std::shared_ptr<Device> device = std::dynamic_pointer_cast<Device>(node);
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
					NodeEditor::DeleteNode(node->getUniqueID());
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
							connectedPin->parentNode->getName(),
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
				ImGui::Text("Input: \"%s\" on node \"%s\"", link->getInputData()->getDisplayString(), link->getInputData()->parentNode->getName());
				ImGui::Text("Output: \"%s\" on node \"%s\"", link->getOutputData()->getDisplayString(), link->getOutputData()->parentNode->getName());
				ImGui::Separator();
				if (ImGui::MenuItem("Disconnect")) disconnect(link);
				ImGui::EndPopup();
			}


			static glm::vec2 mouseRightClickPosition;
			if (ImGui::IsMouseClicked(1)) mouseRightClickPosition = ImGui::GetMousePos();
			if (ImGui::BeginPopup("Background Context Menu")) {
				std::shared_ptr<Node> newNode = nodeAdderContextMenu();
				if (newNode) {
					Environnement::NodeGraph::addNode(newNode); //TODO: why are we adding the node twice ?
					Environnement::addNode(newNode);
					NodeEditor::SetNodePosition(newNode->getUniqueID(), NodeEditor::ScreenToCanvas(mouseRightClickPosition));
					NodeEditor::SelectNode(newNode->getUniqueID());
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			NodeEditor::Resume();

		}


		//===== Update list of selected nodes =====

		static NodeEditor::NodeId selectedNodeIds[1024];
		int selectedNodeCount = NodeEditor::GetSelectedNodes(selectedNodeIds, 1024);
		getSelectedNodes().clear();
		if (selectedNodeCount > 0) {
			std::vector<int> selectedIds;
			for (int i = 0; i < selectedNodeCount; i++) {
				int selectedNodeId = selectedNodeIds[i].Get();
				//negative unique ids represent split node halves
				//if (selectedNodeId < 0) selectedNodeId = abs(selectedNodeId);
				if (selectedNodeId >= 100000) selectedNodeId = selectedNodeId - 100000;
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
			for (int id : selectedIds) getSelectedNodes().push_back(getNode(id));
		}
		
		//===== Update list of selected links =====
		
		static NodeEditor::LinkId selectedLinkIds[1024];
		int selectedLinkCount = NodeEditor::GetSelectedLinks(selectedLinkIds, 1024);
		std::vector<std::shared_ptr<NodeLink>>& selectedLinks = getSelectedLinks();
		selectedLinks.clear();
		if(selectedLinkCount > 0){
			std::vector<int> selectedIds;
			for(int i = 0; i < selectedLinkCount; i++){
				int selectedLinkID = selectedLinkIds[i].Get();
				selectedLinks.push_back(getLink(selectedLinkID));
			}
		}

		NodeEditor::End();

		/*
		static std::vector<std::shared_ptr<Node>> copiedNodes;

		if (ImGui::IsItemHovered()) {
			if ((ImGui::IsKeyDown(GLFW_KEY_LEFT_CONTROL) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_CONTROL))) {
			
				if (ImGui::IsKeyPressed(GLFW_KEY_C, false)) {
					if (!selectedNodes.empty()) {
						copiedNodes = selectedNodes;
						Logger::warn("Copied {} Nodes", copiedNodes.size());
					}
				}
				if (ImGui::IsKeyPressed(GLFW_KEY_V, false)) {
					for (auto copiedNode : copiedNodes) {
						//TODO: Duplicate Node and add to nodegraph
						//std::shared_ptr<Node> copy = std::make_shared<Node>(copiedNode);
						//addNode(copy);
					}
				}
			}
		}
		 */
		
		std::vector<std::shared_ptr<Node>> selectedNodes = getSelectedNodes();

		if (ImGui::IsKeyPressed(GLFW_KEY_BACKSPACE)){
			//node editor is hovered and no other item is active (such as text input fields)
			if (!ImGui::IsAnyItemActive() && ImGui::IsItemHovered()) {
				if(!selectedNodes.empty()){
					for (auto& node : selectedNodes) NodeEditor::DeleteNode(node->getUniqueID());
				}
				if(!selectedLinks.empty()){
					for(auto& link : selectedLinks) disconnect(link);
				}
			}
		}



		if (getWasJustLoaded()) {
			getWasJustLoaded() = false;
			for (auto node : getNodes()) node->restoreSavedPosition();
			centerView();
			showFlow();
		}

	}

	void centerView() {
		NodeEditor::NavigateToContent();
	}

	void showFlow() {
		for (auto link : getLinks()) {
			NodeEditor::Flow(link->getUniqueID());
		}
	}

	bool b_showOutputValues;
	bool& getShowOutputValues(){ return b_showOutputValues; }


}
