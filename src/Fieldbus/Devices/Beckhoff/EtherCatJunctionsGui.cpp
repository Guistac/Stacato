#include <pch.h>

#include "EtherCatJunctions.h"

#include <imgui.h>
#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

void CU1124::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1124")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void CU1128::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1128")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EK1100::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1128")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}

void EL2008::deviceSpecificGui() {
	if(ImGui::BeginTabItem("EL2008")){
		
		for(int i = 0; i < 8; i++){
			ImGui::PushID(i);
			
			char name[32];
			snprintf(name, 32, "Channel %i", i + 1);
			ImGui::Checkbox(name, &outputs[i]);
			
			ImGui::PopID();
		}
		
		ImGui::EndTabItem();
	}
}

void EL5001::deviceSpecificGui() {
	if(ImGui::BeginTabItem("CU1128")){
		ImGui::Text("This device has no settings or controls");
		ImGui::EndTabItem();
	}
}
