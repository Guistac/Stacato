#include <pch.h>

#include "Gui/Gui.h"

#include "Environnement/Environnement.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"
#include "Fieldbus/Utilities/EtherCatDeviceFactory.h"
#include "Environnement/NodeGraph/Utilities/ioNodeFactory.h"

std::vector<std::shared_ptr<ioNode>> selectedNodes;

namespace ImGuiNodeEditor {
    ax::NodeEditor::EditorContext* nodeEditorContext;

    void CreateContext() {
        nodeEditorContext = ax::NodeEditor::CreateEditor();
    }

    void DestroyContext() {
        ax::NodeEditor::DestroyEditor(nodeEditorContext);
    }
}

namespace NodeEditor = ax::NodeEditor;

void nodeGraph() {

    static bool showValues = true;

    Environnement::nodeGraph.evaluate();

    glm::vec2 sideBarSize(ImGui::GetTextLineHeight() * 20.0, ImGui::GetContentRegionAvail().y);
    if (ImGui::BeginChild("SideBar", sideBarSize)) {

        if (ImGui::BeginTabBar("NodeEditorSidePanel")) {

            for (auto node : selectedNodes) {
                if (ImGui::BeginTabItem(node->getName())) {
                    if (ImGui::BeginChild("NodePropertyChild", ImGui::GetContentRegionAvail())) {
                        node->propertiesGui();
                        ImGui::EndChild();
                    }
                    ImGui::EndTabItem();
                }
            }

            if (ImGui::BeginTabItem("Add Nodes")) {
                nodeAdder();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

	
        ImGui::EndChild();
    }

	ImGui::SameLine();

    //========= VERTICAL SEPARATOR ==========

	ImGui::BeginGroup();

	nodeEditor(showValues);

    if (ImGui::BeginDragDropTarget()){
        const ImGuiPayload* payload;
        payload = ImGui::AcceptDragDropPayload("EtherCatSlave");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char * slaveDeviceName = *(const char**)payload->Data;
            std::shared_ptr<ioNode> newSlave = EtherCatDeviceFactory::getDeviceByName(slaveDeviceName);
            Environnement::nodeGraph.addIoNode(newSlave);
            payload = nullptr;
        }
        payload = ImGui::AcceptDragDropPayload("ProcessorNode");
        if (payload != nullptr && payload->DataSize == sizeof(const char*)) {
            const char* processorName = *(const char**)payload->Data;
            std::shared_ptr<ioNode> newProcessor = ioNodeFactory::getIoNodeByName(processorName);
            Environnement::nodeGraph.addIoNode(newProcessor);
            payload = nullptr;
        }
        ImGui::EndDragDropTarget();
    }

	if (ImGui::Button("Center View")) ax::NodeEditor::NavigateToContent();
    ImGui::SameLine();
    ImGui::Checkbox("Always Show Values", &showValues);

	ImGui::EndGroup();

}



void nodeEditor(bool alwaysShowValues) {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
	NodeEditor::SetCurrentEditor(ImGuiNodeEditor::nodeEditorContext);
	NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

    drawNodes(alwaysShowValues);
    drawLinks();

    // Handle creation action, returns true if editor want to create new object (node or link)
    if (NodeEditor::BeginCreate(ImVec4(1.0, 1.0, 1.0, 1.0), 1.0)) {
        createLink(); //TODO: don't allow duplicate links
    }
    // Wraps up object creation action handling.
    NodeEditor::EndCreate();


    // Handle deletion action
    if (NodeEditor::BeginDelete()) {
        deleteLink();
        deleteNode();
    }
    NodeEditor::EndDelete(); // Wrap up deletion action

    static NodeEditor::NodeId contextNodeId = 0;
    static NodeEditor::LinkId contextLinkId = 0;
    static NodeEditor::PinId  contextPinId = 0;

    NodeEditor::Suspend();
    if (NodeEditor::ShowNodeContextMenu(&contextNodeId))		ImGui::OpenPopup("Node Context Menu");
    else if (NodeEditor::ShowPinContextMenu(&contextPinId))		ImGui::OpenPopup("Pin Context Menu");
    else if (NodeEditor::ShowLinkContextMenu(&contextLinkId))	ImGui::OpenPopup("Link Context Menu");
    else if (NodeEditor::ShowBackgroundContextMenu())			ImGui::OpenPopup("Background Context Menu");
    NodeEditor::Resume();

    NodeEditor::Suspend();
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

    if (ImGui::BeginPopup("Background Context Menu")) {
        ImGui::Text("Background Context Menu");
        ImGui::EndPopup();
    }
    NodeEditor::Resume();

    getSelectedNodes();

	NodeEditor::End();

}

void nodeAdder() {

    if (ImGui::BeginListBox("##NodeList", ImGui::GetContentRegionAvail())) {

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("EtherCAT Slaves")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            for (auto& manufacturer : EtherCatDeviceFactory::getDeviceTypes()) {
                if (ImGui::TreeNode(manufacturer.name)) {
                    for (auto& slave : manufacturer.devices) {
                        bool selected = false;
                        const char* deviceName = slave->getNodeTypeName();
                        ImGui::Selectable(deviceName, &selected);
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                            ImGui::SetDragDropPayload("EtherCatSlave", &deviceName, sizeof(const char*));
                            ImGui::Text(deviceName);
                            ImGui::EndDragDropSource();
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("Axis")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            bool selected = false;
            ImGui::Selectable("Linear Axis", &selected);
            ImGui::Selectable("Rotating Axis", &selected);
            ImGui::Selectable("State Machine", &selected);
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("Network IO")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            bool selected = false;
            ImGui::Selectable("OSC", &selected);
            ImGui::Selectable("Artnet", &selected);
            ImGui::Selectable("PSN", &selected);
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::PushFont(Fonts::robotoBold15);
        if (ImGui::TreeNode("Data Processors")) {
            ImGui::PushFont(Fonts::robotoRegular15);
            for (ioNode* node : ioNodeFactory::getIoNodeList()) {
                bool selected = false;
                const char* processorName = node->getNodeTypeName();
                ImGui::Selectable(processorName, &selected);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("ProcessorNode", &processorName, sizeof(const char*));
                    ImGui::Text(processorName);
                    ImGui::EndDragDropSource();
                }
            }
            ImGui::PopFont();
            ImGui::TreePop();
        }
        ImGui::PopFont();

        ImGui::EndListBox();
    }
}

void drawNodes(bool alwaysShowValues) {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetTextLineHeight() * 0.2, 0));
    for (auto node : nodeGraph.getIoNodes()) node->nodeGui(alwaysShowValues);
    ImGui::PopStyleVar(2); //reset item spacing
}

void drawLinks() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    for (auto link : nodeGraph.getIoLinks())
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
    static NodeEditor::NodeId selectedNodeIds[16];
    int selectedNodeCount = NodeEditor::GetSelectedNodes(selectedNodeIds, 16);
    selectedNodes.clear();
    if (selectedNodeCount > 0) {
        for (int i = 0; i < selectedNodeCount; i++) {
            selectedNodes.push_back(Environnement::nodeGraph.getIoNode(selectedNodeIds[i].Get()));
        }
    }
}