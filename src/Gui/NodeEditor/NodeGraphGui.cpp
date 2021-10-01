#include <pch.h>

#include "Gui/Gui.h"
#include <imgui_internal.h>



#include "Environnement/Environnement.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/NodeFactory.h"
#include "Gui/Framework/Colors.h"

std::vector<std::shared_ptr<Node>> selectedNodes;

namespace NodeEditor = ax::NodeEditor;

bool isEditingAllowed() {
    return !EtherCatFieldbus::isCyclicExchangeActive();
}

void nodeGraph() {

    //TODO: should the graph be updated in the draw loop?
    //Environnement::nodeGraph.evaluate();

    //========= NODE INSPECTOR AND ADDER PANEL =========

    static float sideBarWidth = ImGui::GetTextLineHeight() * 28.0;
    static float minSideBarWidth = ImGui::GetTextLineHeight() * 5.0;

    glm::vec2 sideBarSize(sideBarWidth, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("SideBar", sideBarSize)) {

        //if there are no selected nodes, display the node adder list
        if (selectedNodes.empty()) nodeAdder();
        else if (selectedNodes.size() == 1) {
            std::shared_ptr<Node> selectedNode = selectedNodes.front();
            ImGui::PushFont(Fonts::robotoBold20);
            ImGui::Text(selectedNode->getName());
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
                for (auto node : selectedNodes) {
                    //we don't display the custom name in the tab
                    //so we don't switch tabs while renaming the custom name of the node
                    //we have to use pushID and PopID to avoid problems when selecting multiple nodes of the same type
                    //this way we can have multiple tabs with the same name
                    ImGui::PushID(node->getUniqueID());
                    if (ImGui::BeginTabItem(node->getNodeName())) {
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
    Environnement::nodeGraph.nodeEditorGui();

    if (isEditingAllowed()) {
        std::shared_ptr<Node> newDraggedNode = acceptDraggedNode();
        if (newDraggedNode) {
            Environnement::nodeGraph.addIoNode(newDraggedNode);
            NodeEditor::SetNodePosition(newDraggedNode->getUniqueID(), NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
        }
    }

    if (ImGui::Button("Center View")) Environnement::nodeGraph.centerView();
    ImGui::SameLine();
    if (ImGui::Button("Show Flow")) Environnement::nodeGraph.showFlow();
    ImGui::SameLine();
    ImGui::Checkbox("Show Output Values", &Environnement::nodeGraph.b_showOutputValues);
    ImGui::SameLine();

    if(!isEditingAllowed()) ImGui::TextColored(Colors::gray, "Editing is disabled while the system is running");

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

void NodeGraph::nodeEditorGui() {

	NodeEditor::SetCurrentEditor(ImGuiNodeEditor::nodeEditorContext);
	NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

    //===== DRAW NODES =====

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2, 0));
    for (auto node : Environnement::nodeGraph.getNodes()) node->nodeGui();
    ImGui::PopStyleVar(2);

    //===== DRAW LINKS =====

    for (auto link : Environnement::nodeGraph.getLinks())
        NodeEditor::Link(link->getUniqueID(),
            link->getInputData()->getUniqueID(),
            link->getOutputData()->getUniqueID(),
            ImColor(1.0f, 1.0f, 1.0f),
            1.0);



    if (isEditingAllowed()) {

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
                    removeIoNode(deletedNode);
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
            ImGui::Text("Node Context Menu, Node#%i", contextNodeId);
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Pin Context Menu")) {
            ImGui::Text("Pin Context Menu, Pin#%i", contextPinId);
            ImGui::Text(Environnement::nodeGraph.getPin(contextPinId.Get())->getValueString());
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Link Context Menu")) {
            ImGui::Text("Link Context Menu, Link#%i", contextLinkId);
            ImGui::EndPopup();
        }


        static glm::vec2 mouseRightClickPosition;
        if (ImGui::IsMouseClicked(1)) mouseRightClickPosition = ImGui::GetMousePos();
        if (ImGui::BeginPopup("Background Context Menu")) {
            std::shared_ptr<Node> newNode = nodeAdderContextMenu();
            if (newNode) {
                Environnement::nodeGraph.addIoNode(newNode);
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
