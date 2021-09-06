#include <pch.h>

#include "Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"

void mainWindow() {
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
		if (ImGui::BeginTabItem("Sequencer")) {
			sequencer();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	etherCatStartModal();
}