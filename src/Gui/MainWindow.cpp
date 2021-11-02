#include <pch.h>

#include "Gui.h"

#include "Plot/PlotGui.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatGui.h"

void mainWindow() {

	ImGui::PushStyleColor(ImGuiCol_TabActive, glm::vec4(0.6, 0.4, 0.0, 1.0));

	if (ImGui::BeginTabBar("MainTabBar")) {

		if (ImGui::BeginTabItem("Node Graph")) {
			nodeGraph();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Fieldbus")) {
			etherCatGui();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Log")) {
			log();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Environnement")) {
			environnementGui();
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

	etherCatStartModal();

	ImGui::PopStyleColor();
}