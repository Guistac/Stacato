#include <pch.h>

#include "EtherCatGui.h"

#include <imgui.h>
#include <imgui_internal.h>

void etherCatGui() {

	if (ImGui::BeginTabBar("##FieldbusTabBar")) {

		if (ImGui::BeginTabItem("EtherCAT Slaves")) {
			etherCatSlaves();
			ImGui::EndTabItem();
		}
		
		if (ImGui::BeginTabItem("Fieldbus Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		
		//we need to detect when the tab was just opened to set some variables in the parameters
		static bool isTabOpen = false;
		if (ImGui::BeginTabItem("Fieldbus Parameters")) {
			etherCatParameters(!isTabOpen);
			isTabOpen = true;
			ImGui::EndTabItem();
		}
		else isTabOpen = false;

		ImGui::EndTabBar();
	}

}