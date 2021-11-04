#include <pch.h>

#include "Gui.h"

#include "Plot/PlotGui.h"
#include "Environnement/Environnement.h"

void mainWindow() {

	ImGui::PushStyleColor(ImGuiCol_TabActive, glm::vec4(0.6, 0.4, 0.0, 1.0));

	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("Environnement")) {
			Environnement::gui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Machines")) {
			machineListGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Plot")) {
			plotGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Stage View")) {
			ImGui::Text("There will be a stage 2D or 3D model here");
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::PopStyleColor();
}