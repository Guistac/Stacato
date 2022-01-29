#include <pch.h>

#include "VIPA-053-1EC01.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "VipaModule.h"

void VipaBusCoupler_053_1EC01::deviceSpecificGui() {
    if (ImGui::BeginTabItem("VIPA")) {
        
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("SLIO Modules");
		ImGui::PopFont();
		
		ImGui::SameLine();
		if(ImGui::Button("Auto Detect Modules")){
			configureFromDeviceModules();
		}
		
		ImGui::SameLine();
		ImGui::Text("%s", Enumerator::getDisplayString(configureFromDeviceModulesDownloadStatus));
		
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
		
		if(ImGui::BeginTable("##VipaModuleTable", 4, tableFlags)){
			
			ImGui::TableSetupColumn("Manage");
			ImGui::TableSetupColumn("Index");
			ImGui::TableSetupColumn("Type");
			ImGui::TableSetupColumn("Name");
			ImGui::TableHeadersRow();
			
			std::shared_ptr<VipaModule> deletedModule = nullptr;
			std::shared_ptr<VipaModule> movedUpModule = nullptr;
			std::shared_ptr<VipaModule> movedDownModule = nullptr;
			
			float rowHeight = ImGui::GetFrameHeight() + ImGui::GetTextLineHeight() * 0.2;
			
			for(int i = 0; i < modules.size(); i++){
				
				ImGui::PushID(i);
				
				std::shared_ptr<VipaModule> module = modules[i];
				ImGui::TableNextRow();
				if(module == selectedModule) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(Colors::blue));
				
				ImGui::TableSetColumnIndex(0);
				float spacing = ImGui::GetTextLineHeight() * 0.1;
				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(spacing));
				if (buttonCross("##remove")) deletedModule = module;
				ImGui::SameLine();
				bool disableButton = i == 0;
				if(disableButton) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpModule = module;
				if(disableButton) END_DISABLE_IMGUI_ELEMENT
				ImGui::SameLine();
				disableButton = i == modules.size() - 1;
				if(disableButton) BEGIN_DISABLE_IMGUI_ELEMENT
				if (ImGui::ArrowButton("##moveDown", ImGuiDir_Down)) movedDownModule = module;
				if(disableButton) END_DISABLE_IMGUI_ELEMENT
				ImGui::PopStyleVar();
				
				ImGui::TableSetColumnIndex(1);
				static char indexLabelString[16];
				sprintf(indexLabelString, "%i", module->moduleIndex);
				ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns;
				if (ImGui::Selectable(indexLabelString, false, selectable_flags, ImVec2(0.0, 0.0))){
					selectedModule = module;
				}
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", module->getDisplayName());
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%s", module->getSaveName());
				
				ImGui::PopID();
			}
			
			ImGui::TableNextRow();
			
			ImGui::TableSetColumnIndex(0);
			if(ImGui::Button("Add Module")) ImGui::OpenPopup("ModuleAdderPopup");
			if (ImGui::BeginPopup("ModuleAdderPopup")) {
				for (auto& module : VipaModuleFactory::getModules()) {
					static char moduleItemString[128];
					sprintf(moduleItemString, "%s (%s)", module->getDisplayName(), module->getSaveName());
					if(ImGui::MenuItem(moduleItemString)){
						addModule(module->getInstance());
					}
				}
				ImGui::EndPopup();
			}
			if(modules.empty()){
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("No Modules");
			}
		
			if(deletedModule) removeModule(deletedModule);
			if(movedUpModule) moveModuleUp(movedUpModule);
			if(movedDownModule) moveModuleDown(movedDownModule);
			
			ImGui::EndTable();
		}
		
		ImGui::Separator();
		
		if(selectedModule){
			selectedModule->moduleGui();
		}
	
		
		
		
		
		
		
		
		
        ImGui::EndTabItem();
    }
}
