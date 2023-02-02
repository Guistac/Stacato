#include <pch.h>

#include "ModularDeviceProfile.h"

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

namespace EtherCAT::ModularDeviceProfile{

void ModularDevice::moduleManagerGui(){
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Modules");
	ImGui::PopFont();
	
	ImGui::SameLine();
	ImGui::BeginDisabled(!isDetected());
	if(ImGui::Button("Discover Modules")) discoverDeviceModules();
	ImGui::EndDisabled();
	
	ImGui::SameLine();
	ImGui::Text("%s", Enumerator::getDisplayString(moduleDiscoveryStatus));
	
	ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_SizingFixedFit;
	
	if(ImGui::BeginTable("##VipaModuleTable", 4, tableFlags)){
		
		ImGui::TableSetupColumn("Manage");
		ImGui::TableSetupColumn("Index");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		
		std::shared_ptr<DeviceModule> deletedModule = nullptr;
		std::shared_ptr<DeviceModule> movedUpModule = nullptr;
		std::shared_ptr<DeviceModule> movedDownModule = nullptr;
		
		float rowHeight = ImGui::GetFrameHeight() + ImGui::GetTextLineHeight() * 0.2;
		
		for(int i = 0; i < modules.size(); i++){
			
			ImGui::PushID(i);
			
			std::shared_ptr<DeviceModule> deviceModule = modules[i];
			ImGui::TableNextRow();
			if(deviceModule == selectedModule) ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImColor(Colors::blue));
			
			ImGui::TableSetColumnIndex(0);
			float spacing = ImGui::GetTextLineHeight() * 0.1;
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(spacing));
			if (buttonCross("##remove")) deletedModule = deviceModule;
			ImGui::SameLine();
			bool disableButton = i == 0;
			ImGui::BeginDisabled(disableButton);
			if (ImGui::ArrowButton("##moveUp", ImGuiDir_Up)) movedUpModule = deviceModule;
			ImGui::EndDisabled();
			ImGui::SameLine();
			disableButton = i == modules.size() - 1;
			ImGui::BeginDisabled(disableButton);
			if (ImGui::ArrowButton("##moveDown", ImGuiDir_Down)) movedDownModule = deviceModule;
			ImGui::EndDisabled();
			ImGui::PopStyleVar();
			
			ImGui::TableSetColumnIndex(1);
			static char indexLabelString[16];
			sprintf(indexLabelString, "%i", deviceModule->moduleIndex);
			ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns;
			if (ImGui::Selectable(indexLabelString, false, selectable_flags, ImVec2(0.0, 0.0))){
				selectedModule = deviceModule;
			}
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("%s", deviceModule->getDisplayName());
			ImGui::TableSetColumnIndex(3);
			ImGui::Text("%s", deviceModule->getSaveName());
			
			ImGui::PopID();
		}
		
		ImGui::TableNextRow();
		
		ImGui::TableSetColumnIndex(0);
		if(ImGui::Button("Add Module")) ImGui::OpenPopup("ModuleAdderPopup");
		
		if (ImGui::BeginPopup("ModuleAdderPopup")) {
			for (auto deviceModule : getModuleFactory()) {
				static char moduleItemString[128];
				sprintf(moduleItemString, "%s (%s)", deviceModule->getDisplayName(), deviceModule->getSaveName());
				if(ImGui::MenuItem(moduleItemString)){
					addModule(deviceModule->getNewInstance());
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
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", selectedModule->getDisplayName());
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansRegular20);
		ImGui::Text("%s", selectedModule->getSaveName());
		ImGui::PopFont();
		selectedModule->moduleGui();
	}

	
	
	
	
	
	
	
	
	
}


};
