#include <pch.h>

#include "Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatGui.h"
#include "Project/Environnement.h"

#include "Framework/Colors.h"

void toolbar(float height) {

	glm::vec2 buttonSize(ImGui::GetTextLineHeight() * 4.0, ImGui::GetTextLineHeight() * 2.0);

	bool disableStartButton = EtherCatFieldbus::isCyclicExchangeStarting();
	if (disableStartButton) BEGIN_DISABLE_IMGUI_ELEMENT
	if (!EtherCatFieldbus::isCyclicExchangeActive()) {
		if (ImGui::Button("Start", buttonSize)) {
			EtherCatFieldbus::start();
			ImGui::OpenPopup("Starting EtherCAT Fieldbus");
		}
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		if (ImGui::Button("Stop", buttonSize)) EtherCatFieldbus::stop();
		ImGui::PopStyleColor();
	}
	if (disableStartButton) END_DISABLE_IMGUI_ELEMENT

	ImGui::SameLine();

	bool disableScan = EtherCatFieldbus::isCyclicExchangeActive();
	if(disableScan) BEGIN_DISABLE_IMGUI_ELEMENT
	if (ImGui::Button("Scan", buttonSize)) EtherCatFieldbus::scanNetwork();
	if (disableScan) END_DISABLE_IMGUI_ELEMENT

	ImGui::SameLine();
	
	bool disableMachineToggleButton = !EtherCatFieldbus::isCyclicExchangeActive();

	if(disableMachineToggleButton) BEGIN_DISABLE_IMGUI_ELEMENT
	if (Environnement::areAllMachinesEnabled()) {
		ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
		if (ImGui::Button("Disable All", buttonSize)) {
			Environnement::disableAllMachines();
		}
	}
	else {
		if (disableMachineToggleButton) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Button]);
		else if(Environnement::areNoMachinesEnabled()) ImGui::PushStyleColor(ImGuiCol_Button, Colors::blue);
		else ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
		if (ImGui::Button("Enable All", buttonSize)) {
			Environnement::enableAllMachines();
		}
	}
	if(disableMachineToggleButton) END_DISABLE_IMGUI_ELEMENT

	ImGui::PopStyleColor();


	etherCatStartModal();
}