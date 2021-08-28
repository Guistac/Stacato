#include <pch.h>
#include "EasyCAT.h"

#include <imgui.h>

void EasyCAT::deviceSpecificGui() {
	if (ImGui::BeginTabItem("EasyCAT Configuration")) {
		ImGui::Text("Hello From EasyCAT!");

		ImGui::EndTabItem();
	}
}