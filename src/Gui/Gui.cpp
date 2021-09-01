#include <pch.h>

#include "Gui.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Fieldbus/EtherCatFieldbus.h"

void gui() {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
	ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);


	ImGui::Begin("EtherCAT Fieldbus Test Program", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	bool disableButton = EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting;
	if (disableButton) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0)); }
	if (ImGui::Button("Start")) {
		EtherCatFieldbus::start();
		ImGui::OpenPopup("Starting EtherCAT Fieldbus");
	}
	if (disableButton) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }
	ImGui::SameLine();

	disableButton = !EtherCatFieldbus::b_processRunning;
	if (disableButton) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0)); }
	if (ImGui::Button("Stop")) EtherCatFieldbus::stop();
	if (disableButton) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }

	ImGui::Separator();

	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("EtherCAT Slaves")) {
			etherCatSlaves();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Node Graph")) {
			nodeGraph();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Fieldbus Metrics")) {
			etherCatMetrics();
			ImGui::EndTabItem();
		}
		//we need to detect when the tab was just opened to set some variables in the parameters
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
		if (ImGui::BeginTabItem("Log")) {
			log();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	modals();

	ImGui::End();
}


void modals() {
	etherCatStartModal();
}


