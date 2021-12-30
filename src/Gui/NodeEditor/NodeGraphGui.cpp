#include <pch.h>


#include "NodeEditorGui.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_node_editor.h>
#include <GLFW/glfw3.h>

#include "NodeGraph/NodeGraph.h"
#include "Project/Project.h"
#include "Project/Environnement.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

namespace NodeEditor = ax::NodeEditor;

void EnvironnementNodeEditorGui() {

    //========= NODE INSPECTOR AND ADDER PANEL =========

    static float sideBarWidth = ImGui::GetTextLineHeight() * 28.0;
    static float minSideBarWidth = ImGui::GetTextLineHeight() * 5.0;

    glm::vec2 sideBarSize(sideBarWidth, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("SideBar", sideBarSize)) {

        //if there are no selected nodes, display the Environnement Name Editor and Node adder list
		if (Environnement::nodeGraph->selectedNodes.empty()) {
			
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("Environnement Editor");
			ImGui::PopFont();
			
			
			ImGui::PushFont(Fonts::robotoBold15);
			if(ImGui::CollapsingHeader("Project")){
				ImGui::PushFont(Fonts::robotoRegular15);
				ImGui::Text("Project Name :");
				ImGui::InputText("##EnvName", (char*)Environnement::getName(), 256);
				if(ImGui::IsItemDeactivatedAfterEdit()) Environnement::updateName();
				
				ImGui::Text("Notes :");
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::InputTextMultiline("##notes", (char*)Environnement::getNotes(), 65536);
				ImGui::PopFont();
			}
			ImGui::PopFont();
			
			ImGui::Separator();
			
			nodeAdder();
		}
        else if (Environnement::nodeGraph->selectedNodes.size() == 1) {
            std::shared_ptr<Node> selectedNode = Environnement::nodeGraph->selectedNodes.front();
            ImGui::PushFont(Fonts::robotoBold20);
            ImGui::Text("%s", selectedNode->getName());
            ImGui::PopFont();
            ImGui::Separator();
            if (ImGui::BeginChild("NodePropertyChild", ImGui::GetContentRegionAvail())) {
                selectedNode->propertiesGui();
                ImGui::EndChild();
            }
        }
        else {
            ImGui::PushFont(Fonts::robotoBold20);
            ImGui::Text("Multiple Nodes Selected");
            ImGui::PopFont();
            if (ImGui::BeginTabBar("NodeEditorSidePanel")) {
                for (auto node : Environnement::nodeGraph->selectedNodes) {
                    //we don't display the custom name in the tab
                    //so we don't switch tabs while renaming the custom name of the node
                    //we have to use pushID and PopID to avoid problems when selecting multiple nodes of the same type
                    //this way we can have multiple tabs with the same name
                    ImGui::PushID(node->getUniqueID());
                    if (ImGui::BeginTabItem(node->getSaveName())) {
                        if (ImGui::BeginChild("NodePropertyChild", ImGui::GetContentRegionAvail())) {
                            node->propertiesGui();
                            ImGui::EndChild();
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::PopID();
                }
                ImGui::EndTabBar();
            }
        }

        ImGui::EndChild();
    }

    //========= VERTICAL MOVABLE SEPARATOR ==========

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::SameLine();
    ImGui::InvisibleButton("VerticalSplitter", glm::vec2(ImGui::GetTextLineHeight() / 3.0, ImGui::GetContentRegionAvail().y));
    if (ImGui::IsItemActive()) sideBarWidth += ImGui::GetIO().MouseDelta.x;
    if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    if (sideBarWidth < minSideBarWidth) sideBarWidth = minSideBarWidth;
    ImGui::SameLine();
    ImGui::PopStyleVar();

    //========== NODE EDITOR BLOCK ============

    ImGui::BeginGroup();

    //Draw the entire node editor
    Environnement::nodeGraph->editorGui();

    if (Project::isEditingAllowed()) {
        std::shared_ptr<Node> newDraggedNode = acceptDraggedNode();
        if (newDraggedNode) {
            Environnement::nodeGraph->addNode(newDraggedNode);
            Environnement::addNode(newDraggedNode);
            NodeEditor::SetNodePosition(newDraggedNode->getUniqueID(), NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
        }
    }

    if (ImGui::Button("Center View")) Environnement::nodeGraph->centerView();
    ImGui::SameLine();
    if (ImGui::Button("Show Flow")) Environnement::nodeGraph->showFlow();
    ImGui::SameLine();
    ImGui::Checkbox("Show Output Values", &Environnement::nodeGraph->b_showOutputValues);
    ImGui::SameLine();

    if (!Project::isEditingAllowed()) ImGui::TextColored(Colors::gray, "Editing is disabled while the system is running");

    ImGui::EndGroup();

}




namespace ImGuiNodeEditor {
    ax::NodeEditor::EditorContext* nodeEditorContext;

    void CreateContext() {
        nodeEditorContext = ax::NodeEditor::CreateEditor();
    }

    void DestroyContext() {
        ax::NodeEditor::DestroyEditor(nodeEditorContext);
    }
}

void NodeGraph::editorGui() {
	
	NodeEditor::SetCurrentEditor(ImGuiNodeEditor::nodeEditorContext);	
	NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

    //===== DRAW NODES =====

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2, 0));
    for (auto node : Environnement::nodeGraph->getNodes()) node->nodeGui();
    ImGui::PopStyleVar(2);

    //===== DRAW LINKS =====

    for (auto link : Environnement::nodeGraph->getLinks())
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
                            //TODO: should we even process the node graph in the gui?
                            /*
                            if (pin1->isInput()) evaluate(pin1->getNode());
                            else if (pin2->isInput()) evaluate(pin2->getNode());
                            */
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
                    Environnement::nodeGraph->removeNode(deletedNode);
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
            case Node::Type::MACHINE:
                ImGui::Text("Type: %s", getNodeType(node->getType())->displayName);
                break;
            case Node::Type::IODEVICE: {
                std::shared_ptr<Device> device = std::dynamic_pointer_cast<Device>(node);
                ImGui::Text("Type: %s", getDeviceType(device->getDeviceType())->displayName);
                if (device->isOnline()) ImGui::Text("Status: Online");
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
            ImGui::Text("Pin : %s", pin->getDisplayName());
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
            ImGui::Text("(#%i)", pin->getUniqueID());
            ImGui::PopStyleColor();
            ImGui::Text("Type: %s", getNodeDataType(pin->getType())->displayName);
            ImGui::Text("Value: %s", pin->getValueString());
            if (ImGui::BeginMenu("Connected Pins")) {
                for (auto connectedPin : pin->getConnectedPins()) {
                    ImGui::Text("\"%s\" (%s #%i) on Node \"%s\" (#%i)",
                        connectedPin->getDisplayName(),
                        getNodeDataType(connectedPin->getType())->displayName,
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
            ImGui::Text("Input: \"%s\" on node \"%s\"", link->getInputData()->getDisplayName(), link->getInputData()->parentNode->getName());
            ImGui::Text("Output: \"%s\" on node \"%s\"", link->getOutputData()->getDisplayName(), link->getOutputData()->parentNode->getName());
            ImGui::Separator();
            if (ImGui::MenuItem("Disconnect")) disconnect(link);
            ImGui::EndPopup();
        }


        static glm::vec2 mouseRightClickPosition;
        if (ImGui::IsMouseClicked(1)) mouseRightClickPosition = ImGui::GetMousePos();
        if (ImGui::BeginPopup("Background Context Menu")) {
            std::shared_ptr<Node> newNode = nodeAdderContextMenu();
            if (newNode) {
                Environnement::nodeGraph->addNode(newNode);
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

    static NodeEditor::NodeId selectedNodeIds[16];
    int selectedNodeCount = NodeEditor::GetSelectedNodes(selectedNodeIds, 16);
    selectedNodes.clear();
    if (selectedNodeCount > 0) {
        std::vector<int> selectedIds;
        for (int i = 0; i < selectedNodeCount; i++) {
            int selectedNodeId = selectedNodeIds[i].Get();
            //even unique ids represent split node halves
            if (selectedNodeId < 0) selectedNodeId = abs(selectedNodeId);
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
            selectedNodes.push_back(getNode(id));
        }
    }


	NodeEditor::End();

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

    if (ImGui::IsKeyPressed(GLFW_KEY_BACKSPACE)){
        //node editor is hovered and no other item is active (such as text input fields)
        if (!selectedNodes.empty() && !ImGui::IsAnyItemActive() && ImGui::IsItemHovered()) {
            for (auto& node : selectedNodes) {
                NodeEditor::DeleteNode(node->getUniqueID());
            }
        }
    }



    if (b_justLoaded) {
        b_justLoaded = false;
        for (auto node : nodes) node->restoreSavedPosition();
        centerView();
        showFlow();
    }

}

void NodeGraph::centerView() {
    NodeEditor::NavigateToContent();
}

void NodeGraph::showFlow() {
    for (auto link : getLinks()) {
        NodeEditor::Flow(link->getUniqueID());
    }
}
