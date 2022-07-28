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

