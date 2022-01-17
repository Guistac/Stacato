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
			informationGui();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Settings")){
			ImGui::BeginChild("##SettingsChild");
			moduleParameterGui();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}
}

void VipaModule::informationGui(){
	if(!inputPins.empty()){
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Outputs");
		ImGui::PopFont();
		for(auto& inputPin : inputPins){
			ImGui::Text("%s", inputPin->getDisplayString());
		}
	}
	if(!outputPins.empty()){
		ImGui::PushFont(Fonts::robotoBold20);
		ImGui::Text("Inputs");
		ImGui::PopFont();
		for(auto& outputPin : outputPins){
			ImGui::Text("%s", outputPin->getDisplayString());
		}
	}
}

void VipaModule::moduleParameterGui(){
	ImGui::Text("No Settings Are Available for this SLIO Module");
}
