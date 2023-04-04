#include <pch.h>

#include <imgui.h>
#include <imgui_node_editor.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include "Environnement/Environnement.h"
#include "Environnement/NodeGraph/NodeGraph.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "EnvironnementGui.h"

#include "Stacato/StacatoEditor.h"
#include "Stacato/Project/StacatoProject.h"
#include "Environnement/Environnement.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Nodes/NodeFactory.h"

namespace Environnement::Gui{


void nodeAdder() {

	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetTextLineHeight() * 1.0);

	ImGui::BeginChild("##NodeList", ImGui::GetContentRegionAvail());

		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Node Library");
		ImGui::PopFont();
	
		auto listNodes = [](const std::vector<std::shared_ptr<Node>>& nodes){
			for (auto& node : nodes) {
				const char* nodeDisplayName = node->getName().c_str();
				ImGui::Selectable(nodeDisplayName);
				const char* nodeSaveName = node->getSaveName();
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
					ImGui::SetDragDropPayload("Node", &nodeSaveName, sizeof(const char*));
					ImGui::Text("%s", nodeDisplayName);
					ImGui::EndDragDropSource();
				}
			}
		};
	
		ImGui::PushFont(Fonts::sansBold15);
		if (ImGui::CollapsingHeader("EtherCAT Devices")) {

			ImGui::PushFont(Fonts::sansRegular15);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
			ImGui::Text("By Manufacturer");
			ImGui::PopStyleColor();
			for (auto& manufacturer : NodeFactory::getEtherCatDevicesByManufacturer()) {
				ImGui::PushFont(Fonts::sansBold15);
				if (ImGui::TreeNode(manufacturer.name)) {
					ImGui::PopFont();
					listNodes(manufacturer.nodes);
					ImGui::TreePop();
				}else ImGui::PopFont();
			}
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
			ImGui::Text("By Category");
			ImGui::PopStyleColor();
			for (auto& category : NodeFactory::getEtherCatDevicesByCategory()) {
				ImGui::PushFont(Fonts::sansBold15);
				if (ImGui::TreeNode(category.name)) {
					ImGui::PopFont();
					listNodes(category.nodes);
					ImGui::TreePop();
				}else ImGui::PopFont();
			}
			
			
			//scan ethercat network
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1.0));
			ImGui::Text("Detected Devices");
			ImGui::PopStyleColor();
			
			ImGui::BeginDisabled(!EtherCatFieldbus::canScan());
			if (ImGui::Button("Scan Network")) EtherCatFieldbus::scan();
			ImGui::EndDisabled();
			
			if (EtherCatFieldbus::isScanning()) {
				ImGui::SameLine();
				auto nic = EtherCatFieldbus::getActiveNetworkInterfaceCard();
				if(nic) ImGui::TextWrapped("Scanning network interface %s", nic->name);
				else ImGui::Text("Scanning...");
			}else if(EtherCatFieldbus::isInitialized()){
				ImGui::PushFont(Fonts::sansBold15);
				ImGui::Text("Devices detected on network interface %s:", EtherCatFieldbus::getActiveNetworkInterfaceCard()->name);
				ImGui::PopFont();
			}
			
			ImGui::TreePush();
			
			for (auto device : EtherCatFieldbus::getUnmatchedDevices()) {
				const char* deviceDisplayName = device->getName().c_str();
				ImGui::Selectable(deviceDisplayName);
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
					ImGui::SetDragDropPayload("DetectedEtherCatDevice", &device, sizeof(std::shared_ptr<EtherCatDevice>));
					ImGui::Text("%s",deviceDisplayName);
					ImGui::EndDragDropSource();
				}
			}
			
			ImGui::TreePop();
			ImGui::PopFont();
		}
		ImGui::PopFont();

		ImGui::PushFont(Fonts::sansBold15);
		if (ImGui::CollapsingHeader("Motion")) {
			
			if(ImGui::TreeNode("Axis")){
				ImGui::PushFont(Fonts::sansRegular15);
				listNodes(NodeFactory::getAllAxisNodes());
				ImGui::TreePop();
				ImGui::PopFont();
			}

			if (ImGui::TreeNode("Machines")) {
				for (auto& category : NodeFactory::getMachinesByCategory()) {
					if (ImGui::TreeNode(category.name)) {
						ImGui::PushFont(Fonts::sansRegular15);
						listNodes(category.nodes);
						ImGui::PopFont();
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
			
			if (ImGui::TreeNode("Safety")) {
				ImGui::PushFont(Fonts::sansRegular15);
				listNodes(NodeFactory::getAllSafetyNodes());
				ImGui::PopFont();
				ImGui::TreePop();
			}
			
		}
		ImGui::PopFont();
	
		ImGui::PushFont(Fonts::sansBold15);
		if (ImGui::CollapsingHeader("Network IO")) {
			ImGui::PushFont(Fonts::sansRegular15);
			listNodes(NodeFactory::getAllNetworkNodes());
			ImGui::PopFont();
		}
		ImGui::PopFont();

		ImGui::PushFont(Fonts::sansBold15);
		if (ImGui::CollapsingHeader("Data Processors")) {
			ImGui::PushFont(Fonts::sansRegular15);
			for (auto category : NodeFactory::getProcessorNodesByCategory()) {
				if (ImGui::TreeNode(category.name)) {
					listNodes(category.nodes);
					ImGui::TreePop();
				}
			}
			ImGui::PopFont();
		}
		ImGui::PopFont();

		ImGui::EndChild();
	
	ImGui::PopStyleVar();
}




std::shared_ptr<Node> nodeAdderContextMenu() {

	std::shared_ptr<Node> output = nullptr;
	
	auto listNodes = [&](const std::vector<std::shared_ptr<Node>>& nodes){
		for (auto node : nodes) {
			if (ImGui::MenuItem(node->getName().c_str())) output = node->duplicate();
		}
	};

	ImGui::MenuItem("Node Editor Menu", nullptr, false, false);
	ImGui::Separator();
	ImGui::MenuItem("EtherCAT devices", nullptr, false, false);
	if (ImGui::BeginMenu("By Manufaturer")) {
		for (auto manufacturer : NodeFactory::getEtherCatDevicesByManufacturer()) {
			if (ImGui::BeginMenu(manufacturer.name)) {
				listNodes(manufacturer.nodes);
				ImGui::EndMenu();
			}
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("By Category")) {
		for (auto category : NodeFactory::getEtherCatDevicesByCategory()) {
			if (ImGui::BeginMenu(category.name)) {
				listNodes(category.nodes);
				ImGui::EndMenu();
			}
		}
		ImGui::EndMenu();
	}

	std::shared_ptr<EtherCatDevice> selectedDetectedSlave = nullptr;
	if (!EtherCatFieldbus::getUnmatchedDevices().empty()) {
		if (ImGui::BeginMenu("Detected Slaves")) {
			for (auto detectedSlave : EtherCatFieldbus::getUnmatchedDevices()) {
				if (ImGui::MenuItem(detectedSlave->getName().c_str())) {
					output = detectedSlave;
					selectedDetectedSlave = detectedSlave;
					EtherCatFieldbus::removeUnmatchedDevice(selectedDetectedSlave);
				}
			}
			ImGui::EndMenu();
		}
	}

	ImGui::Separator();

	if (ImGui::BeginMenu("Motion")) {
		if (ImGui::BeginMenu("Axis")) {
			listNodes(NodeFactory::getAllAxisNodes());
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Machines")) {
			for (auto& category : NodeFactory::getMachinesByCategory()) {
				if (ImGui::BeginMenu(category.name)) {
					listNodes(category.nodes);
					ImGui::EndMenu();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Safety")) {
			listNodes(NodeFactory::getAllSafetyNodes());
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}

	ImGui::Separator();

	if (ImGui::BeginMenu("Network")) {
		listNodes(NodeFactory::getAllNetworkNodes());
		ImGui::EndMenu();
	}

	ImGui::Separator();

	ImGui::MenuItem("Processing Nodes", nullptr, false, false);
	for (auto category : NodeFactory::getProcessorNodesByCategory()) {
		if (ImGui::BeginMenu(category.name)) {
			listNodes(category.nodes);
			ImGui::EndMenu();
		}
	}

	return output;
}



void NodeEditorWindow::onDraw(){
	
	if(!Stacato::Editor::hasCurrentProject()){
		ImGui::TextColored(Colors::gray, "No Project Loaded.");
		return;
	}
	auto project = Stacato::Editor::getCurrentProject();
	auto nodegraph = project->getEnvironnement()->getNodeGraph();
	
	
	//========= NODE INSPECTOR AND ADDER PANEL =========

	 static float sideBarWidth = ImGui::GetTextLineHeight() * 28.0;
	 static float minSideBarWidth = ImGui::GetTextLineHeight() * 5.0;

	 glm::vec2 sideBarSize(sideBarWidth, ImGui::GetContentRegionAvail().y);

	 ImGui::BeginChild("SideBar", sideBarSize);

	
	auto& selectedNodes = nodegraph->getSelectedNodes();
	
	 //if there are no selected nodes, display the Environnement Name Editor and Node adder list
	 if (selectedNodes.empty()) {
		
		 /*
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Environnement Editor");
		ImGui::PopFont();
		
		ImGui::PushFont(Fonts::sansBold15);
		if(ImGui::CollapsingHeader("Project")){
			ImGui::PushFont(Fonts::sansRegular15);
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
		*/
		
		nodeAdder();
		
	 }
	 else if (selectedNodes.size() == 1) {
		std::shared_ptr<Node> selectedNode = selectedNodes.front();
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", selectedNode->getName().c_str());
		ImGui::PopFont();
		ImGui::Separator();
		if (ImGui::BeginChild("NodePropertyChild", ImGui::GetContentRegionAvail())) {
			selectedNode->propertiesGui();
			ImGui::EndChild();
		}
	 }
	 else {
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Multiple Nodes Selected");
		ImGui::PopFont();
		if (ImGui::BeginTabBar("NodeEditorSidePanel")) {
			for (auto node : selectedNodes) {
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
		

	//========= VERTICAL MOVABLE SEPARATOR ==========

	 ImGui::SameLine();
	 sideBarWidth += verticalSeparator(ImGui::GetTextLineHeight() / 3.0);
	 if (sideBarWidth < minSideBarWidth) sideBarWidth = minSideBarWidth;
			  
	//========== NODE EDITOR BLOCK ============

	 if(ImGui::GetContentRegionAvail().x <= 0.0) return;
	 
	 ImGui::BeginChild("NodeGraphEditor", ImGui::GetContentRegionAvail());

		//Draw the entire node editor
		glm::vec2 nodeEditorSize(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight() * 1.7);
		nodegraph->editorGui(nodeEditorSize);

		if (ImGui::Button("Center View")) nodegraph->centerView();
		ImGui::SameLine();
		if (ImGui::Button("Show Flow")) nodegraph->showFlow();
		//ImGui::SameLine();
		//ImGui::Checkbox("Show Output Values", &Environnement::NodeGraph::getShowOutputValues());
		//ImGui::SameLine();
		//if (!Environnement::isRunning()) ImGui::TextColored(Colors::gray, "Editing is disabled while the environnement is running");
	 
	 ImGui::EndChild();
	
}



}
