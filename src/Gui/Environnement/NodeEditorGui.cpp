#include <pch.h>

#include <imgui.h>
#include <imgui_node_editor.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Environnement/NodeGraph/NodeGraphGui.h"

#include "Environnement/Environnement.h"
#include "Environnement/NodeGraph/NodeGraph.h"
#include "Project/Project.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "EnvironnementGui.h"

namespace Environnement::Gui{

void NodeEditorWindow::drawContent(){
	
	//========= NODE INSPECTOR AND ADDER PANEL =========

	 static float sideBarWidth = ImGui::GetTextLineHeight() * 28.0;
	 static float minSideBarWidth = ImGui::GetTextLineHeight() * 5.0;

	 glm::vec2 sideBarSize(sideBarWidth, ImGui::GetContentRegionAvail().y);

	 ImGui::BeginChild("SideBar", sideBarSize);

	 std::vector<std::shared_ptr<Node>> selectedNodes = NodeGraph::getSelectedNodes();
	 //if there are no selected nodes, display the Environnement Name Editor and Node adder list
	 if (selectedNodes.empty()) {
		
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
		
		NodeGraph::Gui::nodeAdder();
		
	 }
	 else if (selectedNodes.size() == 1) {
		std::shared_ptr<Node> selectedNode = selectedNodes.front();
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", selectedNode->getName());
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
		Environnement::NodeGraph::Gui::editor(nodeEditorSize);

		if (ImGui::Button("Center View")) Environnement::NodeGraph::Gui::centerView();
		ImGui::SameLine();
		if (ImGui::Button("Show Flow")) Environnement::NodeGraph::Gui::showFlow();
		ImGui::SameLine();
		ImGui::Checkbox("Show Output Values", &Environnement::NodeGraph::getShowOutputValues());
		ImGui::SameLine();
		if (!Project::isEditingAllowed()) ImGui::TextColored(Colors::gray, "Editing is disabled while the environnement is running");
	 
	 ImGui::EndChild();

	
}

}
