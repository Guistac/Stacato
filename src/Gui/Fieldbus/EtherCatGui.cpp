#include <pch.h>

#include "EtherCatGui.h"

#include <imgui.h>

void etherCatGui() {

	if (ImGui::BeginTabBar("##FieldbusTabBar")) {
		if (ImGui::BeginTabItem("Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Parameters")) {
			etherCatParameters();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

}
