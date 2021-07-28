#include "EtherCatGui.h"

#include "Fieldbus/EtherCAT/EtherCatFieldbus.h"

#include "imgui.h"
#include "imgui_internal.h"

bool etherCatGui() {

	ImGui::Begin("EtherCAT Fieldbus Test Program");

	if (ImGui::Button("Scan Network")) EtherCatFieldbus::scanNetwork();
	ImGui::SameLine();
	if (!EtherCatFieldbus::b_networkScanned) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
	}
	if (ImGui::Button("Configure Slaves")) EtherCatFieldbus::configureSlaves();
	if (!EtherCatFieldbus::b_networkScanned) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();
	if (!EtherCatFieldbus::b_ioMapConfigured) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
	}
	if (ImGui::Button("Start Cyclic Exchange")) EtherCatFieldbus::startCyclicExchange();
	ImGui::SameLine();
	if (ImGui::Button("Stop Cyclic Exchange")) EtherCatFieldbus::stopCyclicExchange();
	if (!EtherCatFieldbus::b_ioMapConfigured) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

	if (EtherCatFieldbus::b_networkScanned) {
		for (EtherCatSlave& slave : EtherCatFieldbus::slaves) {
			if (ImGui::TreeNode(slave.displayName)) {
				ImGui::Text("Address: %i", slave.address);
				if (!slave.b_configured) ImGui::Text("Not Configured");
				else {
					ImGui::Text("Input Bytes: %i (%i bits)", slave.slave_ptr->Ibytes, slave.slave_ptr->Ibits);
					ImGui::Text("Output Bytes: %i (%i bits)", slave.slave_ptr->Obytes, slave.slave_ptr->Obits);
				}
				ImGui::TreePop();
			}
		}
	}

	bool exitFieldbus = false;
	if (ImGui::Button("Return to Fieldbus Selection")) exitFieldbus = true;

	ImGui::End();

	return !exitFieldbus;
}