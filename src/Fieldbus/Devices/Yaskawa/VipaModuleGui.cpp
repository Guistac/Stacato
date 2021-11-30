#include <pch.h>

#include "VipaModule.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"


void VipaModule::moduleGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("%s", getDisplayName());
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::robotoRegular20);
	ImGui::Text("%s", getSaveName());
	ImGui::PopFont();
	
	if(ImGui::BeginTabBar("##ModuleTabBar")){
		
		if(ImGui::BeginTabItem("Information")){
			ImGui::BeginChild("##InformationChild");
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Settings")){
			ImGui::BeginChild("##SettingsChild");
			moduleSpecificGui();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}
}

void VipaModule::moduleSpecificGui(){
	ImGui::Text("No Settings Are Available for this SLIO Module");
}
