#include <pch.h>

#include "Gui/Gui.h"
#include <imgui_internal.h>

#include "Environnement/Environnement.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "NodeGraph/Utilities/ioNodeFactory.h"

std::vector<std::shared_ptr<ioNode>> selectedNodes;

namespace NodeEditor = ax::NodeEditor;

void nodeGraph() {

    static bool showValues = true;

    Environnement::nodeGraph.evaluate();

    float sideBarWidth = ImGui::GetTextLineHeight() * 20.0;
    glm::vec2 sideBarSize(sideBarWidth, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("SideBar", sideBarSize)) {
        if (!selectedNodes.empty() && ImGui::BeginTabBar("NodeEditorSidePanel")) {
            
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
        if (selectedNodes.empty()) {
            nodeAdder();
        }
        ImGui::EndChild();
    }

	ImGui::SameLine();

    //========= VERTICAL SEPARATOR ==========

	ImGui::BeginGroup();

	nodeEditor(showValues);

    std::shared_ptr<ioNode> newDraggedNode = acceptDraggedNode();
    if (newDraggedNode) {
        Environnement::nodeGraph.addIoNode(newDraggedNode);
        NodeEditor::SetNodePosition(newDraggedNode->getUniqueID(), NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
    }

	if (ImGui::Button("Center View")) ax::NodeEditor::NavigateToContent();
    ImGui::SameLine();
    ImGui::Checkbox("Always Show Values", &showValues);

	ImGui::EndGroup();

}



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

void nodeEditor(bool alwaysShowValues) {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
	NodeEditor::SetCurrentEditor(ImGuiNodeEditor::nodeEditorContext);
	NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

    drawNodes(alwaysShowValues);
    drawLinks();

    // Handle creation action, returns true if editor want to create new link
    if (NodeEditor::BeginCreate(ImVec4(1.0, 1.0, 1.0, 1.0), 1.0)) createLink();
    NodeEditor::EndCreate();

    // Handle deletion action
    if (NodeEditor::BeginDelete()) {
        deleteLink();
        deleteNode();
    }
    NodeEditor::EndDelete();

    NodeEditor::Suspend();

    static NodeEditor::NodeId contextNodeId = 0;
    static NodeEditor::LinkId contextLinkId = 0;
    static NodeEditor::PinId  contextPinId = 0;

    if (NodeEditor::ShowNodeContextMenu(&contextNodeId))		ImGui::OpenPopup("Node Context Menu");
    else if (NodeEditor::ShowPinContextMenu(&contextPinId))		ImGui::OpenPopup("Pin Context Menu");
    else if (NodeEditor::ShowLinkContextMenu(&contextLinkId))	ImGui::OpenPopup("Link Context Menu");
    else if (NodeEditor::ShowBackgroundContextMenu())           ImGui::OpenPopup("Background Context Menu");

    if (ImGui::BeginPopup("Node Context Menu")) {
        ImGui::Text("Node Context Menu, Node#%i", contextNodeId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Pin Context Menu")) {
        ImGui::Text("Pin Context Menu, Pin#%i", contextPinId);
        ImGui::Text(Environnement::nodeGraph.getIoData(contextPinId.Get())->getValueString());
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu")) {
        ImGui::Text("Link Context Menu, Link#%i", contextLinkId);
        ImGui::EndPopup();
    }

    static glm::vec2 mouseRightClickPosition;
    if (ImGui::IsMouseClicked(1)) mouseRightClickPosition = ImGui::GetMousePos();
    if (ImGui::BeginPopup("Background Context Menu")) {
        std::shared_ptr<ioNode> newNode = nodeAdderContextMenu();
        if (newNode) {
            Environnement::nodeGraph.addIoNode(newNode);
            NodeEditor::SetNodePosition(newNode->getUniqueID(), NodeEditor::ScreenToCanvas(mouseRightClickPosition));
            NodeEditor::SelectNode(newNode->getUniqueID());
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    NodeEditor::Resume();

    getSelectedNodes();

	NodeEditor::End();

}

void drawNodes(bool alwaysShowValues) {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2, 0));
    for (auto node : Environnement::nodeGraph.getIoNodes()) node->nodeGui(alwaysShowValues);
    ImGui::PopStyleVar(2);
}

void drawLinks() {
    for (auto link : Environnement::nodeGraph.getIoLinks())
        NodeEditor::Link(link->getUniqueID(),
            link->getOutputData()->getUniqueID(),
            link->getInputData()->getUniqueID(),
            ImColor(1.0f, 1.0f, 1.0f),
            1.0);
}

void createLink() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    NodeEditor::PinId pin1Id, pin2Id;
    if (NodeEditor::QueryNewLink(&pin1Id, &pin2Id)) {
        if (pin1Id && pin2Id) {
            std::shared_ptr<ioData> pin1 = nodeGraph.getIoData(pin1Id.Get());
            std::shared_ptr<ioData> pin2 = nodeGraph.getIoData(pin2Id.Get());
            if (pin1 && pin2 && nodeGraph.isConnectionValid(pin1, pin2)) {
                if (NodeEditor::AcceptNewItem(ImColor(1.0f, 1.0f, 1.0f), 3.0)) {
                    std::shared_ptr<ioLink> link = nodeGraph.connect(pin1, pin2);
                    NodeEditor::Link(link->getUniqueID(), pin1Id, pin2Id);
                }
            }
            else NodeEditor::RejectNewItem(ImColor(1.0f, 0.0f, 0.0f), 3.0);
        }
    }
}

void deleteLink() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    NodeEditor::LinkId deletedLinkId;
    while (NodeEditor::QueryDeletedLink(&deletedLinkId)) {
        if (NodeEditor::AcceptDeletedItem()) {
            std::shared_ptr<ioLink> deletedLink = nodeGraph.getIoLink(deletedLinkId.Get());
            if (deletedLink) nodeGraph.disconnect(deletedLink);
        }
    }
}

void deleteNode() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    NodeEditor::NodeId deletedNodeId;
    while (NodeEditor::QueryDeletedNode(&deletedNodeId)) {
        std::shared_ptr<ioNode> deletedNode = nodeGraph.getIoNode(deletedNodeId.Get());
        if (NodeEditor::AcceptDeletedItem()) {
            nodeGraph.removeIoNode(deletedNode);
        }
        else NodeEditor::RejectDeletedItem();
    }
}

void getSelectedNodes() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    static NodeEditor::NodeId selectedNodeIds[16];
    int selectedNodeCount = NodeEditor::GetSelectedNodes(selectedNodeIds, 16);
    selectedNodes.clear();
    if (selectedNodeCount > 0) {
        std::vector<int> selectedIds;
        for (int i = 0; i < selectedNodeCount; i++) {
            //we take the absolute value because split nodes have second id thats the negative of their main unique id
            int selectedNodeId = abs((int)selectedNodeIds[i].Get());
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
            selectedNodes.push_back(nodeGraph.getIoNode(id));
        }
    }
}