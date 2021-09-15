#include <pch.h>

#include "Gui/Gui.h"

#include "Fieldbus/EtherCatFieldbus.h"
#include "Fieldbus/EtherCatSlave.h"

void etherCatSlaves() {

	ImGui::BeginGroup();

	if (EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting) {
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0, 0.0, 0.0, 1.0));
	}
	if (ImGui::Button("Scan Network")) EtherCatFieldbus::scanNetwork();
	if (EtherCatFieldbus::b_processRunning || EtherCatFieldbus::b_processStarting) {
		ImGui::PopItemFlag();
		ImGui::PopStyleColor();
	}
	ImGui::SameLine();
	ImGui::Text("%i Devices Found", EtherCatFieldbus::slaves.size());

	static int selectedSlaveIndex = -1;

	ImVec2 listWidth(ImGui::GetTextLineHeight() * 14, ImGui::GetContentRegionAvail().y);
	if (ImGui::BeginListBox("##DiscoveredEtherCATSlaves", listWidth)) {
		for (auto slave : EtherCatFieldbus::slaves) {
			bool selected = selectedSlaveIndex == slave->getSlaveIndex();
			if (ImGui::Selectable(slave->getName(), &selected)) selectedSlaveIndex = slave->getSlaveIndex();
		}
		if (EtherCatFieldbus::slaves.empty()) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::Selectable("No Devices Detected...");
			ImGui::PopItemFlag();
		}
		ImGui::EndListBox();
	}
	ImGui::EndGroup();
	ImGui::SameLine();

    std::shared_ptr<EtherCatSlave> selectedSlave = nullptr;
	for (auto slave : EtherCatFieldbus::slaves) 
		if (slave->getSlaveIndex() == selectedSlaveIndex) { selectedSlave = slave; break; }

	ImGui::BeginGroup();
	ImGui::PushFont(Fonts::robotoBold20);
	if (selectedSlave) ImGui::Text("%s (Node #%i, Address: %i) ", selectedSlave->getNodeName(), selectedSlave->getSlaveIndex(), selectedSlave->getStationAlias());
	else ImGui::Text("No Device Selected");
	ImGui::PopFont();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	if (ImGui::BeginChild(ImGui::GetID("SelectedSlaveDisplayWindow"))) {
		if (selectedSlave) {
            if (ImGui::BeginTabBar("DevicePropertiesTabBar")) {
                selectedSlave->nodeSpecificGui();
                ImGui::EndTabBar();
            }
		}
		ImGui::EndChild();
	}
	ImGui::PopStyleVar();
	ImGui::EndGroup();

}
