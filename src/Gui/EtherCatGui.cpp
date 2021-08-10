#include "EtherCatGui.h"

#include "Fieldbus/EtherCatFieldbus.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include <iostream>

#include "Gui.h"

bool etherCatGui() {

	ImGui::Begin("EtherCAT Fieldbus Test Program", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	bool disableButton = EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting;
	if (disableButton) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0));
	}
	if (ImGui::Button("Start")) EtherCatFieldbus::start();
	if (disableButton) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();

	disableButton = !EtherCatFieldbus::b_processRunning;
	if (disableButton) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0));
	}
	if (ImGui::Button("Stop")) EtherCatFieldbus::stop();
	if (disableButton) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	
	ImGui::SameLine();
	bool exitFieldbus = false;
	if (ImGui::Button("Select Network Interface Card")) exitFieldbus = true;
	ImGui::Separator();

	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("EtherCAT Slaves")) {
			etherCatSlaves();
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Fieldbus Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		static bool isTabOpen = false;
		bool wasTabOpen = isTabOpen;
		if (ImGui::BeginTabItem("Fieldbus Parameters")) {
			isTabOpen = true;
			etherCatParameters(isTabOpen && !wasTabOpen);
			ImGui::EndTabItem();
		}
		else {
			isTabOpen = false;
		}

		ImGui::EndTabBar();
	}
	
	ImGui::End();

	return !exitFieldbus;
}
