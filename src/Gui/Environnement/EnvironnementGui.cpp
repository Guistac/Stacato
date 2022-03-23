#include <pch.h>

#include <imgui.h>
#include <imgui_internal.h>

#include "EnvironnementGui.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"


//#include "ProjectGui.h"

#include "Environnement/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Fieldbus/EtherCatDevice.h"
#include "Gui/Utilities/DraggableList.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Gui/Fieldbus/EtherCatGui.h"

#include "Gui/Utilities/Log.h"

#include "Environnement/NodeGraph.h"
#include "Gui/Environnement/NodeEditor/NodeEditorGui.h"

#include "Project/Project.h"

#include <imgui_node_editor.h>

namespace Environnement::Gui{

	void gui() {
		if (ImGui::BeginTabBar("##EnvironnementTabBar")) {
			if (ImGui::BeginTabItem("Editor")) {
				editor();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Machine Manager")) {
				machineManager();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Device Manager")) {
				deviceManager();
				ImGui::EndTabItem();
			}
			/*
			if (ImGui::BeginTabItem("Stage Editor")) {
				stageEditorGui();
				ImGui::Text("3D Scene Graph / Geometry importer / Machine movement assignement");
				ImGui::EndTabItem();
			}
			 */
			if (ImGui::BeginTabItem("Fieldbus")) {
				etherCatGui();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Log")) {
				Utilies::Gui::log();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}


	void machineManager(){
		
		static float listWidth = ImGui::GetTextLineHeight() * 15.0;
		static float minlistWidth = ImGui::GetTextLineHeight() * 10.0;
		
		static DraggableList machineList;
		std::shared_ptr<Machine> clickedMachine = nullptr;
		
		ImGui::BeginGroup();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Machine Manager");
		ImGui::PopFont();
		
		std::vector<std::shared_ptr<Machine>>& machines = Environnement::getMachines();
		if (machineList.beginList("##MachineList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
			for (int i = 0; i < machines.size(); i++) {
				std::shared_ptr<Machine> machine = machines[i];
				if (machineList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()), machine == Environnement::selectedMachine)) {

					ImGui::PushFont(Fonts::robotoBold15);
					float shortNameWidth = ImGui::CalcTextSize(machine->shortName).x;
					ImGui::PopFont();
					
					float headerStripWidth = shortNameWidth + ImGui::GetStyle().ItemSpacing.x;
					glm::vec2 min = ImGui::GetWindowPos();
					glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
					glm::vec4 headerStripColor;
					
					if(machine->isEnabled()) headerStripColor = Colors::green;
					else if(machine->isReady()) headerStripColor = Colors::yellow;
					else headerStripColor = Colors::blue;
					
					ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);

					ImGui::PushFont(Fonts::robotoBold15);
					ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
					ImGui::Text("%s", machine->shortName);
					ImGui::PopFont();

					ImGui::SameLine();
					ImGui::Text("%s", machine->getName());
					if (machineList.endItem()) clickedMachine = machine;
				}
			}
			machineList.endList();
		}
		if (machineList.wasReordered()) {
			int oldIndex, newIndex;
			machineList.getReorderedItemIndex(oldIndex, newIndex);
			std::shared_ptr<Machine> tmp = machines[oldIndex];
			machines.erase(machines.begin() + oldIndex);
			machines.insert(machines.begin() + newIndex, tmp);
		}
		if(clickedMachine) Environnement::selectedMachine = clickedMachine;
		
		ImGui::EndGroup();
		
		
		
		
		static float splitterWidth = ImGui::GetTextLineHeight() * 0.5;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::InvisibleButton("VerticalSplitter", glm::vec2(splitterWidth, ImGui::GetContentRegionAvail().y));
		if (ImGui::IsItemActive()) listWidth += ImGui::GetIO().MouseDelta.x;
		if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (listWidth < minlistWidth) listWidth = minlistWidth;
		
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float middle = (min.x + max.x) / 2.0;
		ImGui::GetWindowDrawList()->AddLine(glm::vec2(middle, min.y), glm::vec2(middle, max.y), ImColor(Colors::darkGray), ImGui::GetTextLineHeight() * 0.1);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		
		
		
		
		ImGui::SameLine();
		
		ImGui::BeginChild("##selectedmachine", ImGui::GetContentRegionAvail());
			
		if(Environnement::selectedMachine){
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("%s", Environnement::selectedMachine->getName());
			ImGui::PopFont();
			if(ImGui::BeginTabBar("selectedMachineTabBar")){
				Environnement::selectedMachine->nodeSpecificGui();
				ImGui::EndTabBar();
			}
		}else{
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("No Machine Selected.");
			ImGui::PopFont();
		}
		
		ImGui::EndChild();
		
	}


	void deviceManager() {
		
		static float listWidth = ImGui::GetTextLineHeight() * 15.0;
		static float minlistWidth = ImGui::GetTextLineHeight() * 15.0;
		
		static DraggableList etherCatDeviceList;
		std::vector<std::shared_ptr<EtherCatDevice>>& etherCatDevices = Environnement::getEtherCatDevices();
		
		std::shared_ptr<EtherCatDevice> clickedDevice = nullptr;
		
		ImGui::BeginGroup();
		
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("EtherCat Device Manager");
		ImGui::PopFont();
		
		if (etherCatDeviceList.beginList("##EtherCatDeviceList", glm::vec2(listWidth, ImGui::GetContentRegionAvail().y), 4.0)) {
			for (int i = 0; i < etherCatDevices.size(); i++) {
				std::shared_ptr<EtherCatDevice> etherCatDevice = etherCatDevices[i];
				if (etherCatDeviceList.beginItem(glm::vec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing()))) {
					
					ImGui::PushFont(Fonts::robotoBold15);
					static char numberString[16];
					if(!etherCatDevice->isDetected()) sprintf(numberString, "#-");
					else sprintf(numberString, "#%i", etherCatDevice->getSlaveIndex());
					
					float headerStripWidth = ImGui::CalcTextSize(numberString).x + ImGui::GetStyle().ItemSpacing.x;
					glm::vec2 min = ImGui::GetWindowPos();
					glm::vec2 max = min + glm::vec2(headerStripWidth, ImGui::GetWindowSize().y);
					glm::vec4 headerStripColor;
					
					if(etherCatDevice->isConnected()) headerStripColor = Colors::green;
					else if(etherCatDevice->isDetected()) headerStripColor = Colors::yellow;
					else headerStripColor = Colors::blue;
					
					ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImColor(headerStripColor), 10.0, ImDrawFlags_RoundCornersLeft);
				
					ImGui::SameLine(ImGui::GetStyle().ItemSpacing.x / 2.0);
					int slaveIndex = etherCatDevice->getSlaveIndex();
					if(!etherCatDevice->isDetected()) ImGui::PushStyleColor(ImGuiCol_Text, Colors::darkGray);
					else ImGui::PushStyleColor(ImGuiCol_Text, Colors::white);
					ImGui::Text("%s", numberString);
					ImGui::PopStyleColor();
					ImGui::PopFont();

					ImGui::SameLine();
					ImGui::Text("%s", etherCatDevice->getName());
					
					
					if(etherCatDeviceList.endItem()) clickedDevice = etherCatDevice;
				}
			}
			etherCatDeviceList.endList();
		}
		if(clickedDevice) Environnement::selectedEtherCatDevice = clickedDevice;
		
		ImGui::EndGroup();
		
		
		static float splitterWidth = ImGui::GetTextLineHeight() * 0.5;
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::SameLine();
		ImGui::InvisibleButton("VerticalSplitter", glm::vec2(splitterWidth, ImGui::GetContentRegionAvail().y));
		if (ImGui::IsItemActive()) listWidth += ImGui::GetIO().MouseDelta.x;
		if (ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		if (listWidth < minlistWidth) listWidth = minlistWidth;
		
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		float middle = (min.x + max.x) / 2.0;
		ImGui::GetWindowDrawList()->AddLine(glm::vec2(middle, min.y), glm::vec2(middle, max.y), ImColor(Colors::darkGray), ImGui::GetTextLineHeight() * 0.1);
		ImGui::SameLine();
		ImGui::PopStyleVar();
		
		
		
		ImGui::SameLine();
		
		ImGui::BeginChild("##selectedDevice", ImGui::GetContentRegionAvail());
			
		if(Environnement::selectedEtherCatDevice){
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("%s", Environnement::selectedEtherCatDevice->getName());
			ImGui::PopFont();
			if(ImGui::BeginTabBar("selectedDeviceTabBar")){
				Environnement::selectedEtherCatDevice->nodeSpecificGui();
				ImGui::EndTabBar();
			}
		}else{
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("No Device Selected.");
			ImGui::PopFont();
		}
		
		ImGui::EndChild();
		
	}









void editor() {

	   //========= NODE INSPECTOR AND ADDER PANEL =========

	   static float sideBarWidth = ImGui::GetTextLineHeight() * 28.0;
	   static float minSideBarWidth = ImGui::GetTextLineHeight() * 5.0;

	   glm::vec2 sideBarSize(sideBarWidth, ImGui::GetContentRegionAvail().y);
	   if (ImGui::BeginChild("SideBar", sideBarSize)) {
		   
		   std::vector<std::shared_ptr<Node>> selectedNodes = NodeGraph::getSelectedNodes();
		   //if there are no selected nodes, display the Environnement Name Editor and Node adder list
		   if (selectedNodes.empty()) {
			   
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
			   
			   NodeGraph::Gui::nodeAdder();
			   
		   }
		   else if (selectedNodes.size() == 1) {
			   std::shared_ptr<Node> selectedNode = selectedNodes.front();
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
	   Environnement::NodeGraph::Gui::editor();

	   if (Project::isEditingAllowed()) {
		   std::shared_ptr<Node> newDraggedNode = NodeGraph::Gui::acceptDraggedNode();
		   if (newDraggedNode) {
			   Environnement::NodeGraph::addNode(newDraggedNode);
			   Environnement::addNode(newDraggedNode);
			   ax::NodeEditor::SetNodePosition(newDraggedNode->getUniqueID(), ax::NodeEditor::ScreenToCanvas(ImGui::GetMousePos()));
		   }
	   }

	   if (ImGui::Button("Center View")) Environnement::NodeGraph::Gui::centerView();
	   ImGui::SameLine();
	   if (ImGui::Button("Show Flow")) Environnement::NodeGraph::Gui::showFlow();
	   ImGui::SameLine();
	   ImGui::Checkbox("Show Output Values", &Environnement::NodeGraph::Gui::getShowOutputValues());
	   ImGui::SameLine();

	   if (!Project::isEditingAllowed()) ImGui::TextColored(Colors::gray, "Editing is disabled while the environnement is running");

	   ImGui::EndGroup();

   }








}
