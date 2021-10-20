#include <pch.h>

#include "Gui/Gui.h"
#include "Environnement/Environnement.h"
#include "Motion/Machine/Machine.h"
#include "Gui/Framework/Colors.h"

#include <imgui.h>

void machineListGui() {
	/*
	glm::vec2 maxMachineGuiSize(ImGui::GetTextLineHeight() * 30.0, ImGui::GetTextLineHeight() * 10.0);
	glm::vec2 minMachineGuiSize(ImGui::GetTextLineHeight() * 15.0, ImGui::GetTextLineHeight() * 5.0);
	glm::vec2 availableSpace = ImGui::GetContentRegionAvail();
	std::vector<std::shared_ptr<Machine>> machines = Environnement::getMachines();

	int maxHorizontalBoxes = std::floor(availableSpace.x / minMachineGuiSize.x);
	int minHorizontalBoxes = std::floor(availableSpace.x / maxMachineGuiSize.x);
	int maxVerticalBoxes = std::floor(availableSpace.y / minMachineGuiSize.y);
	int minVerticalBoxes = std::floor(availableSpace.y / maxMachineGuiSize.y);

	
	glm::vec2 machineUiSize(ImGui::GetTextLineHeight() * 20.0, ImGui::GetTextLineHeight() * 5.0);
	for (auto machine : Environnement::getMachines()) {
		if (ImGui::BeginChild(machine->getName(), machineUiSize, true)) {
			ImGui::PushFont(Fonts::robotoBold20);
			ImGui::Text("%s", machine->getName());
			ImGui::PopFont();

			glm::vec2 buttonSize;
			buttonSize.x = ImGui::GetTextLineHeight() * 10.0;
			buttonSize.y = ImGui::GetTextLineHeight() * 2.0;

			bool isMachineReady = machine->isReady();

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			if (machine->isEnabled()) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				ImGui::Button("Enabled", buttonSize);
			}
			else if (isMachineReady) {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::yellow);
				ImGui::Button("Ready", buttonSize);
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::red);
				ImGui::Button("Not Ready", buttonSize);
			}
			ImGui::PopStyleColor();
			ImGui::PopItemFlag();

			ImGui::SameLine();
			if (!isMachineReady) BEGIN_DISABLE_IMGUI_ELEMENT
				if (machine->isEnabled()) {
					if (ImGui::Button("Disable", buttonSize)) machine->disable();
				}
				else {
					if (ImGui::Button("Enable", buttonSize)) machine->enable();
				}
			if (!isMachineReady) END_DISABLE_IMGUI_ELEMENT

			//machine->miniatureGui();

			ImGui::EndChild();
		}
	}
	
	*/
}