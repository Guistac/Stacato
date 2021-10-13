#include <pch.h>

#include "Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatGui.h"

#include "Framework/Colors.h"

void toolbar(float height) {

	glm::vec2 buttonSize(100.0, ImGui::GetTextLineHeight() * 2.0);

	bool disableStartButton = EtherCatFieldbus::isCyclicExchangeStarting();
	if (disableStartButton) BEGIN_DISABLE_IMGUI_ELEMENT
	if (!EtherCatFieldbus::isCyclicExchangeActive()) {
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
	if (disableStartButton) END_DISABLE_IMGUI_ELEMENT

	ImGui::SameLine();

	bool disableScan = EtherCatFieldbus::isCyclicExchangeActive();
	if(disableScan) BEGIN_DISABLE_IMGUI_ELEMENT
	if (ImGui::Button("Scan", buttonSize)) EtherCatFieldbus::scanNetwork();
	if(disableScan) END_DISABLE_IMGUI_ELEMENT

	etherCatStartModal();
}