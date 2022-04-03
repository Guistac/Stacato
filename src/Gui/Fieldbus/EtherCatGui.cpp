#include <pch.h>

#include "EtherCatGui.h"

#include <imgui.h>

void etherCatGui() {

	if (ImGui::BeginTabBar("##FieldbusTabBar")) {

		/*
		if (ImGui::BeginTabItem("Devices")) {
			etherCatSlaves();
			ImGui::EndTabItem();
		}
		*/
		
		if (ImGui::BeginTabItem("Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		
		//we need to detect when the tab was just opened to set some variables in the parameters
		static bool isTabOpen = false;
		if (ImGui::BeginTabItem("Parameters")) {
			etherCatParameters(!isTabOpen);
			isTabOpen = true;
			ImGui::EndTabItem();
		}
		else isTabOpen = false;

		ImGui::EndTabBar();
	}

}
