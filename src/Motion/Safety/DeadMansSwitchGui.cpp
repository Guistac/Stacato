#include <pch.h>

#include "DeadMansSwitch.h"

#include <imgui.h>

void DeadMansSwitch::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Controls")){
		
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Settings")){
		
		ImGui::EndTabItem();
	}
}

void DeadMansSwitch::controlsGui(){}
void DeadMansSwitch::settingsGui(){}
