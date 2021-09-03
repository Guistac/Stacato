#include <pch.h>

#include "Gui/Gui.h"

#include "Environnement/Environnement.h"
#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

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
	
	ImGui::BeginGroup();

	if (ImGui::BeginListBox("##NodeList", ImVec2(ImGui::GetTextLineHeight() * 15.0, ImGui::GetContentRegionAvail().y))) {

		if (ImGui::TreeNode("EtherCAT Slaves")) {
			ImGui::Button("Refresh Device List");

			for (auto slave : EtherCatFieldbus::slaves) {
                if (slave->isInNodeGraph()) continue;
				bool selected = false;
				ImGui::Selectable(slave->getName(), &selected);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload("EtherCatSlave", &slave, sizeof(std::shared_ptr<EtherCatSlave>));
					ImGui::Text("%s", slave->getName());
					ImGui::Text("Drop On Node Editor to Add Slave");
                    ImGui::EndDragDropSource();
                }
			}

			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Axis")) {
			bool selected = false;
			ImGui::Selectable("Linear Axis", &selected);
			ImGui::Selectable("Rotating Axis", &selected);
			ImGui::Selectable("State Machine", &selected);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Network IO")) {
			bool selected = false;
			ImGui::Selectable("OSC", &selected);
			ImGui::Selectable("Artnet", &selected);
			ImGui::Selectable("PSN", &selected);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Data Processors")) {
			bool selected = false;
			ImGui::Selectable("Condition", &selected);
			ImGui::Selectable("Add", &selected);
			ImGui::Selectable("Subtract", &selected);
			ImGui::Selectable("Multiply", &selected);
			ImGui::Selectable("Divide", &selected);
			ImGui::Selectable("Limit", &selected);
			ImGui::Selectable("Map", &selected);
			ImGui::TreePop();
		}

		ImGui::EndListBox();
	}

	ImGui::EndGroup();

	ImGui::SameLine();

    //========= VERTICAL SEPARATOR ==========

	ImGui::BeginGroup();

	nodeEditor();

    if (ImGui::BeginDragDropTarget()){
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EtherCatSlave");
        if (payload != nullptr && payload->DataSize == sizeof(std::shared_ptr<EtherCatSlave>)){
            std::shared_ptr<EtherCatSlave> slave = *(std::shared_ptr<EtherCatSlave>*)payload->Data;
			Environnement::add(slave);
        }
        ImGui::EndDragDropTarget();
    }

	if (ImGui::Button("Center View")) ax::NodeEditor::NavigateToContent();

	ImGui::EndGroup();
}



void nodeEditor() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
	NodeEditor::SetCurrentEditor(ImGuiNodeEditor::nodeEditorContext);
	NodeEditor::Begin("Node Editor", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7));

    drawNodes();
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


	NodeEditor::End();
}



void drawNodes() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2, ImGui::GetTextLineHeight() * 0.2));
    for (auto node : nodeGraph.getIoNodes()) node->nodeGui();
    ImGui::PopStyleVar(); //reset item spacing
}

void drawLinks() {
    NodeGraph& nodeGraph = Environnement::nodeGraph;
    for (auto link : nodeGraph.getIoLinks())
        NodeEditor::Link(link->getUniqueID(),
            link->getOutputPin()->getUniqueID(),
            link->getInputPin()->getUniqueID(),
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
