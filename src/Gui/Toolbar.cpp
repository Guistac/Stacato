#include <pch.h>

#include "Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "EtherCAT/EtherCatGui.h"

#include "Framework/Colors.h"

void toolbar(float height) {

	glm::vec2 buttonSize(100.0, ImGui::GetTextLineHeight() * 2.0);

	bool disableButton = EtherCatFieldbus::b_processStarting;
	if (disableButton) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray); }
	if (!EtherCatFieldbus::b_processRunning) {
		if (ImGui::Button("Start", buttonSize)) {
			EtherCatFieldbus::start();
			ImGui::OpenPopup("Starting EtherCAT Fieldbus");
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.6f, 0.1f, 1.0f));
		if (ImGui::Button("Stop", buttonSize)) EtherCatFieldbus::stop();
		ImGui::PopStyleColor();
	}
	if (disableButton) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }

	ImGui::SameLine();

	bool disableScan = EtherCatFieldbus::b_processRunning;
	if(disableScan) { ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true); ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray); }
	if (ImGui::Button("Scan", buttonSize)) EtherCatFieldbus::scanNetwork();
	if(disableScan) { ImGui::PopItemFlag(); ImGui::PopStyleColor(); }

	etherCatStartModal();
}